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
 * Based on enforcer-ng/src/db/db_backend.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "config.h"

#include "libdbo/backend.h"
#if defined(HAVE_SQLITE3)
#include "libdbo/backend/sqlite.h"
#endif
#if defined(HAVE_COUCHDB)
#include "libdbo/backend/couchdb.h"
#endif
#if defined(HAVE_MYSQL)
#include "libdbo/backend/mysql.h"
#endif
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <stdlib.h>
#include <string.h>

/* DB BACKEND HANDLE */

static libdbo_mm_t __backend_handle_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_backend_handle_t));

libdbo_backend_handle_t* libdbo_backend_handle_new(void) {
    libdbo_backend_handle_t* backend_handle =
        (libdbo_backend_handle_t*)libdbo_mm_new0(&__backend_handle_alloc);

    return backend_handle;
}

void libdbo_backend_handle_free(libdbo_backend_handle_t* backend_handle) {
    if (backend_handle) {
        if (backend_handle->disconnect_function) {
            (void)(*backend_handle->disconnect_function)(backend_handle->data);
        }
        if (backend_handle->free_function) {
            (*backend_handle->free_function)(backend_handle->data);
        }
        libdbo_mm_delete(&__backend_handle_alloc, backend_handle);
    }
}

int libdbo_backend_handle_initialize(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->initialize_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->initialize_function((void*)backend_handle->data);
}

int libdbo_backend_handle_shutdown(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->shutdown_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->shutdown_function((void*)backend_handle->data);
}

int libdbo_backend_handle_connect(const libdbo_backend_handle_t* backend_handle, const libdbo_configuration_list_t* configuration_list) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!configuration_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->connect_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->connect_function((void*)backend_handle->data, configuration_list);
}

int libdbo_backend_handle_disconnect(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->disconnect_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->disconnect_function((void*)backend_handle->data);
}

int libdbo_backend_handle_create(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->create_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->create_function((void*)backend_handle->data, object, object_field_list, value_set);
}

libdbo_result_list_t* libdbo_backend_handle_read(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    if (!backend_handle) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }
    if (!backend_handle->read_function) {
        return NULL;
    }

    return backend_handle->read_function((void*)backend_handle->data, object, join_list, clause_list);
}

int libdbo_backend_handle_update(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->update_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->update_function((void*)backend_handle->data, object, object_field_list, value_set, clause_list);
}

int libdbo_backend_handle_delete(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->delete_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->delete_function((void*)backend_handle->data, object, clause_list);
}

int libdbo_backend_handle_count(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!count) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->count_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->count_function((void*)backend_handle->data, object, join_list, clause_list, count);
}

int libdbo_backend_handle_transaction_begin(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_begin_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->transaction_begin_function((void*)backend_handle->data);
}

int libdbo_backend_handle_transaction_commit(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_commit_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->transaction_commit_function((void*)backend_handle->data);
}

int libdbo_backend_handle_transaction_rollback(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_rollback_function) {
        return DB_ERROR_UNKNOWN;
    }

    return backend_handle->transaction_rollback_function((void*)backend_handle->data);
}

const void* libdbo_backend_handle_data(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return NULL;
    }

    return backend_handle->data;
}

int libdbo_backend_handle_set_initialize(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_initialize_t initialize_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->initialize_function = initialize_function;
    return DB_OK;
}

int libdbo_backend_handle_set_shutdown(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_shutdown_t shutdown_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->shutdown_function = shutdown_function;
    return DB_OK;
}

int libdbo_backend_handle_set_connect(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_connect_t connect_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->connect_function = connect_function;
    return DB_OK;
}

int libdbo_backend_handle_set_disconnect(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_disconnect_t disconnect_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->disconnect_function = disconnect_function;
    return DB_OK;
}

int libdbo_backend_handle_set_create(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_create_t create_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->create_function = create_function;
    return DB_OK;
}

int libdbo_backend_handle_set_read(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_read_t read_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->read_function = read_function;
    return DB_OK;
}

int libdbo_backend_handle_set_update(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_update_t update_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->update_function = update_function;
    return DB_OK;
}

