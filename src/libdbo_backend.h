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
 * Based on enforcer-ng/src/db/db_backend.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo_backend.h */
/** \defgroup libdbo_backend_handle libdbo_backend_handle
 * Database Backend Handle.
 * These are the functions and container for defining a database backend handle.
 */
/** \defgroup libdbo_backend libdbo_backend
 * Database Backend.
 * These are the functions and container for handling a database backend.
 */
/** \defgroup libdbo_backend_meta_data libdbo_backend_meta_data
 * Database Backend Meta Data.
 * These are the functions and container for handling a database backend meta
 * data value.
 */
/** \defgroup libdbo_backend_meta_data_list libdbo_backend_meta_data_list
 * Database Backend Meta Data List.
 * These are the functions and container for handling lists of database backend
 * meta data.
 */
/** \defgroup libdbo_backend_factory libdbo_backend_factory
 * Database Backend Factory.
 * These are the functions for accessing the available database backends.
 */

#ifndef libdbo_backend_h
#define libdbo_backend_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_backend_handle;
struct libdbo_backend;
struct libdbo_backend_meta_data;
struct libdbo_backend_meta_data_list;
#endif

/** \addtogroup libdbo_backend_handle */
/** \{ */
/**
 * A database backend handle that contains all function pointers for a backend
 * and the backend specific data.
 */
typedef struct libdbo_backend_handle libdbo_backend_handle_t;
/** \} */
/** \addtogroup libdbo_backend */
/** \{ */
/**
 * A database backend.
 */
typedef struct libdbo_backend libdbo_backend_t;
/** \} */
/** \addtogroup libdbo_backend_meta_data */
/** \{ */
/**
 * A database backend meta data that may be used by backends to store backend
 * specific data about objects and results.
 */
typedef struct libdbo_backend_meta_data libdbo_backend_meta_data_t;
/** \} */
/** \addtogroup libdbo_backend_meta_data_list */
/** \{ */
/**
 * A list of database backend meta data that may be used by backends to store
 * backend specific data about objects and results.
 */
typedef struct libdbo_backend_meta_data_list libdbo_backend_meta_data_list_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include "libdbo_configuration.h"
#include "libdbo_result.h"
#include "libdbo_object.h"
#include "libdbo_join.h"
#include "libdbo_clause.h"
#include "libdbo_value.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup libdbo_backend_handle */
/** \{ */

/**
 * Function pointer for initializing a database backend. The backend handle
 * specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_initialize_t)(void* data);

/**
 * Function pointer for shutting down a database backend. The backend handle
 * specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_shutdown_t)(void* data);

/**
 * Function pointer for connecting a database backend. The backend handle
 * specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_connect_t)(void* data, const libdbo_configuration_list_t* configuration_list);

/**
 * Function pointer for disconnecting a database backend. The backend handle
 * specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_disconnect_t)(void* data);

/**
 * Function pointer for creating a object in a database backend. The backend
 * handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_create_t)(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set);

/**
 * Function pointer for reading objects from database backend. The backend
 * handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error or if no objects where
 * read.
 */
typedef libdbo_result_list_t* (*libdbo_backend_handle_read_t)(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list);

/**
 * Function pointer for updating objects in a database backend. The backend
 * handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_update_t)(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list);

/**
 * Function pointer for deleting objects from database backend. The backend
 * handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_delete_t)(void* data, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list);

/**
 * Function pointer for counting objects from database backend. The backend
 * handle specific data is supplied in `data`. Returns the size in `size`.
 * \param[in] data a void pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[out] count a size_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_count_t)(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count);

/**
 * Function pointer for freeing the backend handle specific data in `data`.
 * \param[in] data a void pointer.
 */
typedef void (*libdbo_backend_handle_free_t)(void* data);

