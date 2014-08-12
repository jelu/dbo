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
 * Based on enforcer-ng/src/db/db_object.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/object.h */
/** \defgroup libdbo_object libdbo_object
 * Database Object.
 * These are the functions and container for handling a database object.
 */
/** \defgroup libdbo_object_field libdbo_object_field
 * Database Object Field.
 * These are the functions and container for handling a database object field.
 */
/** \defgroup libdbo_object_field_list libdbo_object_field_list
 * Database Object Field List.
 * These are the functions and container for handling database object fields.
 */

#ifndef libdbo_object_h
#define libdbo_object_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_object;
struct libdbo_object_field;
struct libdbo_object_field_list;
#endif

/** \addtogroup libdbo_object */
/** \{ */
/**
 * A database object.
 */
typedef struct libdbo_object libdbo_object_t;
/** \} */

/** \addtogroup libdbo_object_field */
/** \{ */
/**
 * A representation of an field/value for a database object.
 */
typedef struct libdbo_object_field libdbo_object_field_t;
/** \} */

/** \addtogroup libdbo_object_field_list */
/** \{ */
/**
 * A list of object fields.
 */
typedef struct libdbo_object_field_list libdbo_object_field_list_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/connection.h>
#include <libdbo/result.h>
#include <libdbo/join.h>
#include <libdbo/clause.h>
#include <libdbo/type.h>
#include <libdbo/value.h>
#include <libdbo/enum.h>
#include <libdbo/backend.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_object_field {
    libdbo_object_field_t* next;
    const char* name;
    libdbo_type_t type;
    const libdbo_enum_t* enum_set;
};
#endif

/** \addtogroup libdbo_object_field */
/** \{ */

/**
 * Create a database object field.
 * \return a libdbo_object_field_t pointer or NULL on error.
 */
libdbo_object_field_t* libdbo_object_field_new(void);

/**
 * Create a database object field that is a copy of another.
 * \param[in] from_object_field a libdbo_object_field_t pointer.
 * \return a libdbo_object_field_t pointer or NULL on error.
 */
libdbo_object_field_t* libdbo_object_field_new_copy(const libdbo_object_field_t* from_object_field);

/**
 * Delete a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 */
void libdbo_object_field_free(libdbo_object_field_t* object_field);

/**
 * Copy the content of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \param[in] from_object_field a libdbo_object_field_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_copy(libdbo_object_field_t* object_field, const libdbo_object_field_t* from_object_field);

/**
 * Get the name of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return a character pointer or NULL on error or if no field name has been set.
 */
const char* libdbo_object_field_name(const libdbo_object_field_t* object_field);

/**
 * Get the type of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return a libdbo_type_t.
 */
libdbo_type_t libdbo_object_field_type(const libdbo_object_field_t* object_field);

/**
 * Get the enumerate set of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return a NULL terminated libdbo_enum_t list or NULL on error or if no enumerate
 * set has been set.
 */
const libdbo_enum_t* libdbo_object_field_enum_set(const libdbo_object_field_t* object_field);

/**
 * Set the name of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \param[in] name a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_set_name(libdbo_object_field_t* object_field, const char* name);

/**
 * Set the type of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \param[in] type a libdbo_type_t.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_set_type(libdbo_object_field_t* object_field, libdbo_type_t type);

/**
 * Set the enumerate set of a database object field.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \param[in] enum_set a NULL terminated libdbo_enum_t list.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_set_enum_set(libdbo_object_field_t* object_field, const libdbo_enum_t* enum_set);

/**
 * Check if the object field is not empty.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return LIBDBO_ERROR_* if empty, otherwise LIBDBO_OK.
 */
int libdbo_object_field_not_empty(const libdbo_object_field_t* object_field);

/**
 * Get the next object field connected in a database object field list.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return a libdbo_object_field_t pointer or NULL on error or if there are no more
 * object fields in the list.
 */
const libdbo_object_field_t* libdbo_object_field_next(const libdbo_object_field_t* object_field);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_object_field_list {
    libdbo_object_field_t* begin;
    libdbo_object_field_t* end;
    size_t size;
};
#endif

