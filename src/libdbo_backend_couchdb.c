/*
 * Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Based on enforcer-ng/src/db/db_backend_couchdb.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/backend/couchdb.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include <openssl/sha.h>

#define REQUEST_BUFFER_SIZE (4*1024*1024)

#define COUCHLIBDBO_REQUEST_GET 1
#define COUCHLIBDBO_REQUEST_PUT 2
#define COUCHLIBDBO_REQUEST_POST 3
#define COUCHLIBDBO_REQUEST_DELETE 4

/**
 * Keep track of if we have initialized things needed for this backend such as
 * CURL.
 */
static int __couchdb_initialized = 0;

/**
 * The CouchDB database backend specific data.
 */
typedef struct libdbo_backend_couchdb {
    char* url;
    CURL* curl;
    char* buffer;
    size_t buffer_position;
    char* write;
    size_t write_length;
    size_t write_position;
} libdbo_backend_couchdb_t;

static libdbo_mm_t __couchdb_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_couchdb_t));

/*
typedef struct libdbo_backend_couchdb_query {
    libdbo_backend_couchdb_t* backend_couchdb;
    int fields;
    const libdbo_object_t* object;
} libdbo_backend_couchdb_query_t;

static libdbo_mm_t __couchdb_query_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_couchdb_query_t));
*/

static int libdbo_backend_couchdb_initialize(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!__couchdb_initialized) {
        if (curl_global_init(CURL_GLOBAL_ALL)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        __couchdb_initialized = 1;
    }
    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_shutdown(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__couchdb_initialized) {
        curl_global_cleanup();
        __couchdb_initialized = 0;
    }
    return LIBDBO_OK;
}

/**
 * Callback function for CURL to get the response from a HTTP request.
 * \param[in] ptr a void pointer.
 * \param[in] size a size_t.
 * \param[in] nmemb a size_t.
 * \param[in] userdata a void pointer.
 * \return a size_t.
 */
static size_t __db_backend_couchdb_write_response(void* ptr, size_t size, size_t nmemb, void* userdata) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)userdata;

    if(backend_couchdb->buffer_position + size * nmemb >= REQUEST_BUFFER_SIZE - 1) {
        return 0;
    }

    memcpy(backend_couchdb->buffer + backend_couchdb->buffer_position, ptr, size * nmemb);
    backend_couchdb->buffer_position += size * nmemb;

    return size * nmemb;
}

/**
 * Callback function for CURL to write the HTTP request data.
 * \param[in] ptr a void pointer.
 * \param[in] size a size_t.
 * \param[in] nmemb a size_t.
 * \param[in] userdata a void pointer.
 * \return a size_t.
 */
static size_t __db_backend_couchdb_read_request(void* ptr, size_t size, size_t nmemb, void* userdata) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)userdata;
    size_t write = 0;

    if ((backend_couchdb->write_length - backend_couchdb->write_position) > (size * nmemb)) {
        write = (size * nmemb);
    }
    else if ((backend_couchdb->write_length - backend_couchdb->write_position)) {
        write = (backend_couchdb->write_length - backend_couchdb->write_position);
    }

    if (write) {
        memcpy(ptr, backend_couchdb->write + backend_couchdb->write_position, write);
        backend_couchdb->write_position += write;
    }
    return write;
}

/**
 * Make a request to CouchDB. The URL is specified by `request_url`, the request
 * type by `request_type` and the JSON data by `root`.
 * \param[in] backend_couchdb a libdbo_backend_couchdb_t pointer.
 * \param[in] request_url a character pointer.
 * \param[in] request_type an integer.
 * \param[in] root a json_t pointer.
 * \return a long with the HTTP response code or zero on error.
 */
