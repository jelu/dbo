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
 * Based on enforcer-ng/src/db/db_join.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/join.h */
/** \defgroup libdbo_join libdbo_join
 * Database Join.
 * These are the functions and container for handling a database join.
 */
/** \defgroup libdbo_join_list libdbo_join_list
 * Database Join List.
 * These are the functions and container for handling database joins.
 */

#ifndef libdbo_join_h
#define libdbo_join_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_join;
struct libdbo_join_list;
#endif

/** \addtogroup libdbo_join */
/** \{ */
/**
 * A database join description.
 */
typedef struct libdbo_join libdbo_join_t;
/** \} */

/** \addtogroup libdbo_join_list */
/** \{ */
/**
 * A list of database joins.
 */
typedef struct libdbo_join_list libdbo_join_list_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/type.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_join {
    libdbo_join_t* next;
    char* from_table;
    char* from_field;
    char* to_table;
    char* to_field;
};
#endif

/** \addtogroup libdbo_join */
/** \{ */

/**
 * Create a new database join.
 * \return a libdbo_join_t pointer or NULL on error.
 */
libdbo_join_t* libdbo_join_new(void);

/**
 * Delete a database join.
 * \param[in] join a libdbo_join_t pointer.
 */
void libdbo_join_free(libdbo_join_t* join);

/**
 * Get the from table name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \return a character pointer or NULL on error or if no from table name has
 * been set.
 */
const char* libdbo_join_from_table(const libdbo_join_t* join);

/**
 * Get the from field name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \return a character pointer or NULL on error or if no from field name has
 * been set.
 */
const char* libdbo_join_from_field(const libdbo_join_t* join);

/**
 * Get the to table name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \return a character pointer or NULL on error or if no to table name has been
 * set.
 */
const char* libdbo_join_to_table(const libdbo_join_t* join);

/**
 * Get the to field name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \return a character pointer or NULL on error or if no to field name has been
 * set.
 */
const char* libdbo_join_to_field(const libdbo_join_t* join);

/**
 * Set the from table name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \param[in] from_table a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_join_set_from_table(libdbo_join_t* join, const char* from_table);

/**
 * Set the from field name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \param[in] from_field a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_join_set_from_field(libdbo_join_t* join, const char* from_field);

/**
 * Set the to table name of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \param[in] to_table a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_join_set_to_table(libdbo_join_t* join, const char* to_table);

/**
 * Set the to field of a database join.
 * \param[in] join a libdbo_join_t pointer.
 * \param[in] to_field a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_join_set_to_field(libdbo_join_t* join, const char* to_field);

/**
 * Check if the database join is not empty.
 * \param[in] join a libdbo_join_t pointer.
 * \return LIBDBO_ERROR_* if empty, otherwise LIBDBO_OK.
 */
int libdbo_join_not_empty(const libdbo_join_t* join);

/**
 * Get the next database join connected in a database join list.
 * \param[in] join a libdbo_join_t pointer.
 * \return a libdbo_join_t pointer or NULL on error or if there are no more database
 * joins in the list.
 */
const libdbo_join_t* libdbo_join_next(const libdbo_join_t* join);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_join_list {
    libdbo_join_t* begin;
    libdbo_join_t* end;
};
#endif

/** \addtogroup libdbo_join_list */
/** \{ */

/**
 * Create a new database join list.
 * \return a libdbo_join_list_t pointer or NULL on error.
 */
libdbo_join_list_t* libdbo_join_list_new(void);

/**
 * Delete a database join list and all database joins within the list.
 * \param[in] join_list a libdbo_join_list_t pointer.
 */
void libdbo_join_list_free(libdbo_join_list_t* join_list);

/**
 * Add a database join to a database join list, this takes over the ownership
 * of the database join.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \param[in] join a libdbo_join_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_join_list_add(libdbo_join_list_t* join_list, libdbo_join_t* join);

/**
 * Return the first database join in a database join list.
 * \param[in] join_list a libdbo_join_list_t pointer.
 * \return a libdbo_join_t pointer or NULL on error or if the list is empty.
 */
const libdbo_join_t* libdbo_join_list_begin(const libdbo_join_list_t* join_list);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_join_t libdbo_join_t
#define db_join_list_t libdbo_join_list_t
#define db_join_new(...) libdbo_join_new(__VA_ARGS__)
#define db_join_free(...) libdbo_join_free(__VA_ARGS__)
#define db_join_from_table(...) libdbo_join_from_table(__VA_ARGS__)
#define db_join_from_field(...) libdbo_join_from_field(__VA_ARGS__)
#define db_join_to_table(...) libdbo_join_to_table(__VA_ARGS__)
#define db_join_to_field(...) libdbo_join_to_field(__VA_ARGS__)
#define db_join_set_from_table(...) libdbo_join_set_from_table(__VA_ARGS__)
#define db_join_set_from_field(...) libdbo_join_set_from_field(__VA_ARGS__)
#define db_join_set_to_table(...) libdbo_join_set_to_table(__VA_ARGS__)
#define db_join_set_to_field(...) libdbo_join_set_to_field(__VA_ARGS__)
#define db_join_not_empty(...) libdbo_join_not_empty(__VA_ARGS__)
#define db_join_next(...) libdbo_join_next(__VA_ARGS__)
#define db_join_list_new(...) libdbo_join_list_new(__VA_ARGS__)
#define db_join_list_free(...) libdbo_join_list_free(__VA_ARGS__)
#define db_join_list_add(...) libdbo_join_list_add(__VA_ARGS__)
#define db_join_list_begin(...) libdbo_join_list_begin(__VA_ARGS__)
#endif
#endif

#endif