int libdbo_backend_handle_set_delete(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_delete_t delete_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->delete_function = delete_function;
    return DB_OK;
}

int libdbo_backend_handle_set_count(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_count_t count_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->count_function = count_function;
    return DB_OK;
}

int libdbo_backend_handle_set_free(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_free_t free_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->free_function = free_function;
    return DB_OK;
}

int libdbo_backend_handle_set_transaction_begin(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_begin_t transaction_begin_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->transaction_begin_function = transaction_begin_function;
    return DB_OK;
}

int libdbo_backend_handle_set_transaction_commit(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_commit_t transaction_commit_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->transaction_commit_function = transaction_commit_function;
    return DB_OK;
}

int libdbo_backend_handle_set_transaction_rollback(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_rollback_t transaction_rollback_function) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->transaction_rollback_function = transaction_rollback_function;
    return DB_OK;
}

int libdbo_backend_handle_set_data(libdbo_backend_handle_t* backend_handle, void* data) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (backend_handle->data) {
        return DB_ERROR_UNKNOWN;
    }

    backend_handle->data = data;
    return DB_OK;
}

int libdbo_backend_handle_not_empty(const libdbo_backend_handle_t* backend_handle) {
    if (!backend_handle) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->initialize_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->shutdown_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->connect_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->disconnect_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->create_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->read_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->update_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->count_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->delete_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->free_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_begin_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_commit_function) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_handle->transaction_rollback_function) {
        return DB_ERROR_UNKNOWN;
    }
    return DB_OK;
}

/* DB BACKEND */

static libdbo_mm_t __backend_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_backend_t));

libdbo_backend_t* libdbo_backend_new(void) {
    libdbo_backend_t* backend =
        (libdbo_backend_t*)libdbo_mm_new0(&__backend_alloc);

    return backend;
}

void libdbo_backend_free(libdbo_backend_t* backend) {
    if (backend) {
        if (backend->handle) {
            libdbo_backend_handle_free(backend->handle);
        }
        if (backend->name) {
            free(backend->name);
        }
        libdbo_mm_delete(&__backend_alloc, backend);
    }
}

const char* libdbo_backend_name(const libdbo_backend_t* backend) {
    if (!backend) {
        return NULL;
    }

    return backend->name;
}

const libdbo_backend_handle_t* libdbo_backend_handle(const libdbo_backend_t* backend) {
    if (!backend) {
        return NULL;
    }

    return backend->handle;
}

int libdbo_backend_set_name(libdbo_backend_t* backend, const char* name) {
    char* new_name;

    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }

    if (!(new_name = strdup(name))) {
        return DB_ERROR_UNKNOWN;
    }

    if (backend->name) {
        free(backend->name);
    }
    backend->name = new_name;
    return DB_OK;
}

int libdbo_backend_set_handle(libdbo_backend_t* backend, libdbo_backend_handle_t* handle) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    backend->handle = handle;
    return DB_OK;
}

int libdbo_backend_not_empty(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->name) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }
    return DB_OK;
}

int libdbo_backend_initialize(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_initialize(backend->handle);
}

int libdbo_backend_shutdown(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_shutdown(backend->handle);
}

int libdbo_backend_connect(const libdbo_backend_t* backend, const libdbo_configuration_list_t* configuration_list) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!configuration_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_connect(backend->handle, configuration_list);
}

int libdbo_backend_disconnect(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_disconnect(backend->handle);
}

int libdbo_backend_create(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_create(backend->handle, object, object_field_list, value_set);
}

libdbo_result_list_t* libdbo_backend_read(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    if (!backend) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }
    if (!backend->handle) {
        return NULL;
    }

    return libdbo_backend_handle_read(backend->handle, object, join_list, clause_list);
}

int libdbo_backend_update(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_update(backend->handle, object, object_field_list, value_set, clause_list);
}

int libdbo_backend_delete(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_delete(backend->handle, object, clause_list);
}

int libdbo_backend_count(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!count) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_count(backend->handle, object, join_list, clause_list, count);
}

int libdbo_backend_transaction_begin(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_transaction_begin(backend->handle);
}