static long __db_backend_couchdb_request(libdbo_backend_couchdb_t* backend_couchdb, const char* request_url, int request_type, json_t* root) {
    CURLcode status;
    long code;
    char url[1024];
    char* urlp;
    int ret, left;
    struct curl_slist* headers = NULL;

    if (!backend_couchdb) {
        return 0;
    }
    if (!backend_couchdb->url) {
        return 0;
    }
    if (!backend_couchdb->buffer) {
        return 0;
    }
    if (!request_url) {
        return 0;
    }

    if (backend_couchdb->curl) {
        curl_easy_cleanup(backend_couchdb->curl);
    }
    if (!(backend_couchdb->curl = curl_easy_init())) {
        return 0;
    }

    left = sizeof(url);
    urlp = url;

    if ((ret = snprintf(urlp, left, "%s", backend_couchdb->url)) >= left) {
        return 0;
    }
    urlp += ret;
    left -= ret;

    if (*(urlp - 1) != '/') {
        if (*request_url != '/') {
            if ((ret = snprintf(urlp, left, "/")) >= left) {
                return 0;
            }
            urlp += ret;
            left -= ret;
        }
    }

    if ((ret = snprintf(urlp, left, "%s", request_url)) >= left) {
        return 0;
    }
    urlp += ret;
    left -= ret;

    if ((status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_URL, url))
        || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_WRITEFUNCTION, __db_backend_couchdb_write_response))
        || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_WRITEDATA, backend_couchdb)))
    {
        puts(curl_easy_strerror(status));
        return 0;
    }
    backend_couchdb->buffer_position = 0;

    switch (request_type) {
    case COUCHLIBDBO_REQUEST_GET:
        break;

    case COUCHLIBDBO_REQUEST_PUT:
        if (!root) {
            return 0;
        }

        if (backend_couchdb->write) {
            return 0;
        }
        backend_couchdb->write = json_dumps(root, JSON_ENSURE_ASCII);
        if (!backend_couchdb->write) {
            return 0;
        }
        backend_couchdb->write_length = strlen(backend_couchdb->write);
        backend_couchdb->write_position = 0;

        headers = curl_slist_append(headers, "Content-Type: application/json");

        if ((status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_HTTPHEADER, headers))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_INFILESIZE, (long)backend_couchdb->write_length))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_READFUNCTION, __db_backend_couchdb_read_request))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_READDATA, backend_couchdb))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_PUT, 1)))
        {
            curl_slist_free_all(headers);
            free(backend_couchdb->write);
            backend_couchdb->write = NULL;
            puts(curl_easy_strerror(status));
            return 0;
        }
        break;

    case COUCHLIBDBO_REQUEST_POST:
        if (!root) {
            return 0;
        }

        if (backend_couchdb->write) {
            return 0;
        }
        backend_couchdb->write = json_dumps(root, JSON_ENSURE_ASCII);
        if (!backend_couchdb->write) {
            return 0;
        }
        backend_couchdb->write_length = strlen(backend_couchdb->write);
        backend_couchdb->write_position = 0;

        headers = curl_slist_append(headers, "Content-Type: application/json");

        if ((status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_HTTPHEADER, headers))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_POSTFIELDS, NULL))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_POSTFIELDSIZE, (long)backend_couchdb->write_length))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_READFUNCTION, __db_backend_couchdb_read_request))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_READDATA, backend_couchdb))
            || (status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_POST, 1)))
        {
            curl_slist_free_all(headers);
            free(backend_couchdb->write);
            backend_couchdb->write = NULL;
            puts(curl_easy_strerror(status));
            return 0;
        }
        break;

    case COUCHLIBDBO_REQUEST_DELETE:
        if ((status = curl_easy_setopt(backend_couchdb->curl, CURLOPT_CUSTOMREQUEST, "DELETE"))) {
            puts(curl_easy_strerror(status));
            return 0;
        }
        break;

    default:
        return 0;
    }

    /*
    puts(url);
    if (backend_couchdb->write) puts(backend_couchdb->write);
    curl_easy_setopt(backend_couchdb->curl, CURLOPT_VERBOSE, 1);
    */

    if ((status = curl_easy_perform(backend_couchdb->curl))) {
        puts(curl_easy_strerror(status));
        return 0;
    }

    backend_couchdb->buffer[backend_couchdb->buffer_position] = 0;

    /*
    puts(backend_couchdb->buffer);
    */

    curl_easy_getinfo(backend_couchdb->curl, CURLINFO_RESPONSE_CODE, &code);

    if (headers) {
        curl_slist_free_all(headers);
    }
    if (backend_couchdb->write) {
        free(backend_couchdb->write);
        backend_couchdb->write = NULL;
    }

    return code;
}

