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
 * Based on enforcer-ng/src/db/db_connectionr.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/connection.h */
/** \defgroup libdbo_connection libdbo_connection
 * Database Connection.
 * These are the functions and container for handling a database connection.
 */

#ifndef libdbo_connection_h
#define libdbo_connection_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_connection;
#endif

/** \addtogroup libdbo_connection */
/** \{ */
/**
 * A database connection.
 */
typedef struct libdbo_connection libdbo_connection_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/configuration.h>
#include <libdbo/backend.h>
#include <libdbo/result.h>
#include <libdbo/object.h>
#include <libdbo/join.h>
#include <libdbo/clause.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_connection {
    const libdbo_configuration_list_t* configuration_list;
    libdbo_backend_t* backend;
};
#endif

/** \addtogroup libdbo_connection */
/** \{ */

/**
 * Create a new database connection.
 * \return a libdbo_connection_t pointer or NULL on error.
 */
libdbo_connection_t* libdbo_connection_new(void);

/**
 * Delete a database connection and the database backend within.
 * \param[in] connection a libdbo_connection_t pointer.
 */
void libdbo_connection_free(libdbo_connection_t* connection);

/**
 * Set the database configuration list for a database connection.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_set_configuration_list(libdbo_connection_t* connection, const libdbo_configuration_list_t* configuration_list);

/**
 * Setup the database connection, this verifies the information in the database
 * configuration list and allocated a database backend.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_setup(libdbo_connection_t* connection);

/**
 * Connect to the database.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_connect(const libdbo_connection_t* connection);

/**
 * Disconnect from the database.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_disconnect(const libdbo_connection_t* connection);

/**
 * Create an object in the database. The `object` refer to the database object
 * begin created, the `object_field_list` describes the fields that should be
 * set in the object and the `value_set` has the values for each field.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_create(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set);

/**
 * Read an object or objects from the database.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error or if no objects where
 * read.
 */
libdbo_result_list_t* libdbo_connection_read(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list);

/**
 * Update an object or objects in the database.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_update(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list);

/**
 * Delete an object or objects from the database.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_delete(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list);

/**
 * Count objects from the database. Return the count in `count`.
 * \param[in] connection a libdbo_connection_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[out] count a size_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_count(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count);

/**
 * Begin a transaction for a database connection.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_transaction_begin(const libdbo_connection_t* connection);

/**
 * Commit a transaction for a database connection.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_transaction_commit(const libdbo_connection_t* connection);

/**
 * Roll back a transaction for a database connection.
 * \param[in] connection a libdbo_connection_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_connection_transaction_rollback(const libdbo_connection_t* connection);

/** \} */

#ifdef __cplusplus
}
#endif

#endif
