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
 * Based on enforcer-ng/src/db/db_backend_mysql.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/backend/mysql.h"

#include "libdbo/error.h"
#include "libdbo/mm.h"
#include "libdbo/log.h"

#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

static int libdbo_backend_mysql_transaction_rollback(void*);

/**
 * Keep track of if we have initialized the MySQL backend.
 */
static int __mysql_initialized = 0;

/**
 * The MySQL database backend specific data.
 */
typedef struct libdbo_backend_mysql {
    MYSQL* db;
    int transaction;
    unsigned int timeout;
} libdbo_backend_mysql_t;

static libdbo_mm_t __mysql_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_mysql_t));

/**
 * The MySQL database backend specific data for a statement bind.
 */
typedef struct libdbo_backend_mysql_bind libdbo_backend_mysql_bind_t;
struct libdbo_backend_mysql_bind {
    libdbo_backend_mysql_bind_t* next;
    MYSQL_BIND* bind;
    unsigned long length;
    my_bool error;
    int value_enum;
};

static libdbo_mm_t __mysql_bind_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_mysql_bind_t));

/**
 * The MySQL database backend specific data for statements.
 */
typedef struct libdbo_backend_mysql_statement {
    libdbo_backend_mysql_t* backend_mysql;
    MYSQL_STMT* statement;
    MYSQL_BIND* mysql_bind_input;
    libdbo_backend_mysql_bind_t* bind_input;
    libdbo_backend_mysql_bind_t* bind_input_end;
    MYSQL_BIND* mysql_bind_output;
    libdbo_backend_mysql_bind_t* bind_output;
    libdbo_backend_mysql_bind_t* bind_output_end;
    libdbo_object_field_list_t* object_field_list;
    int fields;
    int bound;
} libdbo_backend_mysql_statement_t;

static libdbo_mm_t __mysql_statement_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_mysql_statement_t));

/**
 * MySQL finish function.
 *
 * Frees all data related to a libdbo_backend_mysql_statement_t.
 */
static inline void __db_backend_mysql_finish(libdbo_backend_mysql_statement_t* statement) {
    libdbo_backend_mysql_bind_t* bind;

    if (!statement) {
        return;
    }

    if (statement->statement) {
        mysql_stmt_close(statement->statement);
    }
    if (statement->mysql_bind_input) {
        free(statement->mysql_bind_input);
    }
    while (statement->bind_input) {
        bind = statement->bind_input;
        statement->bind_input = bind->next;
        libdbo_mm_delete(&__mysql_bind_alloc, bind);
    }
    while (statement->bind_output) {
        bind = statement->bind_output;
        statement->bind_output = bind->next;
        if (bind->bind && bind->bind->buffer) {
            free(bind->bind->buffer);
        }
        libdbo_mm_delete(&__mysql_bind_alloc, bind);
    }
    if (statement->mysql_bind_output) {
        free(statement->mysql_bind_output);
    }
    if (statement->object_field_list) {
        libdbo_object_field_list_free(statement->object_field_list);
    }

    libdbo_mm_delete(&__mysql_statement_alloc, statement);
}

/**
 * MySQL prepare function.
 *
 * Creates a libdbo_backend_mysql_statement_t based on a SQL string and an object
 * field list.
 */