static int libdbo_backend_couchdb_connect(void* data, const libdbo_configuration_list_t* configuration_list) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;
    const libdbo_configuration_t* url;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_couchdb->curl) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!backend_couchdb->buffer) {
        if (!(backend_couchdb->buffer = calloc(REQUEST_BUFFER_SIZE, 1))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (!(url = libdbo_configuration_list_find(configuration_list, "url"))) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_couchdb->url) {
        free(backend_couchdb->url);
    }
    if (!(backend_couchdb->url = strdup(libdbo_configuration_value(url)))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_disconnect(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (backend_couchdb->curl) {
        curl_easy_cleanup(backend_couchdb->curl);
        backend_couchdb->curl = NULL;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_create(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;
    json_t* root;
    json_t* json_value;
    const libdbo_object_field_t* object_field;
    const libdbo_value_t* value;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    size_t value_pos;
    long code;
    char string[1024];
    char* stringp;
    int ret, left;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    root = json_object();
    if (!root) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    object_field = libdbo_object_field_list_begin(object_field_list);
    value_pos = 0;
    while (object_field) {
        if (!(value = libdbo_value_set_at(value_set, value_pos))) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        switch (libdbo_value_type(value)) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(value, &int32)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            if (!(json_value = json_integer(int32))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(value, &uint32)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            if (!(json_value = json_integer(uint32))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

#ifdef JSON_INTEGER_IS_LONG_LONG
        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(value, &int64)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            if (!(json_value = json_integer(int64))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(value, &uint64)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            if (!(json_value = json_integer(uint64))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;
#endif

        case LIBDBO_TYPE_TEXT:
            if (!(json_value = json_string(libdbo_value_text(value)))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_ENUM:
            if (libdbo_value_enum_value(value, &int32)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            if (!(json_value = json_integer(int32))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        left = sizeof(string);
        stringp = string;

        if ((ret = snprintf(stringp, left, "%s_%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
            json_decref(json_value);
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        if (json_object_set_new(root, string, json_value)) {
            json_decref(json_value);
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        object_field = libdbo_object_field_next(object_field);
    }

    if (!(json_value = json_string(libdbo_object_table(object)))) {
        json_decref(root);
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (json_object_set_new(root, "type", json_value)) {
        json_decref(json_value);
        json_decref(root);
        return LIBDBO_ERROR_UNKNOWN;
    }

    code = __db_backend_couchdb_request(backend_couchdb, "", COUCHLIBDBO_REQUEST_POST, root);
    json_decref(root);
    if (code != 201 && code != 202) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

/**
 * Covert a JSON object to a database result.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] json_object a json_t pointer.
 * \return a libdbo_result_t pointer or NULL on error.
 */
static libdbo_result_t* __db_backend_couchdb_result_from_json_object(const libdbo_object_t* object, json_t* json_object) {
    size_t size, i;
    libdbo_result_t* result;
    libdbo_value_set_t* value_set = NULL;
    void *json_iter;
    json_t *json_value = NULL;
    const libdbo_object_field_t* object_field;
    char key[1024];
    char* keyp;
    int ret, left;
    libdbo_backend_meta_data_list_t* backend_meta_data_list = NULL;
    libdbo_backend_meta_data_t* backend_meta_data = NULL;
    libdbo_value_t* value = NULL;

    if (!object) {
        return NULL;
    }
    if (!json_object) {
        return NULL;
    }

    size = 0;
    json_iter = json_object_iter(json_object);
    while (json_iter) {
        if (!strcmp(json_object_iter_key(json_iter), "_rev")) {
            json_iter = json_object_iter_next(json_object, json_iter);
            continue;
        }
        if (!strcmp(json_object_iter_key(json_iter), "type")) {
            json_iter = json_object_iter_next(json_object, json_iter);
            continue;
        }

        size++;
        json_iter = json_object_iter_next(json_object, json_iter);
    }

    if (!(result = libdbo_result_new())
        || !(value_set = libdbo_value_set_new(size))
        || libdbo_result_set_value_set(result, value_set))
    {
        libdbo_result_free(result);
        libdbo_value_set_free(value_set);
        return NULL;
    }

    if (!(value = libdbo_value_new())
        || !(json_value = json_object_get(json_object, "_rev"))
        || !json_is_string(json_value)
        || libdbo_value_from_text(value, json_string_value(json_value))
        || !(backend_meta_data = libdbo_backend_meta_data_new())
        || libdbo_backend_meta_data_set_name(backend_meta_data, "rev")
        || libdbo_backend_meta_data_set_value(backend_meta_data, value))
    {
        libdbo_value_free(value);
        libdbo_backend_meta_data_free(backend_meta_data);
        libdbo_result_free(result);
        return NULL;
    }
    value = NULL;

    if (!(backend_meta_data_list = libdbo_backend_meta_data_list_new())
        || libdbo_backend_meta_data_list_add(backend_meta_data_list, backend_meta_data))
    {
        libdbo_backend_meta_data_free(backend_meta_data);
        libdbo_backend_meta_data_list_free(backend_meta_data_list);
        libdbo_result_free(result);
        return NULL;
    }
    backend_meta_data = NULL;

    if (libdbo_result_set_backend_meta_data_list(result, backend_meta_data_list)) {
        libdbo_backend_meta_data_list_free(backend_meta_data_list);
        libdbo_result_free(result);
        return NULL;
    }
    backend_meta_data_list = NULL;

    i = 0;
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (i == size) {
            libdbo_result_free(result);
            return NULL;
        }

        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_PRIMARY_KEY) {
            json_value = json_object_get(json_object, "_id");
        }
        else {
            left = sizeof(key);
            keyp = key;

            if ((ret = snprintf(keyp, left, "%s_%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                libdbo_result_free(result);
                return NULL;
            }
            keyp += ret;
            left -= ret;

            json_value = json_object_get(json_object, key);
        }
        if (!json_value) {
            libdbo_result_free(result);
            return NULL;
        }

        switch (libdbo_object_field_type(object_field)) {
        case LIBDBO_TYPE_PRIMARY_KEY:
            if (!json_is_string(json_value)
                || libdbo_value_from_text(libdbo_value_set_get(value_set, i), json_string_value(json_value))
                || libdbo_value_set_primary_key(libdbo_value_set_get(value_set, i)))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            if (!json_is_string(json_value)
                || libdbo_value_from_text(libdbo_value_set_get(value_set, i), json_string_value(json_value)))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_ENUM:
            /*
             * Enum needs to be handled elsewhere since we don't know the
             * enum_set_t here.
             */
        case LIBDBO_TYPE_INT32:
        case LIBDBO_TYPE_UINT32:
        case LIBDBO_TYPE_INT64:
        case LIBDBO_TYPE_UINT64:
            if (!json_is_number(json_value)
#ifdef JSON_INTEGER_IS_LONG_LONG
                || libdbo_value_from_int64(libdbo_value_set_get(value_set, i), json_integer_value(json_value)))
#else
                || libdbo_value_from_int32(libdbo_value_set_get(value_set, i), json_integer_value(json_value)))
#endif
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        default:
            libdbo_result_free(result);
            return NULL;
        }

        object_field = libdbo_object_field_next(object_field);
        i++;
    }

    return result;
}

/**
 * Store the response from a CouchDB request into a database result list
 * specified by `result_list`. If the response comes from a CouchDB view then
 * `view` must be non-zero.
 * \param[in] backend_couchdb a libdbo_backend_couchdb_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \param[in] view an integer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_couchdb_store_result(libdbo_backend_couchdb_t* backend_couchdb, const libdbo_object_t* object, libdbo_result_list_t* result_list, int view) {
    json_t *root;
    json_t *rows;
    json_t *entry;
    json_error_t error;
    size_t i;
    libdbo_result_t* result;

    if (!(root = json_loads(backend_couchdb->buffer, 0, &error))) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (view) {
        if (!json_is_object(root)) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }
        rows = json_object_get(root, "rows");
        if (!rows) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }
    else {
        rows = root;
    }

    if (json_is_object(rows)) {
        if (view) {
            entry = json_object_get(rows, "doc");
            if (!entry) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        else {
            entry = rows;
        }

        if (!(result = __db_backend_couchdb_result_from_json_object(object, entry))) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        if (libdbo_result_list_add(result_list, result)) {
            json_decref(root);
            libdbo_result_free(result);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }
    else if (json_is_array(rows)) {
        for (i = 0; i < json_array_size(rows); i++) {
            entry = json_array_get(rows, i);
            if (!json_is_object(entry)) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            if (view) {
                entry = json_object_get(entry, "doc");
                if (!entry) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }

            if (!(result = __db_backend_couchdb_result_from_json_object(object, entry))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            if (libdbo_result_list_add(result_list, result)) {
                json_decref(root);
                libdbo_result_free(result);
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
    }
    else {
        json_decref(root);
        return LIBDBO_ERROR_UNKNOWN;
    }
    json_decref(root);
    return LIBDBO_OK;
}

/**
 * Build parts of a map function from the database clause list specified by
 * `clause_list`, append the result to `stringp`. How much that is left in the
 * buffer pointed by `stringp` is specified by `left`.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[in] stringp a character pointer pointer.
 * \param[in] left an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_couchdb_build_map_function(const libdbo_object_t* object, const libdbo_clause_list_t* clause_list, char** stringp, int* left) {
    const libdbo_clause_t* clause;
    int ret;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    const char* text;

    if (!clause_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!stringp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*stringp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (*left < 1) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        switch (libdbo_clause_operator(clause)) {
        case LIBDBO_CLAUSE_OPERATOR_AND:
            if ((ret = snprintf(*stringp, *left, " &&")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_OPERATOR_OR:
            if ((ret = snprintf(*stringp, *left, " ||")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        *stringp += ret;
        *left -= ret;

        if ((ret = snprintf(*stringp, *left, " doc.%s_%s", libdbo_object_table(object), libdbo_clause_field(clause))) >= *left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        *stringp += ret;
        *left -= ret;

        switch (libdbo_clause_type(clause)) {
        case LIBDBO_CLAUSE_EQUAL:
            if ((ret = snprintf(*stringp, *left, " == ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_NOT_EQUAL:
            if ((ret = snprintf(*stringp, *left, " != ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_THEN:
            if ((ret = snprintf(*stringp, *left, " < ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_OR_EQUAL:
            if ((ret = snprintf(*stringp, *left, " <= ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_OR_EQUAL:
            if ((ret = snprintf(*stringp, *left, " >= ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_THEN:
            if ((ret = snprintf(*stringp, *left, " > ")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NULL:
            if ((ret = snprintf(*stringp, *left, " == null")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *stringp += ret;
            *left -= ret;
            clause = libdbo_clause_next(clause);
            continue;
            break;

        case LIBDBO_CLAUSE_IS_NOT_NULL:
            if ((ret = snprintf(*stringp, *left, " != null")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *stringp += ret;
            *left -= ret;
            clause = libdbo_clause_next(clause);
            continue;
            break;

        case LIBDBO_CLAUSE_NESTED:
            if ((ret = snprintf(*stringp, *left, " (")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *stringp += ret;
            *left -= ret;
            if (__db_backend_couchdb_build_map_function(object, libdbo_clause_list(clause), stringp, left)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*stringp, *left, " )")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *stringp += ret;
            *left -= ret;
            clause = libdbo_clause_next(clause);
            continue;
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        *stringp += ret;
        *left -= ret;

        switch (libdbo_value_type(libdbo_clause_value(clause))) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(libdbo_clause_value(clause), &int32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*stringp, *left, "%d", int32)) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(libdbo_clause_value(clause), &uint32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*stringp, *left, "%u", uint32)) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(libdbo_clause_value(clause), &int64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*stringp, *left, "%ld", int64)) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(libdbo_clause_value(clause), &uint64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*stringp, *left, "%lu", uint64)) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            text = libdbo_value_text(libdbo_clause_value(clause));
            if (!text) {
                return LIBDBO_ERROR_UNKNOWN;
            }

            if ((ret = snprintf(*stringp, *left, "\"")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *stringp += ret;
            *left -= ret;

            while (*text) {
                if (*text == '"') {
                    if ((ret = snprintf(*stringp, *left, "\\\"")) >= *left) {
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                }
                else {
                    if ((ret = snprintf(*stringp, *left, "%c", *text)) >= *left) {
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                }
                *stringp += ret;
                *left -= ret;
                text++;
            }

            if ((ret = snprintf(*stringp, *left, "\"")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        *stringp += ret;
        *left -= ret;

        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

static libdbo_result_list_t* libdbo_backend_couchdb_read(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;
    long code;
    libdbo_result_list_t* result_list;
    char string[4096];
    char* stringp;
    int ret, left, only_ids, have_clauses;
    const libdbo_clause_t* clause;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char hash_string[(SHA256_DIGEST_LENGTH*2)+1];
    SHA256_CTX sha256;

    if (!__couchdb_initialized) {
        return NULL;
    }
    if (!backend_couchdb) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }

    if (join_list) {
        /*
         * Joins is not supported by this backend, check if there are any and
         * return error if so.
         */
        if (libdbo_join_list_begin(join_list)) {
            return NULL;
        }
    }

    only_ids = 0;
    have_clauses = 0;
    if (clause_list) {
        clause = libdbo_clause_list_begin(clause_list);
        only_ids = 1;
        while (clause) {
            if (libdbo_clause_table(clause)) {
                /*
                 * This backend only supports clauses on the objects table.
                 */
                if (strcmp(libdbo_clause_table(clause), libdbo_object_table(object))) {
                    return NULL;
                }
            }

            if (strcmp(libdbo_clause_field(clause), libdbo_object_primary_key_name(object))) {
                only_ids = 0;
                have_clauses = 1;
            }
            clause = libdbo_clause_next(clause);
        }
    }

    if (!(result_list = libdbo_result_list_new())) {
        return NULL;
    }

    if (only_ids) {
        clause = libdbo_clause_list_begin(clause_list);
        while (clause) {
            left = sizeof(string);
            stringp = string;

            switch (libdbo_value_type(libdbo_clause_value(clause))) {
            case LIBDBO_TYPE_INT32:
                if (libdbo_value_to_int32(libdbo_clause_value(clause), &int32)) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                if ((ret = snprintf(stringp, left, "/%d", int32)) >= left) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                break;

            case LIBDBO_TYPE_UINT32:
                if (libdbo_value_to_uint32(libdbo_clause_value(clause), &uint32)) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                if ((ret = snprintf(stringp, left, "/%u", uint32)) >= left) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                break;

            case LIBDBO_TYPE_INT64:
                if (libdbo_value_to_int64(libdbo_clause_value(clause), &int64)) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                if ((ret = snprintf(stringp, left, "/%ld", int64)) >= left) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                break;

            case LIBDBO_TYPE_UINT64:
                if (libdbo_value_to_uint64(libdbo_clause_value(clause), &uint64)) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                if ((ret = snprintf(stringp, left, "/%lu", uint64)) >= left) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                break;

            case LIBDBO_TYPE_TEXT:
                if ((ret = snprintf(stringp, left, "/%s", libdbo_value_text(libdbo_clause_value(clause)))) >= left) {
                    libdbo_result_list_free(result_list);
                    return NULL;
                }
                break;

            default:
                libdbo_result_list_free(result_list);
                return NULL;
            }
            stringp += ret;
            left -= ret;

            code = __db_backend_couchdb_request(backend_couchdb, string, COUCHLIBDBO_REQUEST_GET, NULL);
            if (code != 200) {
                libdbo_result_list_free(result_list);
                return NULL;
            }

            if (__db_backend_couchdb_store_result(backend_couchdb, object, result_list, 0)) {
                libdbo_result_list_free(result_list);
                return NULL;
            }

            clause = libdbo_clause_next(clause);
        }
    }
    else if (have_clauses) {
        json_t* map = NULL;
        json_t* view = NULL;
        json_t* views = NULL;
        json_t* root = NULL;

        left = sizeof(string);
        stringp = string;

        if ((ret = snprintf(stringp, left, "function(doc) { if (doc.type == \"%s\"", libdbo_object_table(object))) >= left) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
        stringp += ret;
        left -= ret;

        if (__db_backend_couchdb_build_map_function(object, clause_list, &stringp, &left)) {
            libdbo_result_list_free(result_list);
            return NULL;
        }

        if ((ret = snprintf(stringp, left, ") { emit(doc._id, doc.test_name); } }")) >= left) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
        stringp += ret;
        left -= ret;

        SHA256_Init(&sha256);
        SHA256_Update(&sha256, string, (unsigned long)(stringp - string));
        SHA256_Final(hash, &sha256);

        for (ret = 0; ret < SHA256_DIGEST_LENGTH; ret++) {
            sprintf(&hash_string[ret*2], "%02x", hash[ret]);
        }
        hash_string[(SHA256_DIGEST_LENGTH*2)] = 0;

        if (!(map = json_string(string))
            || !(view = json_object())
            || !(views = json_object())
            || !(root = json_object()))
        {
            json_decref(map);
            json_decref(view);
            json_decref(views);
            json_decref(root);
            libdbo_result_list_free(result_list);
            return NULL;
        }

        if (json_object_set(view, "map", map)) {
            json_decref(map);
            json_decref(view);
            json_decref(views);
            json_decref(root);
            libdbo_result_list_free(result_list);
            return NULL;
        }
        json_decref(map);

        if (json_object_set(views, "view", view)) {
            json_decref(view);
            json_decref(views);
            json_decref(root);
            libdbo_result_list_free(result_list);
            return NULL;
        }
        json_decref(view);

        if (json_object_set(root, "views", views)) {
            json_decref(views);
            json_decref(root);
            libdbo_result_list_free(result_list);
            return NULL;
        }
        json_decref(views);

        left = sizeof(string);
        stringp = string;

        if ((ret = snprintf(stringp, left, "/_design/%s", hash_string)) >= left) {
            json_decref(root);
            libdbo_result_list_free(result_list);
            return NULL;
        }
        stringp += ret;
        left -= ret;

        code = __db_backend_couchdb_request(backend_couchdb, string, COUCHLIBDBO_REQUEST_PUT, root);
        json_decref(root);
        if (code != 201 && code != 202 && code != 409) {
            libdbo_result_list_free(result_list);
            return NULL;
        }

        left = sizeof(string);
        stringp = string;

        if ((ret = snprintf(stringp, left, "/_design/%s/_view/view?include_docs=true", hash_string)) >= left) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
        stringp += ret;
        left -= ret;

        code = __db_backend_couchdb_request(backend_couchdb, string, COUCHLIBDBO_REQUEST_GET, NULL);
        if (code != 200) {
            libdbo_result_list_free(result_list);
            return NULL;
        }

        if (__db_backend_couchdb_store_result(backend_couchdb, object, result_list, 1)) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
    }
    else {
        left = sizeof(string);
        stringp = string;

        if ((ret = snprintf(stringp, left, "/_design/application/_view/%s?include_docs=true", libdbo_object_table(object))) >= left) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
        stringp += ret;
        left -= ret;

        code = __db_backend_couchdb_request(backend_couchdb, string, COUCHLIBDBO_REQUEST_GET, NULL);
        if (code != 200) {
            libdbo_result_list_free(result_list);
            return NULL;
        }

        if (__db_backend_couchdb_store_result(backend_couchdb, object, result_list, 1)) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
    }

    return result_list;
}

static int libdbo_backend_couchdb_update(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;
    long code;
    char url[4096];
    char* urlp;
    int ret, left;
    const libdbo_clause_t* clause;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    const libdbo_backend_meta_data_t* rev;
    json_t* root;
    json_t* json_value;
    const libdbo_object_field_t* object_field;
    const libdbo_value_t* value;
    size_t value_pos;
    char string[1024];
    char* stringp;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * We need the rev from the backend_meta_data_list in order to update
     * objects in CouchDB
     */
    if (!libdbo_object_backend_meta_data_list(object)) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!(rev = libdbo_backend_meta_data_list_find(libdbo_object_backend_meta_data_list(object), "rev"))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        if (libdbo_clause_table(clause)) {
            /*
             * This backend only supports clauses on the objects table.
             */
            if (strcmp(libdbo_clause_table(clause), libdbo_object_table(object))) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }

        /*
         * Only support updating by id
         */
        if (strcmp(libdbo_clause_field(clause), libdbo_object_primary_key_name(object))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        clause = libdbo_clause_next(clause);
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        left = sizeof(url);
        urlp = url;

        switch (libdbo_value_type(libdbo_clause_value(clause))) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(libdbo_clause_value(clause), &int32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%d", int32)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(libdbo_clause_value(clause), &uint32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%u", uint32)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(libdbo_clause_value(clause), &int64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%ld", int64)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(libdbo_clause_value(clause), &uint64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%lu", uint64)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            if ((ret = snprintf(urlp, left, "/%s", libdbo_value_text(libdbo_clause_value(clause)))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }

        root = json_object();
        if (!root) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        object_field = libdbo_object_field_list_begin(object_field_list);
        value_pos = 0;
        while (object_field) {
            if (!(value = libdbo_value_set_at(value_set, value_pos))) {
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            switch (libdbo_value_type(value)) {
            case LIBDBO_TYPE_INT32:
                if (libdbo_value_to_int32(value, &int32)) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                if (!(json_value = json_integer(int32))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_UINT32:
                if (libdbo_value_to_uint32(value, &uint32)) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                if (!(json_value = json_integer(uint32))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

#ifdef JSON_INTEGER_IS_LONG_LONG
            case LIBDBO_TYPE_INT64:
                if (libdbo_value_to_int64(value, &int64)) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                if (!(json_value = json_integer(int64))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_UINT64:
                if (libdbo_value_to_uint64(value, &uint64)) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                if (!(json_value = json_integer(uint64))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;
#endif

            case LIBDBO_TYPE_TEXT:
                if (!(json_value = json_string(libdbo_value_text(value)))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_ENUM:
                if (libdbo_value_enum_value(value, &int32)) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                if (!(json_value = json_integer(int32))) {
                    json_decref(root);
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            default:
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            left = sizeof(string);
            stringp = string;

            if ((ret = snprintf(stringp, left, "%s_%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                json_decref(json_value);
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            if (json_object_set_new(root, string, json_value)) {
                json_decref(json_value);
                json_decref(root);
                return LIBDBO_ERROR_UNKNOWN;
            }

            object_field = libdbo_object_field_next(object_field);
        }

        if (!(json_value = json_string(libdbo_object_table(object)))) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }
        if (json_object_set_new(root, "type", json_value)) {
            json_decref(json_value);
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        if (!(json_value = json_string(libdbo_value_text(libdbo_backend_meta_data_value(rev))))) {
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }
        if (json_object_set_new(root, "_rev", json_value)) {
            json_decref(json_value);
            json_decref(root);
            return LIBDBO_ERROR_UNKNOWN;
        }

        code = __db_backend_couchdb_request(backend_couchdb, url, COUCHLIBDBO_REQUEST_PUT, root);
        json_decref(root);
        if (code != 201 && code != 202) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_delete(void* data, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;
    long code;
    char url[4096];
    char* urlp;
    int ret, left;
    const libdbo_clause_t* clause;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    const libdbo_backend_meta_data_t* rev;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * We need the rev from the backend_meta_data_list in order to delete
     * objects in CouchDB
     */
    if (!libdbo_object_backend_meta_data_list(object)) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!(rev = libdbo_backend_meta_data_list_find(libdbo_object_backend_meta_data_list(object), "rev"))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        if (libdbo_clause_table(clause)) {
            /*
             * This backend only supports clauses on the objects table.
             */
            if (strcmp(libdbo_clause_table(clause), libdbo_object_table(object))) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }

        /*
         * Only support deleting by id
         */
        if (strcmp(libdbo_clause_field(clause), libdbo_object_primary_key_name(object))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        clause = libdbo_clause_next(clause);
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        left = sizeof(url);
        urlp = url;

        switch (libdbo_value_type(libdbo_clause_value(clause))) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(libdbo_clause_value(clause), &int32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%d", int32)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(libdbo_clause_value(clause), &uint32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%u", uint32)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(libdbo_clause_value(clause), &int64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%ld", int64)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(libdbo_clause_value(clause), &uint64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(urlp, left, "/%lu", uint64)) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            if ((ret = snprintf(urlp, left, "/%s", libdbo_value_text(libdbo_clause_value(clause)))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        urlp += ret;
        left -= ret;

        if ((ret = snprintf(urlp, left, "?rev=%s", libdbo_value_text(libdbo_backend_meta_data_value(rev)))) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        code = __db_backend_couchdb_request(backend_couchdb, url, COUCHLIBDBO_REQUEST_DELETE, NULL);
        if (code != 200 && code != 202) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_count(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    return LIBDBO_ERROR_UNKNOWN;
}

static void libdbo_backend_couchdb_free(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (backend_couchdb) {
        if (backend_couchdb->url) {
            free(backend_couchdb->url);
        }
        if (backend_couchdb->curl) {
            libdbo_backend_couchdb_disconnect(backend_couchdb);
        }
        if (backend_couchdb->buffer) {
            free(backend_couchdb->buffer);
        }
        libdbo_mm_delete(&__couchdb_alloc, backend_couchdb);
    }
}

static int libdbo_backend_couchdb_transaction_begin(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_transaction_commit(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_couchdb_transaction_rollback(void* data) {
    libdbo_backend_couchdb_t* backend_couchdb = (libdbo_backend_couchdb_t*)data;

    if (!__couchdb_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_couchdb) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

libdbo_backend_handle_t* libdbo_backend_couchdb_new_handle(void) {
    libdbo_backend_handle_t* backend_handle = NULL;
    libdbo_backend_couchdb_t* backend_couchdb =
        (libdbo_backend_couchdb_t*)libdbo_mm_new0(&__couchdb_alloc);

    if (backend_couchdb && (backend_handle = libdbo_backend_handle_new())) {
        if (libdbo_backend_handle_set_data(backend_handle, (void*)backend_couchdb)
            || libdbo_backend_handle_set_initialize(backend_handle, libdbo_backend_couchdb_initialize)
            || libdbo_backend_handle_set_shutdown(backend_handle, libdbo_backend_couchdb_shutdown)
            || libdbo_backend_handle_set_connect(backend_handle, libdbo_backend_couchdb_connect)
            || libdbo_backend_handle_set_disconnect(backend_handle, libdbo_backend_couchdb_disconnect)
            || libdbo_backend_handle_set_create(backend_handle, libdbo_backend_couchdb_create)
            || libdbo_backend_handle_set_read(backend_handle, libdbo_backend_couchdb_read)
            || libdbo_backend_handle_set_update(backend_handle, libdbo_backend_couchdb_update)
            || libdbo_backend_handle_set_delete(backend_handle, libdbo_backend_couchdb_delete)
            || libdbo_backend_handle_set_count(backend_handle, libdbo_backend_couchdb_count)
            || libdbo_backend_handle_set_free(backend_handle, libdbo_backend_couchdb_free)
            || libdbo_backend_handle_set_transaction_begin(backend_handle, libdbo_backend_couchdb_transaction_begin)
            || libdbo_backend_handle_set_transaction_commit(backend_handle, libdbo_backend_couchdb_transaction_commit)
            || libdbo_backend_handle_set_transaction_rollback(backend_handle, libdbo_backend_couchdb_transaction_rollback))
        {
            libdbo_backend_handle_free(backend_handle);
            libdbo_mm_delete(&__couchdb_alloc, backend_couchdb);
            return NULL;
        }
    }
    return backend_handle;
}
