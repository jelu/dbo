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
 * Based on enforcer-ng/src/db/db_object.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo_object.h"
#include "libdbo_error.h"

#include "libdbo_mm.h"

#include <stdlib.h>

/* DB OBJECT FIELD */

static libdbo_mm_t __object_field_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_object_field_t));

libdbo_object_field_t* libdbo_object_field_new(void) {
    libdbo_object_field_t* object_field =
        (libdbo_object_field_t*)libdbo_mm_new0(&__object_field_alloc);

    if (object_field) {
        object_field->type = DB_TYPE_EMPTY;
    }

    return object_field;
}

libdbo_object_field_t* libdbo_object_field_new_copy(const libdbo_object_field_t* from_object_field) {
    libdbo_object_field_t* object_field;

    if (!from_object_field) {
        return NULL;
    }

    if (!(object_field = libdbo_object_field_new())
        || libdbo_object_field_copy(object_field, from_object_field))
    {
        libdbo_object_field_free(object_field);
        return NULL;
    }

    return object_field;
}

void libdbo_object_field_free(libdbo_object_field_t* object_field) {
    if (object_field) {
        libdbo_mm_delete(&__object_field_alloc, object_field);
    }
}

int libdbo_object_field_copy(libdbo_object_field_t* object_field, const libdbo_object_field_t* from_object_field) {
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (!from_object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field->next) {
        return DB_ERROR_UNKNOWN;
    }

    object_field->name = from_object_field->name;
    object_field->type = from_object_field->type;
    object_field->enum_set = from_object_field->enum_set;

    return DB_OK;
}

const char* libdbo_object_field_name(const libdbo_object_field_t* object_field) {
    if (!object_field) {
        return NULL;
    }

    return object_field->name;
}

libdbo_type_t libdbo_object_field_type(const libdbo_object_field_t* object_field) {
    if (!object_field) {
        return DB_TYPE_EMPTY;
    }

    return object_field->type;
}

const libdbo_enum_t* libdbo_object_field_enum_set(const libdbo_object_field_t* object_field) {
    if (!object_field) {
        return NULL;
    }

    return object_field->enum_set;
}

int libdbo_object_field_set_name(libdbo_object_field_t* object_field, const char* name) {
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (!name) {
        return DB_ERROR_UNKNOWN;
    }

    object_field->name = name;
    return DB_OK;
}

int libdbo_object_field_set_type(libdbo_object_field_t* object_field, libdbo_type_t type) {
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (type == DB_TYPE_EMPTY) {
        return DB_ERROR_UNKNOWN;
    }

    object_field->type = type;
    return DB_OK;
}

int libdbo_object_field_set_enum_set(libdbo_object_field_t* object_field, const libdbo_enum_t* enum_set) {
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field->type != DB_TYPE_ENUM) {
        return DB_ERROR_UNKNOWN;
    }

    object_field->enum_set = enum_set;
    return DB_OK;
}

int libdbo_object_field_not_empty(const libdbo_object_field_t* object_field) {
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field->name) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field->type == DB_TYPE_EMPTY) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field->type == DB_TYPE_ENUM && !object_field->enum_set) {
        return DB_ERROR_UNKNOWN;
    }
    return DB_OK;
}

const libdbo_object_field_t* libdbo_object_field_next(const libdbo_object_field_t* object_field) {
    if (!object_field) {
        return NULL;
    }

    return object_field->next;
}

/* DB OBJECT FIELD LIST */

static libdbo_mm_t __object_field_list_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_object_field_list_t));

libdbo_object_field_list_t* libdbo_object_field_list_new(void) {
    libdbo_object_field_list_t* object_field_list =
        (libdbo_object_field_list_t*)libdbo_mm_new0(&__object_field_list_alloc);

    return object_field_list;
}

libdbo_object_field_list_t* libdbo_object_field_list_new_copy(const libdbo_object_field_list_t* from_object_field_list) {
    libdbo_object_field_list_t* object_field_list;

    if (!from_object_field_list) {
        return NULL;
    }

    if (!(object_field_list = libdbo_object_field_list_new())
        || libdbo_object_field_list_copy(object_field_list, from_object_field_list))
    {
        libdbo_object_field_list_free(object_field_list);
        return NULL;
    }

    return object_field_list;
}

void libdbo_object_field_list_free(libdbo_object_field_list_t* object_field_list) {
    if (object_field_list) {
        if (object_field_list->begin) {
            libdbo_object_field_t* this = object_field_list->begin;
            libdbo_object_field_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_object_field_free(this);
                this = next;
            }
        }
        libdbo_mm_delete(&__object_field_list_alloc, object_field_list);
    }
}

int libdbo_object_field_list_copy(libdbo_object_field_list_t* object_field_list, const libdbo_object_field_list_t* from_object_field_list) {
    libdbo_object_field_t* object_field;
    libdbo_object_field_t* object_field_copy;

    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    /*
     * TODO: Should we be able to copy into a object field list that already
     * contains data?
     */
    if (object_field_list->begin) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field_list->end) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field_list->size) {
        return DB_ERROR_UNKNOWN;
    }
    if (!from_object_field_list) {
        return DB_ERROR_UNKNOWN;
    }

    object_field = from_object_field_list->begin;
    while (object_field) {
        if (!(object_field_copy = libdbo_object_field_new_copy(object_field))
            || libdbo_object_field_list_add(object_field_list, object_field_copy))
        {
            return DB_ERROR_UNKNOWN;
        }

        object_field = object_field->next;
    }

    return DB_OK;
}

