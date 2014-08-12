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
 * Based on enforcer-ng/src/db/db_clause.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/clause.h */
/** \defgroup libdbo_clause libdbo_clause
 * Database Clause.
 * These are the functions and container for handling a database clause.
 */
/** \defgroup libdbo_clause_list libdbo_clause_list
 * Database Clause List.
 * These are the functions and container for handling database clauses.
 */

#ifndef libdbo_clause_h
#define libdbo_clause_h

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup libdbo_clause */
/** \{ */

/**
 * The clause operation to make on the value.
 */
typedef enum {
    /**
     * Empty, not set or unknown.
     */
    LIBDBO_CLAUSE_UNKNOWN,
    /**
     * ==
     */
    LIBDBO_CLAUSE_EQUAL,
    /**
     * !=
     */
    LIBDBO_CLAUSE_NOT_EQUAL,
    /**
     * <
     */
    LIBDBO_CLAUSE_LESS_THEN,
    /**
     * <=
     */
    LIBDBO_CLAUSE_LESS_OR_EQUAL,
    /**
     * >=
     */
    LIBDBO_CLAUSE_GREATER_OR_EQUAL,
    /**
     * >
     */
    LIBDBO_CLAUSE_GREATER_THEN,
    /**
     * Is null.
     */
    LIBDBO_CLAUSE_IS_NULL,
    /**
     * Is not null.
     */
    LIBDBO_CLAUSE_IS_NOT_NULL,
    /**
     * This adds a nested clause as in wrapping the content with ( ).
     */
    LIBDBO_CLAUSE_NESTED
} libdbo_clause_type_t;

/**
 * Short for LIBDBO_CLAUSE_EQUAL.
 */
#define LIBDBO_CLAUSE_EQ LIBDBO_CLAUSE_EQUAL
/**
 * Short for LIBDBO_CLAUSE_NOT_EQUAL.
 */
#define LIBDBO_CLAUSE_NE LIBDBO_CLAUSE_NOT_EQUAL
/**
 * Short for LIBDBO_CLAUSE_LESS_THEN.
 */
#define LIBDBO_CLAUSE_LT LIBDBO_CLAUSE_LESS_THEN
/**
 * Short for LIBDBO_CLAUSE_LESS_OR_EQUAL.
 */
#define LIBDBO_CLAUSE_LE LIBDBO_CLAUSE_LESS_OR_EQUAL
/**
 * Short for LIBDBO_CLAUSE_GREATER_OR_EQUAL.
 */
#define LIBDBO_CLAUSE_GE LIBDBO_CLAUSE_GREATER_OR_EQUAL
/**
 * Short for LIBDBO_CLAUSE_GREATER_THEN.
 */
#define LIBDBO_CLAUSE_GT LIBDBO_CLAUSE_GREATER_THEN

/**
 * The operator to do between the previous clause and this one.
 */
typedef enum {
    /**
     * Empty, not set or unknown.
     */
    LIBDBO_CLAUSE_OPERATOR_UNKNOWN,
    /**
     * ||
     */
    LIBDBO_CLAUSE_OPERATOR_AND,
    /**
     * &&
     */
    LIBDBO_CLAUSE_OPERATOR_OR
} libdbo_clause_operator_t;

/**
 * Short for LIBDBO_CLAUSE_OPERATOR_AND.
 */
#define LIBDBO_CLAUSE_OP_AND LIBDBO_CLAUSE_OPERATOR_AND
/**
 * Short for LIBDBO_CLAUSE_OPERATOR_OR.
 */
#define LIBDBO_CLAUSE_OP_OR  LIBDBO_CLAUSE_OPERATOR_OR

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_clause;
struct libdbo_clause_list;
#endif

/**
 * A database clause, describes the comparison of a database object field and a
 * value.
 */
typedef struct libdbo_clause libdbo_clause_t;
/** \} */

/** \addtogroup libdbo_clause_list */
/** \{ */
/**
 * A list of database clauses.
 */
typedef struct libdbo_clause_list libdbo_clause_list_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/value.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_clause {
    libdbo_clause_t* next;
    char* table;
    char* field;
    libdbo_clause_type_t type;
    libdbo_value_t value;
    libdbo_clause_operator_t clause_operator;
    libdbo_clause_list_t* clause_list;
};
#endif

/** \addtogroup libdbo_clause */
/** \{ */

/**
 * Create a new database clause.
 * \return a libdbo_clause_t pointer or NULL on error.
 */
libdbo_clause_t* libdbo_clause_new(void);

/**
 * Delete a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 */
void libdbo_clause_free(libdbo_clause_t* clause);

/**
 * Get the table name of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a character pointer or NULL on error or if no table name has been set.
 */
const char* libdbo_clause_table(const libdbo_clause_t* clause);

/**
 * Get the field name of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a character pointer or NULL on error or if no field name has been set.
 */
const char* libdbo_clause_field(const libdbo_clause_t* clause);

/**
 * Get the database clause type of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_clause_type_t.
 */
libdbo_clause_type_t libdbo_clause_type(const libdbo_clause_t* clause);

/**
 * Get the database value of a database value.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_value_t pointer or NULL on error.
 */
const libdbo_value_t* libdbo_clause_value(const libdbo_clause_t* clause);

/**
 * Get the database clause operator of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_clause_operator_t.
 */
libdbo_clause_operator_t libdbo_clause_operator(const libdbo_clause_t* clause);

/**
 * Get the database clause list of a database clause, this is used for nested
 * database clauses.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_clause_list_t pointer or NULL on error or if no database clause
 * list has been set.
 */
const libdbo_clause_list_t* libdbo_clause_list(const libdbo_clause_t* clause);