int libdbo_backend_transaction_commit(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_transaction_commit(backend->handle);
}

int libdbo_backend_transaction_rollback(const libdbo_backend_t* backend) {
    if (!backend) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend->handle) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_backend_handle_transaction_rollback(backend->handle);
}

/* DB BACKEND FACTORY */

libdbo_backend_t* libdbo_backend_factory_get_backend(const char* name) {
    libdbo_backend_t* backend = NULL;

    if (!name) {
        return NULL;
    }

#if defined(HAVE_SQLITE3)
    if (!strcmp(name, "sqlite")) {
        if (!(backend = libdbo_backend_new())
            || libdbo_backend_set_name(backend, "sqlite")
            || libdbo_backend_set_handle(backend, libdbo_backend_sqlite_new_handle())
            || libdbo_backend_initialize(backend))
        {
            libdbo_backend_free(backend);
            return NULL;
        }
        return backend;
    }
#endif
#if defined(HAVE_COUCHDB)
    if (!strcmp(name, "couchdb")) {
        if (!(backend = libdbo_backend_new())
            || libdbo_backend_set_name(backend, "couchdb")
            || libdbo_backend_set_handle(backend, libdbo_backend_couchdb_new_handle())
            || libdbo_backend_initialize(backend))
        {
            libdbo_backend_free(backend);
            return NULL;
        }
        return backend;
    }
#endif
#if defined(HAVE_MYSQL)
    if (!strcmp(name, "mysql")) {
        if (!(backend = libdbo_backend_new())
            || libdbo_backend_set_name(backend, "mysql")
            || libdbo_backend_set_handle(backend, libdbo_backend_mysql_new_handle())
            || libdbo_backend_initialize(backend))
        {
            libdbo_backend_free(backend);
            return NULL;
        }
        return backend;
    }
#endif

    return backend;
}

int libdbo_backend_factory_shutdown(void) {
    libdbo_backend_t* backend;
    int ret = DB_OK;

#if defined(HAVE_SQLITE3)
    if (!(backend = libdbo_backend_new())
        || libdbo_backend_set_name(backend, "sqlite")
        || libdbo_backend_set_handle(backend, libdbo_backend_sqlite_new_handle())
        || libdbo_backend_shutdown(backend))
    {
        ret = DB_ERROR_UNKNOWN;
    }
    libdbo_backend_free(backend);
    backend = NULL;
#endif
#if defined(HAVE_COUCHDB)
    if (!(backend = libdbo_backend_new())
        || libdbo_backend_set_name(backend, "couchdb")
        || libdbo_backend_set_handle(backend, libdbo_backend_couchdb_new_handle())
        || libdbo_backend_shutdown(backend))
    {
        ret = DB_ERROR_UNKNOWN;
    }
    libdbo_backend_free(backend);
    backend = NULL;
#endif
#if defined(HAVE_MYSQL)
    if (!(backend = libdbo_backend_new())
        || libdbo_backend_set_name(backend, "mysql")
        || libdbo_backend_set_handle(backend, libdbo_backend_mysql_new_handle())
        || libdbo_backend_shutdown(backend))
    {
        ret = DB_ERROR_UNKNOWN;
    }
    libdbo_backend_free(backend);
    backend = NULL;
#endif

    return ret;
}

/* DB BACKEND META DATA */

static libdbo_mm_t __backend_meta_data_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_backend_meta_data_t));

libdbo_backend_meta_data_t* libdbo_backend_meta_data_new(void) {
    libdbo_backend_meta_data_t* backend_meta_data =
        (libdbo_backend_meta_data_t*)libdbo_mm_new0(&__backend_meta_data_alloc);

    return backend_meta_data;
}

libdbo_backend_meta_data_t* libdbo_backend_meta_data_new_copy(const libdbo_backend_meta_data_t* from_backend_meta_data) {
    libdbo_backend_meta_data_t* backend_meta_data;

    if (!from_backend_meta_data) {
        return NULL;
    }

    backend_meta_data = (libdbo_backend_meta_data_t*)libdbo_mm_new0(&__backend_meta_data_alloc);
    if (backend_meta_data) {
        if (libdbo_backend_meta_data_copy(backend_meta_data, from_backend_meta_data)) {
            libdbo_backend_meta_data_free(backend_meta_data);
            return NULL;
        }
    }

    return backend_meta_data;
}

