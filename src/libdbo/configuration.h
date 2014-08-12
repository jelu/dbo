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
 * Based on enforcer-ng/src/db/db_configuration.h header file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/configuration.h */
/** \defgroup libdbo_configuration libdbo_configuration
 * Database Configuration.
 * These are the functions and container for handling a database configuration
 * value.
 */
/** \defgroup libdbo_configuration_list libdbo_configuration_list
 * Database Configuration List.
 * These are the functions and container for handling database configurations.
 */

#ifndef libdbo_configuration_h
#define libdbo_configuration_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_configuration;
struct libdbo_configuration_list;
#endif

/** \addtogroup libdbo_configuration */
/** \{ */
/**
 * A database configuration represented by a key and value.
 */
typedef struct libdbo_configuration libdbo_configuration_t;
/** \} */
/** \addtogroup libdbo_configuration_list */
/** \{ */
/**
 * A list of database configurations.
 */
typedef struct libdbo_configuration_list libdbo_configuration_list_t;
/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_configuration {
    libdbo_configuration_t* next;
    char* name;
    char* value;
};
#endif

/** \addtogroup libdbo_configuration */
/** \{ */

/**
 * Create a new database configuration.
 * \return a libdbo_configuration_t pointer or NULL on error.
 */
libdbo_configuration_t* libdbo_configuration_new(void);

/**
 * Delete a database configuration.
 * \param[in] configuration a libdbo_configuration_t pointer.
 */
void libdbo_configuration_free(libdbo_configuration_t* configuration);

/**
 * Get the name of a database configuration.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \return a character pointer or NULL on error or if no database configuration
 * name has been set.
 */
const char* libdbo_configuration_name(const libdbo_configuration_t* configuration);

/**
 * Get the value of a database configuration.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \return a character pointer or NULL on error or if no database configuration
 * value has been set.
 */
const char* libdbo_configuration_value(const libdbo_configuration_t* configuration);

/**
 * Set the name of a database configuration.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \param[in] name a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_configuration_set_name(libdbo_configuration_t* configuration, const char* name);

/**
 * Set the value of a database configuration.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \param[in] value a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_configuration_set_value(libdbo_configuration_t* configuration, const char* value);

/**
 * Check if the database configuration is not empty.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \return LIBDBO_ERROR_* if empty, otherwise LIBDBO_OK.
 */
int libdbo_configuration_not_empty(const libdbo_configuration_t* configuration);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_configuration_list {
    libdbo_configuration_t* begin;
    libdbo_configuration_t* end;
};
#endif

/** \addtogroup libdbo_configuration_list */
/** \{ */

/**
 * Create a new database configuration list.
 * \return a libdbo_configuration_list_t pointer or NULL on error.
 */
libdbo_configuration_list_t* libdbo_configuration_list_new(void);

/**
 * Delete a database configuration list and all database configurations in the
 * list.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 */
void libdbo_configuration_list_free(libdbo_configuration_list_t* configuration_list);

/**
 * Add a database configuration to a database configuration list, this takes
 * over the ownership of the database configuration.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \param[in] configuration a libdbo_configuration_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_configuration_list_add(libdbo_configuration_list_t* configuration_list, libdbo_configuration_t* configuration);

/**
 * Find a database configuration by name within a database configuration list.
 * \param[in] configuration_list a libdbo_configuration_list_t pointer.
 * \param[in] name a character pointer.
 * \return a libdbo_configuration_t pointer or NULL on error or if the database
 * configuration does not exist.
 */
const libdbo_configuration_t* libdbo_configuration_list_find(const libdbo_configuration_list_t* configuration_list, const char* name);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_configuration_t libdbo_configuration_t
#define db_configuration_list_t libdbo_configuration_list_t
#define db_configuration_new(...) libdbo_configuration_new(__VA_ARGS__)
#define db_configuration_free(...) libdbo_configuration_free(__VA_ARGS__)
#define db_configuration_name(...) libdbo_configuration_name(__VA_ARGS__)
#define db_configuration_value(...) libdbo_configuration_value(__VA_ARGS__)
#define db_configuration_set_name(...) libdbo_configuration_set_name(__VA_ARGS__)
#define db_configuration_set_value(...) libdbo_configuration_set_value(__VA_ARGS__)
#define db_configuration_not_empty(...) libdbo_configuration_not_empty(__VA_ARGS__)
#define db_configuration_list_new(...) libdbo_configuration_list_new(__VA_ARGS__)
#define db_configuration_list_free(...) libdbo_configuration_list_free(__VA_ARGS__)
#define db_configuration_list_add(...) libdbo_configuration_list_add(__VA_ARGS__)
#define db_configuration_list_find(...) libdbo_configuration_list_find(__VA_ARGS__)
#endif
#endif

#endif