/**
 * Function pointer for beginning a transaction in a database backend. The
 * backend handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_transaction_begin_t)(void* data);

/**
 * Function pointer for committing a transaction in a database backend. The
 * backend handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_transaction_commit_t)(void* data);

/**
 * Function pointer for rolling back a transaction in a database backend. The
 * backend handle specific data is supplied in `data`.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
typedef int (*libdbo_backend_handle_transaction_rollback_t)(void* data);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_backend_handle {
    void* data;
    libdbo_backend_handle_initialize_t initialize_function;
    libdbo_backend_handle_shutdown_t shutdown_function;
    libdbo_backend_handle_connect_t connect_function;
    libdbo_backend_handle_disconnect_t disconnect_function;
    libdbo_backend_handle_create_t create_function;
    libdbo_backend_handle_read_t read_function;
    libdbo_backend_handle_update_t update_function;
    libdbo_backend_handle_delete_t delete_function;
    libdbo_backend_handle_count_t count_function;
    libdbo_backend_handle_free_t free_function;
    libdbo_backend_handle_transaction_begin_t transaction_begin_function;
    libdbo_backend_handle_transaction_commit_t transaction_commit_function;
    libdbo_backend_handle_transaction_rollback_t transaction_rollback_function;
};
#endif

/**
 * Create a new database backend handle.
 * \return a libdbo_backend_handle_t pointer or NULL on error.
 */
libdbo_backend_handle_t* libdbo_backend_handle_new(void);

/**
 * Delete a database backend handle, disconnecting the backend and freeing the
 * backend specific data.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 */
void libdbo_backend_handle_free(libdbo_backend_handle_t* backend_handle);

/**
 * Initiate the backend of a database backend.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_initialize(const libdbo_backend_handle_t* backend_handle);

/**
 * Shutdown the backend of a database backend.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_shutdown(const libdbo_backend_handle_t* backend_handle);

/**
 * Connect to the database of a database backend, the connection specific
 * configuration is given by `configuration_list`.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_connect(const libdbo_backend_handle_t* backend_handle, const libdbo_configuration_list_t* configuration_list);

/**
 * Disconnect from the database in a database backend.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_disconnect(const libdbo_backend_handle_t* backend_handle);

/**
 * Create an object in the database. The `object` refer to the database object
 * begin created, the `object_field_list` describes the fields that should be
 * set in the object and the `value_set` has the values for each field.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_create(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set);

/**
 * Read an object or objects from the database.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error or if no objects where
 * read.
 */
libdbo_result_list_t* libdbo_backend_handle_read(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list);

/**
 * Update an object or objects in the database.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_update(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list);

/**
 * Delete an object or objects from the database.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_delete(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list);

/**
 * Count objects from the database. Return the count in `count`.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[out] count a size_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_count(const libdbo_backend_handle_t* backend_handle, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count);

/**
 * Begin a transaction for a database connection.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_transaction_begin(const libdbo_backend_handle_t* backend_handle);

/**
 * Commit a transaction for a database connection.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_transaction_commit(const libdbo_backend_handle_t* backend_handle);

/**
 * Roll back a transaction for a database connection.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_transaction_rollback(const libdbo_backend_handle_t* backend_handle);

/**
 * Get the backend specific data of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return a void pointer.
 */
const void* libdbo_backend_handle_data(const libdbo_backend_handle_t* backend_handle);

/**
 * Set the initialize function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] initialize_function a libdbo_backend_handle_initialize_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_initialize(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_initialize_t initialize_function);

/**
 * Set the shutdown function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] shutdown_function a libdbo_backend_handle_shutdown_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_shutdown(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_shutdown_t shutdown_function);

/**
 * Set the connect function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] connect_function a libdbo_backend_handle_connect_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_connect(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_connect_t connect_function);

/**
 * Set the disconnect function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] disconnect_function a libdbo_backend_handle_disconnect_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_disconnect(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_disconnect_t disconnect_function);

/**
 * Set the create function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] create_function a libdbo_backend_handle_create_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_create(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_create_t create_function);

/**
 * Set the read function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] read_function a libdbo_backend_handle_read_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_read(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_read_t read_function);

/**
 * Set the update function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] update_function a libdbo_backend_handle_update_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_update(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_update_t update_function);

/**
 * Set the delete function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] delete_function a libdbo_backend_handle_delete_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_delete(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_delete_t delete_function);

/**
 * Set the count function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] count_function a libdbo_backend_handle_count_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_count(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_count_t count_function);

/**
 * Set the free function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] free_function a libdbo_backend_handle_free_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_free(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_free_t free_function);

/**
 * Set the transaction begin function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] transaction_begin_function a libdbo_backend_handle_transaction_begin_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_transaction_begin(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_begin_t transaction_begin_function);

/**
 * Set the transaction commit function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] transaction_commit_function a libdbo_backend_handle_transaction_commit_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_transaction_commit(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_commit_t transaction_commit_function);

/**
 * Set the transaction rollback function of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] transaction_rollback_function a libdbo_backend_handle_transaction_rollback_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_transaction_rollback(libdbo_backend_handle_t* backend_handle, libdbo_backend_handle_transaction_rollback_t transaction_rollback_function);

/**
 * Set the backend specific data of a database backend handle.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \param[in] data a void pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_handle_set_data(libdbo_backend_handle_t* backend_handle, void* data);

/**
 * Check if the database backend handle is not empty.
 * \param[in] backend_handle a libdbo_backend_handle_t pointer.
 * \return DB_ERROR_* if empty, otherwise DB_OK.
 */