void libdbo_backend_meta_data_free(libdbo_backend_meta_data_t* backend_meta_data) {
    if (backend_meta_data) {
        if (backend_meta_data->name) {
            free(backend_meta_data->name);
        }
        if (backend_meta_data->value) {
            libdbo_value_free(backend_meta_data->value);
        }
        libdbo_mm_delete(&__backend_meta_data_alloc, backend_meta_data);
    }
}

int libdbo_backend_meta_data_copy(libdbo_backend_meta_data_t* backend_meta_data, const libdbo_backend_meta_data_t* from_backend_meta_data) {
    if (!backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }
    if (!from_backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }

    if (backend_meta_data->name) {
        free(backend_meta_data->name);
        backend_meta_data->name = NULL;
    }
    if (from_backend_meta_data->name) {
        if (!(backend_meta_data->name = strdup(from_backend_meta_data->name))) {
            return DB_ERROR_UNKNOWN;
        }
    }

    if (from_backend_meta_data->value) {
        if (backend_meta_data->value) {
            libdbo_value_reset(backend_meta_data->value);
        }
        else {
            if (!(backend_meta_data->value = libdbo_value_new())) {
                return DB_ERROR_UNKNOWN;
            }
        }
        if (libdbo_value_copy(backend_meta_data->value, from_backend_meta_data->value)) {
            return DB_ERROR_UNKNOWN;
        }
    }
    else {
        if (backend_meta_data->value) {
            libdbo_value_free(backend_meta_data->value);
            backend_meta_data->value = NULL;
        }
    }

    return DB_OK;
}

const char* libdbo_backend_meta_data_name(const libdbo_backend_meta_data_t* backend_meta_data) {
    if (!backend_meta_data) {
        return NULL;
    }

    return backend_meta_data->name;
}

const libdbo_value_t* libdbo_backend_meta_data_value(const libdbo_backend_meta_data_t* backend_meta_data) {
    if (!backend_meta_data) {
        return NULL;
    }

    return backend_meta_data->value;
}

int libdbo_backend_meta_data_set_name(libdbo_backend_meta_data_t* backend_meta_data, const char* name) {
    char* new_name;

    if (!backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }

    if (!(new_name = strdup(name))) {
        return DB_ERROR_UNKNOWN;
    }

    if (backend_meta_data->name) {
        free(backend_meta_data->name);
    }
    backend_meta_data->name = new_name;
    return DB_OK;
}

int libdbo_backend_meta_data_set_value(libdbo_backend_meta_data_t* backend_meta_data, libdbo_value_t* value) {
    if (!backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }
    if (backend_meta_data->value) {
        return DB_ERROR_UNKNOWN;
    }

    backend_meta_data->value = value;
    return DB_OK;
}

int libdbo_backend_meta_data_not_empty(const libdbo_backend_meta_data_t* backend_meta_data) {
    if (!backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_meta_data->name) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_meta_data->value) {
        return DB_ERROR_UNKNOWN;
    }
    return DB_OK;
}

/* DB BACKEND META DATA LIST */

static libdbo_mm_t __backend_meta_data_list_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_backend_meta_data_list_t));

libdbo_backend_meta_data_list_t* libdbo_backend_meta_data_list_new(void) {
    libdbo_backend_meta_data_list_t* backend_meta_data_list =
        (libdbo_backend_meta_data_list_t*)libdbo_mm_new0(&__backend_meta_data_list_alloc);

    return backend_meta_data_list;
}