/** \addtogroup libdbo_object_field_list */
/** \{ */

/**
 * Create a new object field list.
 * \return a libdbo_object_field_list_t pointer or NULL on error.
 */
libdbo_object_field_list_t* libdbo_object_field_list_new(void);

/**
 * Create a new object field list that is a copy of another.
 * \param[in] from_object_field_list a libdbo_object_field_list_t pointer.
 * \return a libdbo_object_field_list_t pointer or NULL on error.
 */
libdbo_object_field_list_t* libdbo_object_field_list_new_copy(const libdbo_object_field_list_t* from_object_field_list);

/**
 * Delete a object field list and all object fields within the list.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 */
void libdbo_object_field_list_free(libdbo_object_field_list_t* object_field_list);

/**
 * Copy the content of a database object field list.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] from_object_field_list a libdbo_object_field_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_list_copy(libdbo_object_field_list_t* object_field_list, const libdbo_object_field_list_t* from_object_field_list);

/**
 * Add a database object field to a database object field list, this will takes
 * over the ownership of the object field.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] object_field a libdbo_object_field_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_field_list_add(libdbo_object_field_list_t* object_field_list, libdbo_object_field_t* object_field);

/**
 * Return the first database object field in a database object field list.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \return a libdbo_object_field_t pointer or NULL on error or if the list is empty.
 */
const libdbo_object_field_t* libdbo_object_field_list_begin(const libdbo_object_field_list_t* object_field_list);

/**
 * Return the size of a object field list.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \return a size_t, may be zero on error.
 */
size_t libdbo_object_field_list_size(const libdbo_object_field_list_t* object_field_list);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_object {
    const libdbo_connection_t* connection;
    const char* table;
    const char* primary_key_name;
    libdbo_object_field_list_t* object_field_list;
    libdbo_backend_meta_data_list_t* backend_meta_data_list;
};
#endif

/** \addtogroup libdbo_object */
/** \{ */

/**
 * Create a new database object.
 * \return a libdbo_object_t pointer or NULL on error.
 */
libdbo_object_t* libdbo_object_new(void);

/**
 * Delete a database object and the object field list and backend meta data list
 * if set.
 * \param[in] object a libdbo_object_t pointer.
 */
void libdbo_object_free(libdbo_object_t* object);

/**
 * Get the database connection of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \return a libdbo_connection_t pointer or NULL on error or if no connection has
 * been set.
 */
const libdbo_connection_t* libdbo_object_connection(const libdbo_object_t* object);

/**
 * Get the table name of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \return a character pointer or NULL on error or if no table name has been
 * set.
 */
const char* libdbo_object_table(const libdbo_object_t* object);

/**
 * Get the primary key name of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \return a character pointer or NULL on error or if no primary key name has
 * been set.
 */
const char* libdbo_object_primary_key_name(const libdbo_object_t* object);

/**
 * Get the object field list of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \return a libdbo_object_field_list_t pointer or NULL on error or if no object
 * field list has been set.
 */
const libdbo_object_field_list_t* libdbo_object_object_field_list(const libdbo_object_t* object);

/**
 * Get the backend meta data list of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \return a libdbo_backend_meta_data_list_t pointer or NULL on error or if no
 * backend meta data list has been set.
 */
const libdbo_backend_meta_data_list_t* libdbo_object_backend_meta_data_list(const libdbo_object_t* object);

/**
 * Set the database connection of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_set_connection(libdbo_object_t* object, const libdbo_connection_t* connection);

/**
 * Set the table name of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] table a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_set_table(libdbo_object_t* object, const char* table);

/**
 * Set the primary key name of a database object.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] primary_key_name a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_set_primary_key_name(libdbo_object_t* object, const char* primary_key_name);

/**
 * Set the object field list of a database object, this takes over the ownership
 * of the object field list.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_set_object_field_list(libdbo_object_t* object, libdbo_object_field_list_t* object_field_list);

/**
 * Set the backend meta data list of a database object, this takes over the
 * ownership of the backend meta data list.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_set_backend_meta_data_list(libdbo_object_t* object, libdbo_backend_meta_data_list_t* backend_meta_data_list);

/**
 * Create an object in the database. The `object_field_list` describes the
 * fields that should be set in the object and the `value_set` has the values
 * for each field.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_create(const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set);

/**
 * Read an object or objects from the database.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error or if no objects where
 * read.
 */
