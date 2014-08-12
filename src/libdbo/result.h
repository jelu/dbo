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
 * Based on enforcer-ng/src/db/db_result.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/result.h */
/** \defgroup libdbo_result libdbo_result
 * Database Result.
 * These are the functions and container for handling a database result.
 */
/** \defgroup libdbo_result_list libdbo_result_list
 * Database Result List.
 * These are the functions and container for handling database results.
 */

#ifndef libdbo_result_h
#define libdbo_result_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_result;
struct libdbo_result_list;
#endif

/** \addtogroup libdbo_result */
/** \{ */
/**
 * A container for a database result, the data in the result is represented by
 * a fixed size libdbo_value_set_t.
 */
typedef struct libdbo_result libdbo_result_t;
/** \} */

/** \addtogroup libdbo_result_list */
/** \{ */
/**
 * A list of database results.
 */
typedef struct libdbo_result_list libdbo_result_list_t;
/** \} */

/** \addtogroup libdbo_result */
/** \{ */
/**
 * Function pointer for walking a libdbo_result_list.
 * Function pointer for walking a libdbo_result_list. The backend handle specific
 * data is supplied in `data` and setting `finish` to non-zero tells the backend
 * that we are finished with the libdbo_result_list.
 * \param[in] data a void pointer for the backend specific data.
 * \param[in] finish an integer that if non-zero will tell the backend that we
 * are finished with the result list.
 * \return A pointer to the next libdbo_result_t or NULL on error.
 */
typedef libdbo_result_t* (*libdbo_result_list_next_t)(void* data, int finish);
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/value.h>
#include <libdbo/backend.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_result {
    libdbo_result_t* next;
    libdbo_value_set_t* value_set;
    libdbo_backend_meta_data_list_t* backend_meta_data_list;
};
#endif

/** \addtogroup libdbo_result */
/** \{ */

/**
 * Create a new database result.
 * \return a libdbo_result_t pointer or NULL on error.
 */
libdbo_result_t* libdbo_result_new(void);

/**
 * Create a new database result that is a copy of another.
 * \param[in] from_result a libdbo_result_t pointer.
 * \return a libdbo_result_t pointer or NULL on error.
 */
libdbo_result_t* libdbo_result_new_copy(const libdbo_result_t* from_result);

/**
 * Delete a database result and the backend meta data list if set.
 * \param[in] result a libdbo_result_t pointer.
 */
void libdbo_result_free(libdbo_result_t* result);

/**
 * Copy the content of another database result.
 * \param[in] result a libdbo_result_t pointer.
 * \param[in] from_result a libdbo_result_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_copy(libdbo_result_t* result, const libdbo_result_t* from_result);

/**
 * Get the value set of a database result.
 * \param[in] result a libdbo_result_t pointer.
 * \return a libdbo_value_set_t pointer or NULL on error or if no value set has
 * been set.
 */
const libdbo_value_set_t* libdbo_result_value_set(const libdbo_result_t* result);

/**
 * Get the backend meta data list of a database result.
 * \param[in] result a libdbo_result_t pointer.
 * \return a libdbo_backend_meta_data_list_t pointer or NULL on error or if no
 * backend meta data list has been set.
 */
const libdbo_backend_meta_data_list_t* libdbo_result_backend_meta_data_list(const libdbo_result_t* result);

/**
 * Set the value set of a database result.
 * \param[in] result a libdbo_result_t pointer.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_set_value_set(libdbo_result_t* result, libdbo_value_set_t* value_set);

/**
 * Set the backend meta data list of a database result, this takes over the
 * ownership of the backend meta data list.
 * \param[in] result a libdbo_result_t pointer.
 * \param[in] backend_meta_data_list a libdbo_backend_meta_data_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_set_backend_meta_data_list(libdbo_result_t* result, libdbo_backend_meta_data_list_t* backend_meta_data_list);

/**
 * Check if a database result is not empty.
 * \param[in] result a libdbo_result_t pointer.
 * \return DB_ERROR_* if empty, otherwise DB_OK.
 */
int libdbo_result_not_empty(const libdbo_result_t* result);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_result_list {
    libdbo_result_t* begin;
    libdbo_result_t* end;
    libdbo_result_t* current;
    libdbo_result_list_next_t next_function;
    void* next_data;
    size_t size;
    int begun;
};
#endif

/** \addtogroup libdbo_result_list */
/** \{ */

/**
 * Create a new database result list.
 * \return a libdbo_result_list_t pointer or NULL on error.
 */
libdbo_result_list_t* libdbo_result_list_new(void);

/**
 * Create a new database result list that is a copy of another.
 * \param[in] from_result_list a libdbo_result_list_t pointer.
 * \return a libdbo_result_list_t pointer or NULL on error.
 */
libdbo_result_list_t* libdbo_result_list_new_copy(const libdbo_result_list_t* from_result_list);

/**
 * Delete a database result list and all database results within the list.
 * \param[in] result_list a libdbo_result_list_t pointer.
 */
void libdbo_result_list_free(libdbo_result_list_t* result_list);

/**
 * Copy the content of another database result list.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \param[in] from_result_list a libdbo_result_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_list_copy(libdbo_result_list_t* result_list, const libdbo_result_list_t* from_result_list);

/**
 * Set the function pointer for fetching the next database result for a database
 * result list. The backend handle specific data is supplied in `next_data`
 * along with the total size of the result list in `size`.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \param[in] next_function a libdbo_result_list_next_t function pointer.
 * \param[in] next_data a void pointer.
 * \param[in] size a size_t.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_list_set_next(libdbo_result_list_t* result_list, libdbo_result_list_next_t next_function, void* next_data, size_t size);

/**
 * Add a database result to a database result list, this will takes over the
 * ownership of the database result.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \param[in] result a libdbo_result_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_list_add(libdbo_result_list_t* result_list, libdbo_result_t* result);

/**
 * Return the first database result in a database result list and reset the
 * position of the list.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \return a libdbo_result_t pointer or NULL on error or if the list is empty.
 */
const libdbo_result_t* libdbo_result_list_begin(libdbo_result_list_t* result_list);

/**
 * Return the next database result in a database result list.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \return a libdbo_result_t pointer or NULL on error or if the end of the list has
 * been reached.
 */
const libdbo_result_t* libdbo_result_list_next(libdbo_result_list_t* result_list);

/**
 * Return the size of the database result list.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \return a size_t with the size of the database result list or zero on error
 * , if the database result list is empty or if the backend does not support
 * returning the size.
 */
size_t libdbo_result_list_size(const libdbo_result_list_t* result_list);

/**
 * Make sure that all objects in this database result list is loaded into memory
 * so that libdbo_result_list_begin() can be used to iterate over the list multiple
 * times.
 * \param[in] result_list a libdbo_result_list_t pointer.
 * \return DB_ERROR_* on failure, otherwise DB_OK.
 */
int libdbo_result_list_fetch_all(libdbo_result_list_t* result_list);

/** \} */

#ifdef __cplusplus
}
#endif

#endif