int libdbo_backend_handle_not_empty(const libdbo_backend_handle_t* backend_handle);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_backend {
    libdbo_backend_t* next;
    char* name;
    libdbo_backend_handle_t* handle;
};
#endif

/** \addtogroup libdbo_backend */
/** \{ */

/**
 * Create a new database backend.
 * \return a libdbo_backend_t pointer or NULL on error.
 */
libdbo_backend_t* libdbo_backend_new(void);

/**
 * Delete a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 */
void libdbo_backend_free(libdbo_backend_t* backend);

/**
 * Get the name of a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return a character pointer or NULL on error or if no name has been set.
 */
const char* libdbo_backend_name(const libdbo_backend_t* backend);

/**
 * Get the database backend handle of a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return a libdbo_backend_handle_t pointer or NULL on error or if no database
 * backend handle has been set.
 */
const libdbo_backend_handle_t* libdbo_backend_handle(const libdbo_backend_t* backend);

/**
 * Set the name of a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] name a character pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_set_name(libdbo_backend_t* backend, const char* name);

/**
 * Det the database backend handle of a database backend, this takes over the
 * ownership of the database backend handle.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] handle a libdbo_backend_handle_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_set_handle(libdbo_backend_t* backend, libdbo_backend_handle_t* handle);

/**
 * Check if a database backend is not empty.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* if empty, otherwise DB_OK.
 */
int libdbo_backend_not_empty(const libdbo_backend_t* backend);

/**
 * Initiate the backend of a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_initialize(const libdbo_backend_t* backend);

/**
 * Shutdown the backend of a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_shutdown(const libdbo_backend_t* backend);

/**
 * Connect to the database of a database backend, the connection specific
 * configuration is given by `configuration_list`.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_connect(const libdbo_backend_t* backend, const libdbo_configuration_list_t* configuration_list);

/**
 * Disconnect from the database in a database backend.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_disconnect(const libdbo_backend_t* backend);

/**
 * Create an object in the database. The `object` refer to the database object
 * begin created, the `object_field_list` describes the fields that should be
 * set in the object and the `value_set` has the values for each field.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_create(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set);

/**
 * Read an object or objects from the database.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error or if no objects where
 * read.
 */
libdbo_result_list_t* libdbo_backend_read(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list);

/**
 * Update an object or objects in the database.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] object_field_list a libdbo_object_field_list_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_update(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list);

/**
 * Delete an object or objects from the database.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_delete(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list);

/**
 * Count objects from the database. Return the count in `count`.
 * \param[in] backend a libdbo_backend_t pointer.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[out] count a size_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_count(const libdbo_backend_t* backend, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count);

/**
 * Begin a transaction for a database connection.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_transaction_begin(const libdbo_backend_t* backend);

/**
 * Commit a transaction for a database connection.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_transaction_commit(const libdbo_backend_t* backend);

/**
 * Roll back a transaction for a database connection.
 * \param[in] backend a libdbo_backend_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_transaction_rollback(const libdbo_backend_t* backend);

/** \} */

/** \addtogroup libdbo_backend_factory */
/** \{ */

/**
 * Get a new database backend by the name supplied in `name`.
 * \param[in] name a character pointer.
 * \return a libdbo_backend_t pointer or NULL on error or if the database backend
 * does not exist.
 */
libdbo_backend_t* libdbo_backend_factory_get_backend(const char* name);