int libdbo_object_field_list_add(libdbo_object_field_list_t* object_field_list, libdbo_object_field_t* object_field) {
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field) {
        return DB_ERROR_UNKNOWN;
    }
    if (libdbo_object_field_not_empty(object_field)) {
        return DB_ERROR_UNKNOWN;
    }
    if (object_field->next) {
        return DB_ERROR_UNKNOWN;
    }

    if (object_field_list->begin) {
        if (!object_field_list->end) {
            return DB_ERROR_UNKNOWN;
        }
        object_field_list->end->next = object_field;
        object_field_list->end = object_field;
    }
    else {
        object_field_list->begin = object_field;
        object_field_list->end = object_field;
    }
    object_field_list->size++;

    return DB_OK;
}

const libdbo_object_field_t* libdbo_object_field_list_begin(const libdbo_object_field_list_t* object_field_list) {
    if (!object_field_list) {
        return NULL;
    }

    return object_field_list->begin;
}

size_t libdbo_object_field_list_size(const libdbo_object_field_list_t* object_field_list) {
    if (!object_field_list) {
        return 0;
    }

    return object_field_list->size;
}

/* DB OBJECT */

static libdbo_mm_t __object_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_object_t));

libdbo_object_t* libdbo_object_new(void) {
    libdbo_object_t* object =
        (libdbo_object_t*)libdbo_mm_new0(&__object_alloc);

    return object;
}

void libdbo_object_free(libdbo_object_t* object) {
    if (object) {
        if (object->object_field_list) {
            libdbo_object_field_list_free(object->object_field_list);
        }
        if (object->backend_meta_data_list) {
            libdbo_backend_meta_data_list_free(object->backend_meta_data_list);
        }
        libdbo_mm_delete(&__object_alloc, object);
    }
}

const libdbo_connection_t* libdbo_object_connection(const libdbo_object_t* object) {
    if (!object) {
        return NULL;
    }
    return object->connection;
}

const char* libdbo_object_table(const libdbo_object_t* object) {
    if (!object) {
        return NULL;
    }
    return object->table;
}

const char* libdbo_object_primary_key_name(const libdbo_object_t* object) {
    if (!object) {
        return NULL;
    }
    return object->primary_key_name;
}

const libdbo_object_field_list_t* libdbo_object_object_field_list(const libdbo_object_t* object) {
    if (!object) {
        return NULL;
    }
    return object->object_field_list;
}

const libdbo_backend_meta_data_list_t* libdbo_object_backend_meta_data_list(const libdbo_object_t* object) {
    if (!object) {
        return NULL;
    }
    return object->backend_meta_data_list;
}

int libdbo_object_set_connection(libdbo_object_t* object, const libdbo_connection_t* connection) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!connection) {
        return DB_ERROR_UNKNOWN;
    }
    if (object->connection) {
        return DB_ERROR_UNKNOWN;
    }

    object->connection = connection;
    return DB_OK;
}

int libdbo_object_set_table(libdbo_object_t* object, const char* table) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!table) {
        return DB_ERROR_UNKNOWN;
    }
    if (object->table) {
        return DB_ERROR_UNKNOWN;
    }

    object->table = table;
    return DB_OK;
}

int libdbo_object_set_primary_key_name(libdbo_object_t* object, const char* primary_key_name) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }
    if (object->primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }

    object->primary_key_name = primary_key_name;
    return DB_OK;
}

int libdbo_object_set_object_field_list(libdbo_object_t* object, libdbo_object_field_list_t* object_field_list) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (object->object_field_list) {
        return DB_ERROR_UNKNOWN;
    }

    object->object_field_list = object_field_list;
    return DB_OK;
}

int libdbo_object_set_backend_meta_data_list(libdbo_object_t* object, libdbo_backend_meta_data_list_t* backend_meta_data_list) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!backend_meta_data_list) {
        return DB_ERROR_UNKNOWN;
    }

    if (object->backend_meta_data_list) {
        libdbo_backend_meta_data_list_free(object->backend_meta_data_list);
    }

    object->backend_meta_data_list = backend_meta_data_list;
    return DB_OK;
}

int libdbo_object_create(const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->connection) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->table) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }

    if (object_field_list) {
        return libdbo_connection_create(object->connection, object, object_field_list, value_set);
    }
    return libdbo_connection_create(object->connection, object, object->object_field_list, value_set);
}

libdbo_result_list_t* libdbo_object_read(const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    if (!object) {
        return NULL;
    }
    if (!object->connection) {
        return NULL;
    }
    if (!object->table) {
        return NULL;
    }
    if (!object->primary_key_name) {
        return NULL;
    }

    return libdbo_connection_read(object->connection, object, join_list, clause_list);
}

int libdbo_object_update(const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->connection) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->table) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }

    if (object_field_list) {
        return libdbo_connection_update(object->connection, object, object_field_list, value_set, clause_list);
    }
    return libdbo_connection_update(object->connection, object, object->object_field_list, value_set, clause_list);
}

int libdbo_object_delete(const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->connection) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->table) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_connection_delete(object->connection, object, clause_list);
}

int libdbo_object_count(const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    if (!object) {
        return DB_ERROR_UNKNOWN;
    }
    if (!count) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->connection) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->table) {
        return DB_ERROR_UNKNOWN;
    }
    if (!object->primary_key_name) {
        return DB_ERROR_UNKNOWN;
    }

    return libdbo_connection_count(object->connection, object, join_list, clause_list, count);
}