/**
 * Set the table name of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \param[in] table a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_set_table(libdbo_clause_t* clause, const char* table);

/**
 * Set the field name of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \param[in] field a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_set_field(libdbo_clause_t* clause, const char* field);

/**
 * Set the database clause type of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \param[in] type a libdbo_clause_type_t.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_set_type(libdbo_clause_t* clause, libdbo_clause_type_t type);

/**
 * Set the database clause operator of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \param[in] clause_operator a libdbo_clause_operator_t.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_set_operator(libdbo_clause_t* clause, libdbo_clause_operator_t clause_operator);

/**
 * Set the database clause list of a database clause, this is used for nested
 * database clauses. The ownership of the database clause list it taken.
 * \param[in] clause a libdbo_clause_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_set_list(libdbo_clause_t* clause, libdbo_clause_list_t* clause_list);

/**
 * Check if the database clause is not empty.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return LIBDBO_ERROR_* if empty, otherwise LIBDBO_OK.
 */
int libdbo_clause_not_empty(const libdbo_clause_t* clause);

/**
 * Return the next database clause connected in a database clause list.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_clause_t pointer or NULL on error or if there are no more
 * database clauses in the list.
 */
const libdbo_clause_t* libdbo_clause_next(const libdbo_clause_t* clause);

/**
 * Get the writable database value of a database clause.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return a libdbo_value_t pointer or NULL on error.
 */
libdbo_value_t* libdbo_clause_get_value(libdbo_clause_t* clause);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_clause_list {
    libdbo_clause_t* begin;
    libdbo_clause_t* end;
};
#endif

/** \addtogroup libdbo_clause_list */
/** \{ */

/**
 * Create a new database clause list.
 * \return a libdbo_clause_list_t pointer or NULL on error.
 */
libdbo_clause_list_t* libdbo_clause_list_new(void);

/**
 * Delete a database clause list and all database clauses in the list.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 */
void libdbo_clause_list_free(libdbo_clause_list_t* clause_list);

/**
 * Add a database clause to a database clause list, this takes over the
 * ownership of the database clause.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[in] clause a libdbo_clause_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_clause_list_add(libdbo_clause_list_t* clause_list, libdbo_clause_t* clause);

/**
 * Return the first database clause of a database clause list.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \return a libdbo_clause_t pointer or NULL on error or if the list is empty.
 */
const libdbo_clause_t* libdbo_clause_list_begin(const libdbo_clause_list_t* clause_list);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define DB_CLAUSE_UNKNOWN LIBDBO_CLAUSE_UNKNOWN
#define DB_CLAUSE_EQUAL LIBDBO_CLAUSE_EQUAL
#define DB_CLAUSE_NOT_EQUAL LIBDBO_CLAUSE_NOT_EQUAL
#define DB_CLAUSE_LESS_THEN LIBDBO_CLAUSE_LESS_THEN
#define DB_CLAUSE_LESS_OR_EQUAL LIBDBO_CLAUSE_LESS_OR_EQUAL
#define DB_CLAUSE_GREATER_OR_EQUAL LIBDBO_CLAUSE_GREATER_OR_EQUAL
#define DB_CLAUSE_GREATER_THEN LIBDBO_CLAUSE_GREATER_THEN
#define DB_CLAUSE_IS_NULL LIBDBO_CLAUSE_IS_NULL
#define DB_CLAUSE_IS_NOT_NULL LIBDBO_CLAUSE_IS_NOT_NULL
#define DB_CLAUSE_NESTED LIBDBO_CLAUSE_NESTED
#define DB_CLAUSE_OPERATOR_UNKNOWN LIBDBO_CLAUSE_OPERATOR_UNKNOWN
#define DB_CLAUSE_OPERATOR_AND LIBDBO_CLAUSE_OPERATOR_AND
#define DB_CLAUSE_OPERATOR_OR LIBDBO_CLAUSE_OPERATOR_OR
#define db_clause_t libdbo_clause_t
#define db_clause_list_t libdbo_clause_list_t
#define db_clause_new(...) libdbo_clause_new(__VA_ARGS__)
#define db_clause_free(...) libdbo_clause_free(__VA_ARGS__)
#define db_clause_table(...) libdbo_clause_table(__VA_ARGS__)
#define db_clause_field(...) libdbo_clause_field(__VA_ARGS__)
#define db_clause_type(...) libdbo_clause_type(__VA_ARGS__)
#define db_clause_value(...) libdbo_clause_value(__VA_ARGS__)
#define db_clause_operator(...) libdbo_clause_operator(__VA_ARGS__)
#define db_clause_list(...) libdbo_clause_list(__VA_ARGS__)
#define db_clause_set_table(...) libdbo_clause_set_table(__VA_ARGS__)
#define db_clause_set_field(...) libdbo_clause_set_field(__VA_ARGS__)
#define db_clause_set_type(...) libdbo_clause_set_type(__VA_ARGS__)
#define db_clause_set_operator(...) libdbo_clause_set_operator(__VA_ARGS__)
#define db_clause_set_list(...) libdbo_clause_set_list(__VA_ARGS__)
#define db_clause_not_empty(...) libdbo_clause_not_empty(__VA_ARGS__)
#define db_clause_next(...) libdbo_clause_next(__VA_ARGS__)
#define db_clause_get_value(...) libdbo_clause_get_value(__VA_ARGS__)
#define db_clause_list_new(...) libdbo_clause_list_new(__VA_ARGS__)
#define db_clause_list_free(...) libdbo_clause_list_free(__VA_ARGS__)
#define db_clause_list_add(...) libdbo_clause_list_add(__VA_ARGS__)
#define db_clause_list_begin(...) libdbo_clause_list_begin(__VA_ARGS__)
#endif
#endif

#endif