/**
 * Shutdown the database backends created by the factory.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_factory_shutdown(void);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_backend_meta_data {
    libdbo_backend_meta_data_t* next;
    char* name;
    libdbo_value_t* value;
};
#endif

/** \addtogroup libdbo_backend_meta_data */
/** \{ */

/**
 * Create a new database backend meta data.
 * \return a libdbo_backend_meta_data_t pointer or NULL on error.
 */
libdbo_backend_meta_data_t* libdbo_backend_meta_data_new(void);

/**
 * Create a new database backend meta data that is a copy of another.
 * \param[in] from_backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return a libdbo_backend_meta_data_t pointer or NULL on error.
 */
libdbo_backend_meta_data_t* libdbo_backend_meta_data_new_copy(const libdbo_backend_meta_data_t* from_backend_meta_data);

/**
 * Delete a database backend meta data.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 */
void libdbo_backend_meta_data_free(libdbo_backend_meta_data_t* backend_meta_data);

/**
 * Copy a database backend meta data.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \param[in] from_backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_meta_data_copy(libdbo_backend_meta_data_t* backend_meta_data, const libdbo_backend_meta_data_t* from_backend_meta_data);

/**
 * Get the name of a database backend meta data.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return a character pointer or NULL on error or if no name has been set.
 */
const char* libdbo_backend_meta_data_name(const libdbo_backend_meta_data_t* backend_meta_data);

/**
 * Get the database value of a database backend meta data.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return a libdbo_value_t pointer or NULL on error or if no database value has
 * been set.
 */
const libdbo_value_t* libdbo_backend_meta_data_value(const libdbo_backend_meta_data_t* backend_meta_data);

/**
 * Set the name of a database backend meta data.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \param[in] name a character pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_meta_data_set_name(libdbo_backend_meta_data_t* backend_meta_data, const char* name);

/**
 * Set the database value of a database backend meta data, this takes over the
 * ownership of the database value.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \param[in] value a libdbo_value_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_meta_data_set_value(libdbo_backend_meta_data_t* backend_meta_data, libdbo_value_t* value);

/**
 * Check if the database meta data is not empty.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return DB_ERROR_* if empty, otherwise DB_OK.
 */
int libdbo_backend_meta_data_not_empty(const libdbo_backend_meta_data_t* backend_meta_data);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_backend_meta_data_list {
    libdbo_backend_meta_data_t* begin;
    libdbo_backend_meta_data_t* end;
};
#endif

/** \addtogroup libdbo_backend_meta_data_list */
/** \{ */

/**
 * Create a new database backend meta data list.
 * \return a libdbo_backend_meta_data_list_t pointer or NULL on error.
 */
libdbo_backend_meta_data_list_t* libdbo_backend_meta_data_list_new(void);

/**
 * Create a new database backend meta data list that is a copy of another.
 * \param[in] from_backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \return a libdbo_backend_meta_data_list_t pointer or NULL on error.
 */
libdbo_backend_meta_data_list_t* libdbo_backend_meta_data_list_new_copy(const libdbo_backend_meta_data_list_t* from_backend_meta_data_list);

/**
 * Delete a database backend meta data list and all database backend meta data
 * in the list.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 */
void libdbo_backend_meta_data_list_free(libdbo_backend_meta_data_list_t* backend_meta_data_list);

/**
 * Copy a database backend meta data list.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \param[in] from_backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_meta_data_list_copy(libdbo_backend_meta_data_list_t* backend_meta_data_list, const libdbo_backend_meta_data_list_t* from_backend_meta_data_list);

/**
 * Add a database backend meta data to a database backend meta data list, this
 * takes over the ownership of the database backend meta data.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \param[in] backend_meta_data a libdbo_backend_meta_data_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_backend_meta_data_list_add(libdbo_backend_meta_data_list_t* backend_meta_data_list, libdbo_backend_meta_data_t* backend_meta_data);

/**
 * Find a database backend meta data by name in a database backend meta data
 * list.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \param[in] name a character pointer.
 * \return a libdbo_backend_meta_data_t pointer or NULL on error or if the database
 * backend meta data does not exist.
 */
const libdbo_backend_meta_data_t* libdbo_backend_meta_data_list_find(const libdbo_backend_meta_data_list_t* backend_meta_data_list, const char* name);

/** \} */

#ifdef __cplusplus
}
#endif

#endif