libdbo_result_list_t* libdbo_object_read(const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list);

/**
 * Update an object or objects in the database.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_update(const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list);

/**
 * Delete an object or objects from the database.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_delete(const libdbo_object_t* object, const libdbo_clause_list_t* clause_list);

/**
 * Count objects from the database. Return the count in `count`.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[out] count a size_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_object_count(const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_object_t libdbo_object_t
#define db_object_field_t libdbo_object_field_t
#define db_object_field_list_t libdbo_object_field_list_t
#define db_object_field_new(...) libdbo_object_field_new(__VA_ARGS__)
#define db_object_field_new_copy(...) libdbo_object_field_new_copy(__VA_ARGS__)
#define db_object_field_free(...) libdbo_object_field_free(__VA_ARGS__)
#define db_object_field_copy(...) libdbo_object_field_copy(__VA_ARGS__)
#define db_object_field_name(...) libdbo_object_field_name(__VA_ARGS__)
#define db_object_field_type(...) libdbo_object_field_type(__VA_ARGS__)
#define db_object_field_enum_set(...) libdbo_object_field_enum_set(__VA_ARGS__)
#define db_object_field_set_name(...) libdbo_object_field_set_name(__VA_ARGS__)
#define db_object_field_set_type(...) libdbo_object_field_set_type(__VA_ARGS__)
#define db_object_field_set_enum_set(...) libdbo_object_field_set_enum_set(__VA_ARGS__)
#define db_object_field_not_empty(...) libdbo_object_field_not_empty(__VA_ARGS__)
#define db_object_field_next(...) libdbo_object_field_next(__VA_ARGS__)
#define db_object_field_list_new(...) libdbo_object_field_list_new(__VA_ARGS__)
#define db_object_field_list_new_copy(...) libdbo_object_field_list_new_copy(__VA_ARGS__)
#define db_object_field_list_free(...) libdbo_object_field_list_free(__VA_ARGS__)
#define db_object_field_list_copy(...) libdbo_object_field_list_copy(__VA_ARGS__)
#define db_object_field_list_add(...) libdbo_object_field_list_add(__VA_ARGS__)
#define db_object_field_list_begin(...) libdbo_object_field_list_begin(__VA_ARGS__)
#define db_object_field_list_size(...) libdbo_object_field_list_size(__VA_ARGS__)
#define db_object_new(...) libdbo_object_new(__VA_ARGS__)
#define db_object_free(...) libdbo_object_free(__VA_ARGS__)
#define db_object_connection(...) libdbo_object_connection(__VA_ARGS__)
#define db_object_table(...) libdbo_object_table(__VA_ARGS__)
#define db_object_primary_key_name(...) libdbo_object_primary_key_name(__VA_ARGS__)
#define db_object_object_field_list(...) libdbo_object_object_field_list(__VA_ARGS__)
#define db_object_backend_meta_data_list(...) libdbo_object_backend_meta_data_list(__VA_ARGS__)
#define db_object_set_connection(...) libdbo_object_set_connection(__VA_ARGS__)
#define db_object_set_table(...) libdbo_object_set_table(__VA_ARGS__)
#define db_object_set_primary_key_name(...) libdbo_object_set_primary_key_name(__VA_ARGS__)
#define db_object_set_object_field_list(...) libdbo_object_set_object_field_list(__VA_ARGS__)
#define db_object_set_backend_meta_data_list(...) libdbo_object_set_backend_meta_data_list(__VA_ARGS__)
#define db_object_create(...) libdbo_object_create(__VA_ARGS__)
#define db_object_read(...) libdbo_object_read(__VA_ARGS__)
#define db_object_update(...) libdbo_object_update(__VA_ARGS__)
#define db_object_delete(...) libdbo_object_delete(__VA_ARGS__)
#define db_object_count(...) libdbo_object_count(__VA_ARGS__)
#endif
#endif

#endif