static inline int __db_backend_mysql_prepare(libdbo_backend_mysql_t* backend_mysql, libdbo_backend_mysql_statement_t** statement, const char* sql, size_t size, const libdbo_object_field_list_t* object_field_list) {
    unsigned long i, params;
    libdbo_backend_mysql_bind_t* bind;
    const libdbo_object_field_t* object_field;
    MYSQL_BIND* mysql_bind;
    MYSQL_RES* result_metadata = NULL;
    MYSQL_FIELD* field;

    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (*statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!sql) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Prepare the statement.
     */
    if (!(*statement = libdbo_mm_new0(&__mysql_statement_alloc))
        || !((*statement)->statement = mysql_stmt_init(backend_mysql->db))
        || mysql_stmt_prepare((*statement)->statement, sql, size))
    {
        if ((*statement)->statement) {
            libdbo_log(LIBDBO_LOG_ERROR, "MySQL prepare statement error %d: %s (SQL: %s)",
                mysql_stmt_errno((*statement)->statement), mysql_stmt_error((*statement)->statement), sql);
        }
        __db_backend_mysql_finish(*statement);
        *statement = NULL;
        return LIBDBO_ERROR_UNKNOWN;
    }

    (*statement)->backend_mysql = backend_mysql;

    /*
     * Create the input binding based on the number of parameters in the SQL
     * statement.
     */
    if ((params = mysql_stmt_param_count((*statement)->statement)) > 0) {
        if (!((*statement)->mysql_bind_input = calloc(params, sizeof(MYSQL_BIND)))) {
            __db_backend_mysql_finish(*statement);
            *statement = NULL;
            return LIBDBO_ERROR_UNKNOWN;
        }

        for (i = 0; i < params; i++) {
            if (!(bind = libdbo_mm_new0(&__mysql_bind_alloc))) {
                __db_backend_mysql_finish(*statement);
                *statement = NULL;
                return LIBDBO_ERROR_UNKNOWN;
            }

            bind->bind = &((*statement)->mysql_bind_input[i]);
            if (!(*statement)->bind_input) {
                (*statement)->bind_input = bind;
            }
            if ((*statement)->bind_input_end) {
                (*statement)->bind_input_end->next = bind;
            }
            (*statement)->bind_input_end = bind;
        }
    }

    /*
     * Create the output binding based on the object field list given.
     */
    if (object_field_list
        && (params = libdbo_object_field_list_size(object_field_list)) > 0
        && (result_metadata = mysql_stmt_result_metadata((*statement)->statement)))
    {
        if (!((*statement)->object_field_list = libdbo_object_field_list_new_copy(object_field_list))
            || !((*statement)->mysql_bind_output = calloc(params, sizeof(MYSQL_BIND))))
        {
            mysql_free_result(result_metadata);
            __db_backend_mysql_finish(*statement);
            *statement = NULL;
            return LIBDBO_ERROR_UNKNOWN;
        }

        (*statement)->fields = params;
        field = mysql_fetch_field(result_metadata);
        object_field = libdbo_object_field_list_begin(object_field_list);
        for (i = 0; i < params; i++) {
            if (!field
                || !object_field
                || !(bind = libdbo_mm_new0(&__mysql_bind_alloc)))
            {
                mysql_free_result(result_metadata);
                __db_backend_mysql_finish(*statement);
                *statement = NULL;
                return LIBDBO_ERROR_UNKNOWN;
            }

            bind->bind = (mysql_bind = &((*statement)->mysql_bind_output[i]));
            mysql_bind->is_null = (my_bool*)0;
            mysql_bind->error = &bind->error;
            mysql_bind->length = &bind->length;

            switch (libdbo_object_field_type(object_field)) {
            case LIBDBO_TYPE_PRIMARY_KEY:
                switch (field->type) {
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_INT24:
                    mysql_bind->buffer_type = MYSQL_TYPE_LONG;
                    if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint32_t)))) {
                        mysql_free_result(result_metadata);
                        __db_backend_mysql_finish(*statement);
                        *statement = NULL;
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                    mysql_bind->buffer_length = sizeof(libdbo_type_uint32_t);
                    bind->length = mysql_bind->buffer_length;
                    mysql_bind->is_unsigned = 1;
                    break;

                case MYSQL_TYPE_LONGLONG:
                    mysql_bind->buffer_type = MYSQL_TYPE_LONGLONG;
                    if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint64_t)))) {
                        mysql_free_result(result_metadata);
                        __db_backend_mysql_finish(*statement);
                        *statement = NULL;
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                    mysql_bind->buffer_length = sizeof(libdbo_type_uint64_t);
                    bind->length = mysql_bind->buffer_length;
                    mysql_bind->is_unsigned = 1;
                    break;

                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                    mysql_bind->buffer_type = MYSQL_TYPE_STRING;
                    /*
                     * field->length does not include ending NULL character so
                     * we increase it by one.
                     */
                    bind->length = field->length + 1;
                    if (bind->length < LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE) {
                        bind->length = LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE;
                    }
                    if (!(mysql_bind->buffer = calloc(1, bind->length))) {
                        mysql_free_result(result_metadata);
                        __db_backend_mysql_finish(*statement);
                        *statement = NULL;
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                    mysql_bind->buffer_length = bind->length;
                    mysql_bind->is_unsigned = 0;
                    break;

                default:
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_ENUM:
                /*
                 * Enum needs to be handled elsewhere since we don't know the
                 * enum_set_t here.
                 *
                 * TODO: can something be done here?
                 */
            case LIBDBO_TYPE_INT32:
                mysql_bind->buffer_type = MYSQL_TYPE_LONG;
                if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_int32_t)))) {
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                mysql_bind->buffer_length = sizeof(libdbo_type_int32_t);
                bind->length = mysql_bind->buffer_length;
                mysql_bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_UINT32:
                mysql_bind->buffer_type = MYSQL_TYPE_LONG;
                if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint32_t)))) {
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                mysql_bind->buffer_length = sizeof(libdbo_type_uint32_t);
                bind->length = mysql_bind->buffer_length;
                mysql_bind->is_unsigned = 1;
                break;

            case LIBDBO_TYPE_INT64:
                mysql_bind->buffer_type = MYSQL_TYPE_LONGLONG;
                if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_int64_t)))) {
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                mysql_bind->buffer_length = sizeof(libdbo_type_int64_t);
                bind->length = mysql_bind->buffer_length;
                mysql_bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_UINT64:
                mysql_bind->buffer_type = MYSQL_TYPE_LONGLONG;
                if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint64_t)))) {
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                mysql_bind->buffer_length = sizeof(libdbo_type_uint64_t);
                bind->length = mysql_bind->buffer_length;
                mysql_bind->is_unsigned = 1;
                break;

            case LIBDBO_TYPE_TEXT:
                mysql_bind->buffer_type = MYSQL_TYPE_STRING;
                /*
                 * field->length does not include ending NULL character so
                 * we increase it by one.
                 */
                bind->length = field->length + 1;
                if (bind->length < LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE) {
                    bind->length = LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE;
                }
                if (!(mysql_bind->buffer = calloc(1, bind->length))) {
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                mysql_bind->buffer_length = bind->length;
                mysql_bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_ANY:
            case LIBDBO_TYPE_REVISION:
                switch (field->type) {
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_INT24:
                    mysql_bind->buffer_type = MYSQL_TYPE_LONG;
                    if (field->flags & UNSIGNED_FLAG) {
                        if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint32_t)))) {
                            mysql_free_result(result_metadata);
                            __db_backend_mysql_finish(*statement);
                            *statement = NULL;
                            return LIBDBO_ERROR_UNKNOWN;
                        }
                        mysql_bind->buffer_length = sizeof(libdbo_type_uint32_t);
                        mysql_bind->is_unsigned = 1;
                    }
                    else {
                        if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_int32_t)))) {
                            mysql_free_result(result_metadata);
                            __db_backend_mysql_finish(*statement);
                            *statement = NULL;
                            return LIBDBO_ERROR_UNKNOWN;
                        }
                        mysql_bind->buffer_length = sizeof(libdbo_type_int32_t);
                        mysql_bind->is_unsigned = 0;
                    }
                    bind->length = mysql_bind->buffer_length;
                    break;

                case MYSQL_TYPE_LONGLONG:
                    mysql_bind->buffer_type = MYSQL_TYPE_LONGLONG;
                    if (field->flags & UNSIGNED_FLAG) {
                        if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_uint64_t)))) {
                            mysql_free_result(result_metadata);
                            __db_backend_mysql_finish(*statement);
                            *statement = NULL;
                            return LIBDBO_ERROR_UNKNOWN;
                        }
                        mysql_bind->buffer_length = sizeof(libdbo_type_uint64_t);
                        mysql_bind->is_unsigned = 1;
                    }
                    else {
                        if (!(mysql_bind->buffer = calloc(1, sizeof(libdbo_type_int64_t)))) {
                            mysql_free_result(result_metadata);
                            __db_backend_mysql_finish(*statement);
                            *statement = NULL;
                            return LIBDBO_ERROR_UNKNOWN;
                        }
                        mysql_bind->buffer_length = sizeof(libdbo_type_int64_t);
                        mysql_bind->is_unsigned = 0;
                    }
                    bind->length = mysql_bind->buffer_length;
                    break;

                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                    mysql_bind->buffer_type = MYSQL_TYPE_STRING;
                    /*
                     * field->length does not include ending NULL character so
                     * we increase it by one.
                     */
                    bind->length = field->length + 1;
                    if (bind->length < LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE) {
                        bind->length = LIBDBO_BACKEND_MYSQL_STRING_MIN_SIZE;
                    }
                    if (!(mysql_bind->buffer = calloc(1, bind->length))) {
                        mysql_free_result(result_metadata);
                        __db_backend_mysql_finish(*statement);
                        *statement = NULL;
                        return LIBDBO_ERROR_UNKNOWN;
                    }
                    mysql_bind->buffer_length = bind->length;
                    mysql_bind->is_unsigned = 0;
                    break;

                default:
                    mysql_free_result(result_metadata);
                    __db_backend_mysql_finish(*statement);
                    *statement = NULL;
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            default:
                return LIBDBO_ERROR_UNKNOWN;
            }

            if (!(*statement)->bind_output) {
                (*statement)->bind_output = bind;
            }
            if ((*statement)->bind_output_end) {
                (*statement)->bind_output_end->next = bind;
            }
            (*statement)->bind_output_end = bind;
            object_field = libdbo_object_field_next(object_field);
            field = mysql_fetch_field(result_metadata);
        }
        /*
         * If we still have an object field or a MySQL field then the number of
         * fields in both is mismatching and we should return an error.
         */
        if (object_field || field) {
            mysql_free_result(result_metadata);
            __db_backend_mysql_finish(*statement);
            *statement = NULL;
            return LIBDBO_ERROR_UNKNOWN;
        }
    }
    if (result_metadata) {
        mysql_free_result(result_metadata);
    }

    return LIBDBO_OK;
}