libdbo_backend_meta_data_list_t* libdbo_backend_meta_data_list_new_copy(const libdbo_backend_meta_data_list_t* from_backend_meta_data_list) {
    libdbo_backend_meta_data_list_t* backend_meta_data_list;

    if (!from_backend_meta_data_list) {
        return NULL;
    }

    backend_meta_data_list = (libdbo_backend_meta_data_list_t*)libdbo_mm_new0(&__backend_meta_data_list_alloc);
    if (backend_meta_data_list) {
        if (libdbo_backend_meta_data_list_copy(backend_meta_data_list, from_backend_meta_data_list)) {
            libdbo_backend_meta_data_list_free(backend_meta_data_list);
            return NULL;
        }
    }

    return backend_meta_data_list;
}

void libdbo_backend_meta_data_list_free(libdbo_backend_meta_data_list_t* backend_meta_data_list) {
    if (backend_meta_data_list) {
        if (backend_meta_data_list->begin) {
            libdbo_backend_meta_data_t* this = backend_meta_data_list->begin;
            libdbo_backend_meta_data_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_backend_meta_data_free(this);
                this = next;
            }
        }
        libdbo_mm_delete(&__backend_meta_data_list_alloc, backend_meta_data_list);
    }
}

int libdbo_backend_meta_data_list_copy(libdbo_backend_meta_data_list_t* backend_meta_data_list, const libdbo_backend_meta_data_list_t* from_backend_meta_data_list) {
    const libdbo_backend_meta_data_t* backend_meta_data;
    libdbo_backend_meta_data_t* backend_meta_data_copy;

    if (!backend_meta_data_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!from_backend_meta_data_list) {
        return DB_ERROR_UNKNOWN;
    }

    if (backend_meta_data_list->begin) {
        libdbo_backend_meta_data_t* this = backend_meta_data_list->begin;
        libdbo_backend_meta_data_t* next = NULL;

        while (this) {
            next = this->next;
            libdbo_backend_meta_data_free(this);
            this = next;
        }
    }

    backend_meta_data_list->begin = NULL;;
    backend_meta_data_list->end = NULL;

    backend_meta_data = from_backend_meta_data_list->begin;
    while (backend_meta_data) {
        if (!(backend_meta_data_copy = libdbo_backend_meta_data_new())) {
            return DB_ERROR_UNKNOWN;
        }

        if (libdbo_backend_meta_data_copy(backend_meta_data_copy, backend_meta_data)
            || libdbo_backend_meta_data_list_add(backend_meta_data_list, backend_meta_data_copy))
        {
            libdbo_backend_meta_data_free(backend_meta_data_copy);
            return DB_ERROR_UNKNOWN;
        }

        backend_meta_data = backend_meta_data->next;
    }

    return DB_OK;
}

int libdbo_backend_meta_data_list_add(libdbo_backend_meta_data_list_t* backend_meta_data_list, libdbo_backend_meta_data_t* backend_meta_data) {
    if (!backend_meta_data_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_meta_data) {
        return DB_ERROR_UNKNOWN;
    }
    if (libdbo_backend_meta_data_not_empty(backend_meta_data)) {
        return DB_ERROR_UNKNOWN;
    }
    if (backend_meta_data->next) {
        return DB_ERROR_UNKNOWN;
    }

    if (backend_meta_data_list->begin) {
        if (!backend_meta_data_list->end) {
            return DB_ERROR_UNKNOWN;
        }
        backend_meta_data_list->end->next = backend_meta_data;
        backend_meta_data_list->end = backend_meta_data;
    }
    else {
        backend_meta_data_list->begin = backend_meta_data;
        backend_meta_data_list->end = backend_meta_data;
    }

    return DB_OK;
}

const libdbo_backend_meta_data_t* libdbo_backend_meta_data_list_find(const libdbo_backend_meta_data_list_t* backend_meta_data_list, const char* name) {
    libdbo_backend_meta_data_t* backend_meta_data;

    if (!backend_meta_data_list) {
        return NULL;
    }
    if (!name) {
        return NULL;
    }

    backend_meta_data = backend_meta_data_list->begin;
    while (backend_meta_data) {
        if (libdbo_backend_meta_data_not_empty(backend_meta_data)) {
            return NULL;
        }
        if (!strcmp(backend_meta_data->name, name)) {
            break;
        }
        backend_meta_data = backend_meta_data->next;
    }

    return backend_meta_data;
}