/**
 * MySQL fetch function.
 *
 * Fetch the next row in a libdbo_backend_mysql_statement_t.
 */
static inline int __db_backend_mysql_fetch(libdbo_backend_mysql_statement_t* statement) {
    int ret;

    if (!statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!statement->statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Handle output binding if not already done.
     */
    if (!statement->bound) {
        if (statement->mysql_bind_output
            && mysql_stmt_bind_result(statement->statement, statement->mysql_bind_output))
        {
            libdbo_log(LIBDBO_LOG_ERROR, "MySQL statement bind error %d: %s",
                mysql_stmt_errno(statement->statement), mysql_stmt_error(statement->statement));
            return LIBDBO_ERROR_UNKNOWN;
        }
        statement->bound = 1;
    }

    /*
     * Fetch the next row.
     */
    ret = mysql_stmt_fetch(statement->statement);
    if (ret == 1) {
        libdbo_log(LIBDBO_LOG_ERROR, "MySQL statement fetch error %d: %s",
            mysql_stmt_errno(statement->statement), mysql_stmt_error(statement->statement));
        return LIBDBO_ERROR_UNKNOWN;
    }
    else if (ret == MYSQL_DATA_TRUNCATED) {
        int i;
        libdbo_backend_mysql_bind_t* bind;

        /*
         * Scan through all of the output binds and check where the data was
         * truncated and reallocate the buffer and try again. MySQL should have
         * updated bind->length with the required buffer size.
         *
         * We can really only retry fetch on string columns, if another type had
         * a too small buffer its more a programmable error in the prepare
         * function.
         */
        for (i = 0, bind = statement->bind_output; bind; i++, bind = bind->next) {
            if (bind->error) {
                if (statement->mysql_bind_output[i].buffer_type != MYSQL_TYPE_STRING
                    || bind->length <= statement->mysql_bind_output[i].buffer_length)
                {
                    return LIBDBO_ERROR_UNKNOWN;
                }

                free(statement->mysql_bind_output[i].buffer);
                statement->mysql_bind_output[i].buffer = NULL;
                if (!(statement->mysql_bind_output[i].buffer = calloc(1, bind->length))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                statement->mysql_bind_output[i].buffer_length = bind->length;
                bind->error = 0;
                if (mysql_stmt_fetch_column(statement->statement, &(statement->mysql_bind_output[i]), i, 0)
                    || bind->error)
                {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
        }
    }
    else if (ret == MYSQL_NO_DATA) {
        /*
         * Not really an error but we need to indicate that there is no more
         * data some how.
         */
        return LIBDBO_ERROR_UNKNOWN;
    }
    else if (ret) {
        libdbo_log(LIBDBO_LOG_ERROR, "MySQL fetch UNKNOWN %d error %d: %s",
            ret, mysql_stmt_errno(statement->statement), mysql_stmt_error(statement->statement));
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

/**
 * MySQL execute function.
 *
 * Execute a prepared statement in the libdbo_backend_mysql_statement_t.
 */
static inline int __db_backend_mysql_execute(libdbo_backend_mysql_statement_t* statement) {
    if (!statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!statement->statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Bind the input parameters.
     */
    if (statement->mysql_bind_input
        && mysql_stmt_bind_param(statement->statement, statement->mysql_bind_input))
    {
        libdbo_log(LIBDBO_LOG_ERROR, "MySQL execute statement bind error %d: %s",
            mysql_stmt_errno(statement->statement), mysql_stmt_error(statement->statement));
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Execute the statement.
     */
    if (mysql_stmt_execute(statement->statement)) {
        libdbo_log(LIBDBO_LOG_ERROR, "MySQL execute statement error %d: %s",
            mysql_stmt_errno(statement->statement), mysql_stmt_error(statement->statement));
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_mysql_initialize(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;

    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!__mysql_initialized) {
        if (mysql_library_init(0, NULL, NULL)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        __mysql_initialized = 1;
    }
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_shutdown(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;

    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__mysql_initialized) {
        mysql_library_end();
        __mysql_initialized = 0;
    }
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_connect(void* data, const libdbo_configuration_list_t* configuration_list) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    const libdbo_configuration_t* host;
    const libdbo_configuration_t* user;
    const libdbo_configuration_t* pass;
    const libdbo_configuration_t* db;
    const libdbo_configuration_t* port_configuration;
    const libdbo_configuration_t* timeout_configuration;
    int timeout;
    unsigned int port = 0;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_mysql->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    host = libdbo_configuration_list_find(configuration_list, "host");
    user = libdbo_configuration_list_find(configuration_list, "user");
    pass = libdbo_configuration_list_find(configuration_list, "pass");
    db = libdbo_configuration_list_find(configuration_list, "db");
    port_configuration = libdbo_configuration_list_find(configuration_list, "port");
    if (port_configuration) {
        port = atoi(libdbo_configuration_value(port_configuration));
    }

    backend_mysql->timeout = LIBDBO_BACKEND_MYSQL_DEFAULT_TIMEOUT;
    if ((timeout_configuration = libdbo_configuration_list_find(configuration_list, "timeout"))) {
        timeout = atoi(libdbo_configuration_value(timeout_configuration));
        if (timeout < 1) {
            backend_mysql->timeout = LIBDBO_BACKEND_MYSQL_DEFAULT_TIMEOUT;
        }
        else {
            backend_mysql->timeout = (unsigned int)timeout;
        }
    }

    if (!(backend_mysql->db = mysql_init(NULL))
        || mysql_options(backend_mysql->db, MYSQL_OPT_CONNECT_TIMEOUT, &backend_mysql->timeout)
        || !mysql_real_connect(backend_mysql->db,
            (host ? libdbo_configuration_value(host) : NULL),
            (user ? libdbo_configuration_value(user) : NULL),
            (pass ? libdbo_configuration_value(pass) : NULL),
            (db ? libdbo_configuration_value(db) : NULL),
            port,
            NULL,
            0)
        || mysql_autocommit(backend_mysql->db, 1))
    {
        if (backend_mysql->db) {
            libdbo_log(LIBDBO_LOG_ERROR, "MySQL connection error %d: %s",
                mysql_errno(backend_mysql->db), mysql_error(backend_mysql->db));
            mysql_close(backend_mysql->db);
            backend_mysql->db = NULL;
        }
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_mysql_disconnect(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (backend_mysql->transaction) {
        libdbo_backend_mysql_transaction_rollback(backend_mysql);
    }

    mysql_close(backend_mysql->db);
    backend_mysql->db = NULL;

    return LIBDBO_OK;
}

/**
 * Build the clause/WHERE SQL and append it to `sqlp`, how much that is left in
 * the buffer pointed by `sqlp` is specified by `left`.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[in] sqlp a character pointer pointer.
 * \param[in] left an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_mysql_build_clause(const libdbo_object_t* object, const libdbo_clause_list_t* clause_list, char** sqlp, int* left) {
    const libdbo_clause_t* clause;
    int first, ret;

    if (!clause_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!sqlp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*sqlp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (*left < 1) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    first = 1;
    while (clause) {
        if (first) {
            first = 0;
        }
        else {
            switch (libdbo_clause_operator(clause)) {
            case LIBDBO_CLAUSE_OPERATOR_AND:
                if ((ret = snprintf(*sqlp, *left, " AND")) >= *left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_CLAUSE_OPERATOR_OR:
                if ((ret = snprintf(*sqlp, *left, " OR")) >= *left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            default:
                return LIBDBO_ERROR_UNKNOWN;
            }
            *sqlp += ret;
            *left -= ret;
        }

        switch (libdbo_clause_type(clause)) {
        case LIBDBO_CLAUSE_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s = ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_NOT_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s != ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_THEN:
            if ((ret = snprintf(*sqlp, *left, " %s.%s < ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_OR_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s <= ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_OR_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s >= ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_THEN:
            if ((ret = snprintf(*sqlp, *left, " %s.%s > ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NULL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s IS NULL",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NOT_NULL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s IS NOT NULL",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_NESTED:
            if ((ret = snprintf(*sqlp, *left, " (")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *sqlp += ret;
            *left -= ret;
            if (__db_backend_mysql_build_clause(object, libdbo_clause_list(clause), sqlp, left)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*sqlp, *left, " )")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        *sqlp += ret;
        *left -= ret;

        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

/**
 * Bind values from the clause list to a MySQL bind structure.
 * TODO
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_mysql_bind_clause(libdbo_backend_mysql_bind_t** bind, const libdbo_clause_list_t* clause_list) {
    const libdbo_clause_t* clause;
    const libdbo_type_int32_t* int32;
    const libdbo_type_uint32_t* uint32;
    const libdbo_type_int64_t* int64;
    const libdbo_type_uint64_t* uint64;
    const char* text;

    if (!bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!clause_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        if (!*bind) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        (*bind)->bind->length = &((*bind)->bind->buffer_length);
        (*bind)->bind->is_null = (my_bool*)0;

        switch (libdbo_clause_type(clause)) {
        case LIBDBO_CLAUSE_EQUAL:
        case LIBDBO_CLAUSE_NOT_EQUAL:
        case LIBDBO_CLAUSE_LESS_THEN:
        case LIBDBO_CLAUSE_LESS_OR_EQUAL:
        case LIBDBO_CLAUSE_GREATER_OR_EQUAL:
        case LIBDBO_CLAUSE_GREATER_THEN:
            switch (libdbo_value_type(libdbo_clause_value(clause))) {
            case LIBDBO_TYPE_PRIMARY_KEY:
            case LIBDBO_TYPE_INT32:
                if (!(int32 = libdbo_value_int32(libdbo_clause_value(clause)))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_LONG;
                (*bind)->bind->buffer = (void*)int32;
                (*bind)->bind->buffer_length = sizeof(libdbo_type_int32_t);
                (*bind)->bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_UINT32:
                if (!(uint32 = libdbo_value_uint32(libdbo_clause_value(clause)))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_LONG;
                (*bind)->bind->buffer = (void*)uint32;
                (*bind)->bind->buffer_length = sizeof(libdbo_type_uint32_t);
                (*bind)->bind->is_unsigned = 1;
                break;

            case LIBDBO_TYPE_INT64:
                if (!(int64 = libdbo_value_int64(libdbo_clause_value(clause)))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_LONGLONG;
                (*bind)->bind->buffer = (void*)int64;
                (*bind)->bind->buffer_length = sizeof(libdbo_type_int64_t);
                (*bind)->bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_UINT64:
                if (!(uint64 = libdbo_value_uint64(libdbo_clause_value(clause)))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_LONGLONG;
                (*bind)->bind->buffer = (void*)uint64;
                (*bind)->bind->buffer_length = sizeof(libdbo_type_uint64_t);
                (*bind)->bind->is_unsigned = 1;
                break;

            case LIBDBO_TYPE_TEXT:
                if (!(text = libdbo_value_text(libdbo_clause_value(clause)))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_STRING;
                (*bind)->bind->buffer = (void*)text;
                (*bind)->bind->buffer_length = strlen(text);
                (*bind)->bind->is_unsigned = 0;
                break;

            case LIBDBO_TYPE_ENUM:
                if (libdbo_value_enum_value(libdbo_clause_value(clause), &((*bind)->value_enum))) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                (*bind)->bind->buffer_type = MYSQL_TYPE_LONG;
                (*bind)->bind->buffer = (void*)&((*bind)->value_enum);
                (*bind)->bind->buffer_length = sizeof(int);
                (*bind)->bind->is_unsigned = 0;
                break;

            default:
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NULL:
            /* TODO: is null */
            break;

        case LIBDBO_CLAUSE_IS_NOT_NULL:
            /* TODO: is not null */
            break;

        case LIBDBO_CLAUSE_NESTED:
            *bind = (*bind)->next;
            if (__db_backend_mysql_bind_clause(bind, libdbo_clause_list(clause))) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            clause = libdbo_clause_next(clause);
            continue;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }

        *bind = (*bind)->next;
        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

static int __db_backend_mysql_bind_value(libdbo_backend_mysql_bind_t* bind, const libdbo_value_t* value) {
    const libdbo_type_int32_t* int32;
    const libdbo_type_uint32_t* uint32;
    const libdbo_type_int64_t* int64;
    const libdbo_type_uint64_t* uint64;
    const char* text;

    if (!bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!bind->bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    bind->bind->length = &(bind->bind->buffer_length);
    bind->bind->is_null = (my_bool*)0;

    switch (libdbo_value_type(value)) {
    case LIBDBO_TYPE_PRIMARY_KEY:
    case LIBDBO_TYPE_INT32:
        if (!(int32 = libdbo_value_int32(value))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_LONG;
        bind->bind->buffer = (void*)int32;
        bind->bind->buffer_length = sizeof(libdbo_type_int32_t);
        bind->bind->is_unsigned = 0;
        break;

    case LIBDBO_TYPE_UINT32:
        if (!(uint32 = libdbo_value_uint32(value))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_LONG;
        bind->bind->buffer = (void*)uint32;
        bind->bind->buffer_length = sizeof(libdbo_type_uint32_t);
        bind->bind->is_unsigned = 1;
        break;

    case LIBDBO_TYPE_INT64:
        if (!(int64 = libdbo_value_int64(value))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->bind->buffer = (void*)int64;
        bind->bind->buffer_length = sizeof(libdbo_type_int64_t);
        bind->bind->is_unsigned = 0;
        break;

    case LIBDBO_TYPE_UINT64:
        if (!(uint64 = libdbo_value_uint64(value))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->bind->buffer = (void*)uint64;
        bind->bind->buffer_length = sizeof(libdbo_type_uint64_t);
        bind->bind->is_unsigned = 1;
        break;

    case LIBDBO_TYPE_TEXT:
        if (!(text = libdbo_value_text(value))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_STRING;
        bind->bind->buffer = (void*)text;
        bind->bind->buffer_length = strlen(text);
        bind->bind->is_unsigned = 0;
        break;

    case LIBDBO_TYPE_ENUM:
        if (libdbo_value_enum_value(value, &(bind->value_enum))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        bind->bind->buffer_type = MYSQL_TYPE_LONG;
        bind->bind->buffer = (void*)&(bind->value_enum);
        bind->bind->buffer_length = sizeof(int);
        bind->bind->is_unsigned = 0;
        break;

    default:
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int __db_backend_mysql_bind_value_set(libdbo_backend_mysql_bind_t** bind, const libdbo_value_set_t* value_set) {
    size_t i;

    if (!bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    for (i = 0; i < libdbo_value_set_size(value_set); i++, *bind = (*bind)->next) {
        if (!*bind) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        if (__db_backend_mysql_bind_value(*bind, libdbo_value_set_at(value_set, i))) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }
    return LIBDBO_OK;
}

static libdbo_result_t* libdbo_backend_mysql_next(void* data, int finish) {
    libdbo_backend_mysql_statement_t* statement = (libdbo_backend_mysql_statement_t*)data;
    libdbo_result_t* result = NULL;
    libdbo_value_set_t* value_set = NULL;
    const libdbo_object_field_t* object_field;
    libdbo_backend_mysql_bind_t* bind;
    int value;

    if (!statement) {
        return NULL;
    }
    if (!statement->object_field_list) {
        return NULL;
    }
    if (!statement->statement) {
        return NULL;
    }

    if (finish) {
        __db_backend_mysql_finish(statement);
        return NULL;
    }

    if (__db_backend_mysql_fetch(statement)) {
        return NULL;
    }

    if (!(result = libdbo_result_new())
        || !(value_set = libdbo_value_set_new(statement->fields))
        || libdbo_result_set_value_set(result, value_set))
    {
        libdbo_result_free(result);
        libdbo_value_set_free(value_set);
        return NULL;
    }
    object_field = libdbo_object_field_list_begin(statement->object_field_list);
    bind = statement->bind_output;
    value = 0;
    while (object_field) {
        if (!bind || !bind->bind || !bind->bind->buffer) {
            libdbo_result_free(result);
            return NULL;
        }

        switch (libdbo_object_field_type(object_field)) {
        case LIBDBO_TYPE_PRIMARY_KEY:
        case LIBDBO_TYPE_ANY:
        case LIBDBO_TYPE_REVISION:
            switch (bind->bind->buffer_type) {
            case MYSQL_TYPE_LONG:
                if ((bind->bind->is_unsigned
                        && libdbo_value_from_uint32(libdbo_value_set_get(value_set, value), *((libdbo_type_uint32_t*)bind->bind->buffer)))
                    || (!bind->bind->is_unsigned
                        && libdbo_value_from_int32(libdbo_value_set_get(value_set, value), *((libdbo_type_int32_t*)bind->bind->buffer))))
                {
                    libdbo_result_free(result);
                    return NULL;
                }
                break;

            case MYSQL_TYPE_LONGLONG:
                if ((bind->bind->is_unsigned
                        && libdbo_value_from_uint64(libdbo_value_set_get(value_set, value), *((libdbo_type_uint64_t*)bind->bind->buffer)))
                    || (!bind->bind->is_unsigned
                        && libdbo_value_from_int64(libdbo_value_set_get(value_set, value), *((libdbo_type_int64_t*)bind->bind->buffer))))
                {
                    libdbo_result_free(result);
                    return NULL;
                }
                break;

            case MYSQL_TYPE_STRING:
                if ((!bind->length
                        && libdbo_value_from_text(libdbo_value_set_get(value_set, value), ""))
                    || (bind->length
                        && libdbo_value_from_text2(libdbo_value_set_get(value_set, value), (char*)bind->bind->buffer, bind->length)))
                {
                    libdbo_result_free(result);
                    return NULL;
                }
                break;

            default:
                libdbo_result_free(result);
                return NULL;
            }
            if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_PRIMARY_KEY
                && libdbo_value_set_primary_key(libdbo_value_set_get(value_set, value)))
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
            if (bind->bind->buffer_type != MYSQL_TYPE_LONG
                || (bind->bind->is_unsigned
                    && libdbo_value_from_uint32(libdbo_value_set_get(value_set, value), *((libdbo_type_uint32_t*)bind->bind->buffer)))
                || (!bind->bind->is_unsigned
                    && libdbo_value_from_int32(libdbo_value_set_get(value_set, value), *((libdbo_type_int32_t*)bind->bind->buffer))))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_INT64:
        case LIBDBO_TYPE_UINT64:
            if (bind->bind->buffer_type != MYSQL_TYPE_LONGLONG
                || (bind->bind->is_unsigned
                    && libdbo_value_from_uint64(libdbo_value_set_get(value_set, value), *((libdbo_type_uint64_t*)bind->bind->buffer)))
                || (!bind->bind->is_unsigned
                    && libdbo_value_from_int64(libdbo_value_set_get(value_set, value), *((libdbo_type_int64_t*)bind->bind->buffer))))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            if (bind->bind->buffer_type != MYSQL_TYPE_STRING
                || (!bind->length
                    && libdbo_value_from_text(libdbo_value_set_get(value_set, value), ""))
                || (bind->length
                    && libdbo_value_from_text2(libdbo_value_set_get(value_set, value), (char*)bind->bind->buffer, bind->length)))
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
        value++;
        bind = bind->next;
    }
    return result;
}

static int libdbo_backend_mysql_create(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_object_field_t* revision_field = NULL;
    char sql[4*1024];
    char* sqlp;
    int ret, left, first;
    libdbo_backend_mysql_statement_t* statement = NULL;
    libdbo_backend_mysql_bind_t* bind;
    libdbo_value_t revision = LIBDBO_VALUE_EMPTY;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
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
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }

    left = sizeof(sql);
    sqlp = sql;

    if (!libdbo_object_field_list_begin(object_field_list) && !revision_field) {
        /*
         * Special case when tables has no fields except maybe a primary key.
         */
        if ((ret = snprintf(sqlp, left, "INSERT INTO %s () VALUES ()", libdbo_object_table(object))) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;
    }
    else {
        if ((ret = snprintf(sqlp, left, "INSERT INTO %s (", libdbo_object_table(object))) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;

        /*
         * Add the fields from the given object_field_list.
         */
        object_field = libdbo_object_field_list_begin(object_field_list);
        first = 1;
        while (object_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " %s", libdbo_object_field_name(object_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", %s", libdbo_object_field_name(object_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;

            object_field = libdbo_object_field_next(object_field);
        }

        /*
         * Add the revision field if we have one.
         */
        if (revision_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " %s", libdbo_object_field_name(revision_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", %s", libdbo_object_field_name(revision_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;
        }

        if ((ret = snprintf(sqlp, left, " ) VALUES (")) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;

        /*
         * Mark all the fields for binding from the object_field_list.
         */
        object_field = libdbo_object_field_list_begin(object_field_list);
        first = 1;
        while (object_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;

            object_field = libdbo_object_field_next(object_field);
        }

        /*
         * Mark revision field for binding if we have one.
         */
        if (revision_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;
        }

        if ((ret = snprintf(sqlp, left, " )")) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;
    }

    /*
     * Prepare the SQL, create a MySQL statement.
     */
    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), libdbo_object_object_field_list(object))
        || !statement
        || !(bind = statement->bind_input))
    {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Bind all the values from value_set.
     */
    if (__db_backend_mysql_bind_value_set(&bind, value_set)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Bind the revision field value if we have one.
     */
    if (revision_field) {
        if (libdbo_value_from_int64(&revision, 1)
            || __db_backend_mysql_bind_value(bind, &revision))
        {
            libdbo_value_reset(&revision);
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
        libdbo_value_reset(&revision);
    }

    /*
     * Execute the SQL.
     */
    if (__db_backend_mysql_execute(statement)
        || mysql_stmt_affected_rows(statement->statement) != 1)
    {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_mysql_finish(statement);

    return LIBDBO_OK;
}

static libdbo_result_list_t* libdbo_backend_mysql_read(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_join_t* join;
    char sql[4*1024];
    char* sqlp;
    int ret, left, first;
    libdbo_result_list_t* result_list;
    libdbo_backend_mysql_statement_t* statement = NULL;
    libdbo_backend_mysql_bind_t* bind;

    if (!__mysql_initialized) {
        return NULL;
    }
    if (!backend_mysql) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "SELECT")) >= left) {
        return NULL;
    }
    sqlp += ret;
    left -= ret;

    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    first = 1;
    while (object_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s.%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                return NULL;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s.%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                return NULL;
            }
        }
        sqlp += ret;
        left -= ret;

        object_field = libdbo_object_field_next(object_field);
    }

    if ((ret = snprintf(sqlp, left, " FROM %s", libdbo_object_table(object))) >= left) {
        return NULL;
    }
    sqlp += ret;
    left -= ret;

    if (join_list) {
        join = libdbo_join_list_begin(join_list);
        while (join) {
            if ((ret = snprintf(sqlp, left, " INNER JOIN %s ON %s.%s = %s.%s",
                libdbo_join_to_table(join),
                libdbo_join_to_table(join),
                libdbo_join_to_field(join),
                libdbo_join_from_table(join),
                libdbo_join_from_field(join))) >= left)
            {
                return NULL;
            }
            sqlp += ret;
            left -= ret;
            join = libdbo_join_next(join);
        }
    }

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return NULL;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_mysql_build_clause(object, clause_list, &sqlp, &left)) {
            return NULL;
        }
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), libdbo_object_object_field_list(object))
        || !statement)
    {
        __db_backend_mysql_finish(statement);
        return NULL;
    }

    bind = statement->bind_input;

    if (clause_list) {
        if (__db_backend_mysql_bind_clause(&bind, clause_list)) {
            __db_backend_mysql_finish(statement);
            return NULL;
        }
    }

    /*
     * Execute the SQL.
     */
    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return NULL;
    }

    if (!(result_list = libdbo_result_list_new())
        || libdbo_result_list_set_next(result_list, libdbo_backend_mysql_next, statement, mysql_stmt_affected_rows(statement->statement)))
    {
        libdbo_result_list_free(result_list);
        __db_backend_mysql_finish(statement);
        return NULL;
    }
    return result_list;
}

static int libdbo_backend_mysql_update(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_object_field_t* revision_field = NULL;
    const libdbo_clause_t* clause;
    const libdbo_clause_t* revision_clause = NULL;
    libdbo_type_int64_t revision_number = -1;
    char sql[4*1024];
    char* sqlp;
    int ret, left, first;
    libdbo_backend_mysql_statement_t* statement = NULL;
    libdbo_backend_mysql_bind_t* bind;
    libdbo_value_t revision = LIBDBO_VALUE_EMPTY;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
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
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }
    if (revision_field) {
        /*
         * If we have a revision field we should also have it in the clause,
         * find it and get the value for later use or return error if not found.
         */
        clause = libdbo_clause_list_begin(clause_list);
        while (clause) {
            if (!strcmp(libdbo_clause_field(clause), libdbo_object_field_name(revision_field))) {
                revision_clause = clause;
                break;
            }
            clause = libdbo_clause_next(clause);
        }
        if (!revision_clause) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        switch (libdbo_value_type(libdbo_clause_value(revision_clause))) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(libdbo_clause_value(revision_clause), &int32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = int32;
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(libdbo_clause_value(revision_clause), &uint32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = uint32;
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(libdbo_clause_value(revision_clause), &int64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = int64;
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(libdbo_clause_value(revision_clause), &uint64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = uint64;
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "UPDATE %s SET", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    /*
     * Build the update SQL from the object_field_list.
     */
    object_field = libdbo_object_field_list_begin(object_field_list);
    first = 1;
    while (object_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s = ?", libdbo_object_field_name(object_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s = ?", libdbo_object_field_name(object_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        sqlp += ret;
        left -= ret;

        object_field = libdbo_object_field_next(object_field);
    }

    /*
     * Add a new revision if we have any.
     */
    if (revision_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s = ?", libdbo_object_field_name(revision_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s = ?", libdbo_object_field_name(revision_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        sqlp += ret;
        left -= ret;
    }

    /*
     * Build the clauses.
     */
    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_mysql_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Prepare the SQL.
     */
    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), libdbo_object_object_field_list(object))
        || !statement)
    {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    bind = statement->bind_input;

    /*
     * Bind all the values from value_set.
     */
    if (value_set) {
        if (__db_backend_mysql_bind_value_set(&bind, value_set)) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Bind the new revision if we have any.
     */
    if (revision_field) {
        if (libdbo_value_from_int64(&revision, revision_number + 1)
            || __db_backend_mysql_bind_value(bind, &revision))
        {
            libdbo_value_reset(&revision);
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
        libdbo_value_reset(&revision);

        if (bind) {
            bind = bind->next;
        }
    }

    /*
     * Bind the clauses values.
     */
    if (clause_list) {
        if (__db_backend_mysql_bind_clause(&bind, clause_list)) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Execute the SQL.
     */
    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * If we are using revision we have to have a positive number of changes
     * otherwise its a failure.
     */
    if (revision_field) {
        if (mysql_stmt_affected_rows(statement->statement) < 1) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    __db_backend_mysql_finish(statement);
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_delete(void* data, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    char sql[4*1024];
    char* sqlp;
    int ret, left;
    const libdbo_object_field_t* revision_field = NULL;
    const libdbo_object_field_t* object_field;
    const libdbo_clause_t* clause;
    libdbo_backend_mysql_statement_t* statement = NULL;
    libdbo_backend_mysql_bind_t* bind;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }
    if (revision_field) {
        /*
         * If we have a revision field we should also have it in the clause,
         * find it or return error if not found.
         */
        clause = libdbo_clause_list_begin(clause_list);
        while (clause) {
            if (!strcmp(libdbo_clause_field(clause), libdbo_object_field_name(revision_field))) {
                break;
            }
            clause = libdbo_clause_next(clause);
        }
        if (!clause) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "DELETE FROM %s", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_mysql_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), libdbo_object_object_field_list(object))
        || !statement)
    {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    bind = statement->bind_input;

    if (clause_list) {
        if (__db_backend_mysql_bind_clause(&bind, clause_list)) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * If we are using revision we have to have a positive number of changes
     * otherwise its a failure.
     */
    if (revision_field) {
        if (mysql_stmt_affected_rows(statement->statement) < 1) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    __db_backend_mysql_finish(statement);
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_count(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    const libdbo_join_t* join;
    char sql[4*1024];
    char* sqlp;
    int ret, left;
    libdbo_backend_mysql_statement_t* statement = NULL;
    libdbo_backend_mysql_bind_t* bind;
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field = NULL;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!count) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "SELECT COUNT(*)")) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if ((ret = snprintf(sqlp, left, " FROM %s", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if (join_list) {
        join = libdbo_join_list_begin(join_list);
        while (join) {
            if ((ret = snprintf(sqlp, left, " INNER JOIN %s ON %s.%s = %s.%s",
                libdbo_join_to_table(join),
                libdbo_join_to_table(join),
                libdbo_join_to_field(join),
                libdbo_join_from_table(join),
                libdbo_join_from_field(join))) >= left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
            join = libdbo_join_next(join);
        }
    }

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_mysql_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (!(object_field_list = libdbo_object_field_list_new())
        || !(object_field = libdbo_object_field_new())
        || libdbo_object_field_set_name(object_field, "countField")
        || libdbo_object_field_set_type(object_field, LIBDBO_TYPE_UINT32)
        || libdbo_object_field_list_add(object_field_list, object_field))
    {
        libdbo_object_field_free(object_field);
        libdbo_object_field_list_free(object_field_list);
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), object_field_list)
        || !statement)
    {
        libdbo_object_field_list_free(object_field_list);
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    libdbo_object_field_list_free(object_field_list);

    bind = statement->bind_input;

    if (clause_list) {
        if (__db_backend_mysql_bind_clause(&bind, clause_list)) {
            __db_backend_mysql_finish(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_fetch(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    bind = statement->bind_output;
    if (!bind || !bind->bind || !bind->bind->buffer
        || bind->bind->buffer_type != MYSQL_TYPE_LONG
        || !bind->bind->is_unsigned
        || bind->length != sizeof(libdbo_type_uint32_t))
    {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    *count = *((libdbo_type_uint32_t*)bind->bind->buffer);
    __db_backend_mysql_finish(statement);

    return LIBDBO_OK;
}

static void libdbo_backend_mysql_free(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;

    if (backend_mysql) {
        if (backend_mysql->db) {
            (void)libdbo_backend_mysql_disconnect(backend_mysql);
        }
        libdbo_mm_delete(&__mysql_alloc, backend_mysql);
    }
}

static int libdbo_backend_mysql_transaction_begin(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    static const char* sql = "BEGIN TRANSACTION";
    libdbo_backend_mysql_statement_t* statement = NULL;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_mysql->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), NULL)) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_mysql_finish(statement);

    backend_mysql->transaction = 1;
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_transaction_commit(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    static const char* sql = "COMMIT TRANSACTION";
    libdbo_backend_mysql_statement_t* statement = NULL;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), NULL)) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_mysql_finish(statement);

    backend_mysql->transaction = 0;
    return LIBDBO_OK;
}

static int libdbo_backend_mysql_transaction_rollback(void* data) {
    libdbo_backend_mysql_t* backend_mysql = (libdbo_backend_mysql_t*)data;
    static const char* sql = "ROLLBACK TRANSACTION";
    libdbo_backend_mysql_statement_t* statement = NULL;

    if (!__mysql_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_mysql->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_prepare(backend_mysql, &statement, sql, strlen(sql), NULL)) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_mysql_execute(statement)) {
        __db_backend_mysql_finish(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_mysql_finish(statement);

    backend_mysql->transaction = 0;
    return LIBDBO_OK;
}

libdbo_backend_handle_t* libdbo_backend_mysql_new_handle(void) {
    libdbo_backend_handle_t* backend_handle = NULL;
    libdbo_backend_mysql_t* backend_mysql =
        (libdbo_backend_mysql_t*)libdbo_mm_new0(&__mysql_alloc);

    if (backend_mysql && (backend_handle = libdbo_backend_handle_new())) {
        if (libdbo_backend_handle_set_data(backend_handle, (void*)backend_mysql)
            || libdbo_backend_handle_set_initialize(backend_handle, libdbo_backend_mysql_initialize)
            || libdbo_backend_handle_set_shutdown(backend_handle, libdbo_backend_mysql_shutdown)
            || libdbo_backend_handle_set_connect(backend_handle, libdbo_backend_mysql_connect)
            || libdbo_backend_handle_set_disconnect(backend_handle, libdbo_backend_mysql_disconnect)
            || libdbo_backend_handle_set_create(backend_handle, libdbo_backend_mysql_create)
            || libdbo_backend_handle_set_read(backend_handle, libdbo_backend_mysql_read)
            || libdbo_backend_handle_set_update(backend_handle, libdbo_backend_mysql_update)
            || libdbo_backend_handle_set_delete(backend_handle, libdbo_backend_mysql_delete)
            || libdbo_backend_handle_set_count(backend_handle, libdbo_backend_mysql_count)
            || libdbo_backend_handle_set_free(backend_handle, libdbo_backend_mysql_free)
            || libdbo_backend_handle_set_transaction_begin(backend_handle, libdbo_backend_mysql_transaction_begin)
            || libdbo_backend_handle_set_transaction_commit(backend_handle, libdbo_backend_mysql_transaction_commit)
            || libdbo_backend_handle_set_transaction_rollback(backend_handle, libdbo_backend_mysql_transaction_rollback))
        {
            libdbo_backend_handle_free(backend_handle);
            libdbo_mm_delete(&__mysql_alloc, backend_mysql);
            return NULL;
        }
    }
    return backend_handle;
}
