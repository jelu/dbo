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
 * Based on enforcer-ng/src/db/db_value.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/value.h */
/** \defgroup libdbo_value libdbo_value
 * Database Value.
 * These are the functions and container for handling a database value.
 *
 * Example usage of a string value:
 * \code
#include <libdbo/libdbo.h>
#include <stdio.h>

int main(void) {
    libdbo_value_t* value;
    int error;

    if (!(value = libdbo_value_new())) {
        printf("Memory allocation error!\n");
        return -1;
    }

    if ((error = libdbo_value_from_text(value, "A string value"))) {
        printf("libdbo_value_from_text() error %d: %s\n", error, libdbo_errstr(error));
        libdbo_value_free(value);
        return -1;
    }

    printf("value: %s\n", libdbo_value_text(value));
    libdbo_value_free(value);
    return 0;
}
 * \endcode
 *
 * Example usage of an integer value:
 * \code
#include <libdbo/libdbo.h>
#include <stdio.h>

int main(void) {
    libdbo_value_t* value;
    int error;

    if (!(value = libdbo_value_new())) {
        printf("Memory allocation error!\n");
        return -1;
    }

    if ((error = libdbo_value_from_int64(value, 1234567890))) {
        printf("libdbo_value_from_int64() error %d: %s\n", error, libdbo_errstr(error));
        libdbo_value_free(value);
        return -1;
    }

    printf("value: %llu\n", libdbo_value_int64(value));
    libdbo_value_free(value);
    return 0;
}
 * \endcode
 *
 * Example usage of a static string value:
 * \code
#include <libdbo/libdbo.h>
#include <stdio.h>

static libdbo_value_t* value = LIBDBO_VALUE_EMPTY;

int main(void) {
    int error;

    if ((error = libdbo_value_from_text(value, "A string value"))) {
        printf("libdbo_value_from_text() error %d: %s\n", error, libdbo_errstr(error));
        libdbo_value_reset(value);
        return -1;
    }

    printf("value: %s\n", libdbo_value_text(value));
    libdbo_value_reset(value);
    return 0;
}
 * \endcode
 */
/** \defgroup libdbo_value_set libdbo_value_set
 * Database Value Set.
 * A fixed size database value set.
 */

#ifndef libdbo_value_h
#define libdbo_value_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_value;
struct libdbo_value_set;
#endif

/** \addtogroup libdbo_value */
/** \{ */
/**
 * A container for a database value.
 */
typedef struct libdbo_value libdbo_value_t;
/** \} */

/** \addtogroup libdbo_value_set */
/** \{ */
/**
 * A container for a fixed set of database values.
 */
typedef struct libdbo_value_set libdbo_value_set_t;
/** \} */

#ifdef __cplusplus
}
#endif

#include <libdbo/type.h>
#include <libdbo/enum.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_value {
    libdbo_type_t type;
    int primary_key;
    char* text;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    int enum_value;
    const char* enum_text;
};
#endif

/** \addtogroup libdbo_value */
/** \{ */

/**
 * Static database value initializer.
 */
#define LIBDBO_VALUE_EMPTY { LIBDBO_TYPE_EMPTY, 0, NULL, 0, 0, 0, 0, 0, NULL }

/**
 * Create a new database value.
 * \return a libdbo_value_t pointer or NULL on error.
 */
libdbo_value_t* libdbo_value_new();

/**
 * Create a new database value that is a copy of another.
 * \param[in] from_value a libdbo_value_t pointer.
 * \return a libdbo_value_t pointer or NULL on error.
 */
libdbo_value_t* libdbo_value_new_copy(const libdbo_value_t* from_value);

/**
 * Delete a database value.
 * \param[in] value a libdbo_value_t pointer.
 */
void libdbo_value_free(libdbo_value_t* value);

/**
 * Reset a database value, releasing all interal resources and marking it empty.
 * \param[in] value a libdbo_value_t pointer.
 */
void libdbo_value_reset(libdbo_value_t* value);

/**
 * Copy the contant from one database value into another.
 * \param[in] value a libdbo_value_t pointer to copy to.
 * \param[in] from_value a libdbo_value_t pointer to copy from.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_copy(libdbo_value_t* value, const libdbo_value_t* from_value);

/**
 * Compare two database values A and B. Sets `result` with less than, equal to,
 * or greater than zero if A is found, respectively, to be less than, to match,
 * or be greater than B.
 * \param[in] value_a a libdbo_value_t pointer.
 * \param[in] value_b a libdbo_value_t pointer.
 * \param[out] result an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_cmp(const libdbo_value_t* value_a, const libdbo_value_t* value_b, int* result);

/**
 * Get the type of a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a libdbo_type_t.
 */
libdbo_type_t libdbo_value_type(const libdbo_value_t* value);

/**
 * Get a pointer for the 32bit integer in a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a libdbo_type_int32_t pointer or NULL on error, if empty or not a 32bit
 * integer value.
 */
const libdbo_type_int32_t* libdbo_value_int32(const libdbo_value_t* value);

/**
 * Get a pointer for the unsigned 32bit integer in a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a libdbo_type_uint32_t pointer or NULL on error, if empty or not an
 * unsigned 32bit integer value.
 */
const libdbo_type_uint32_t* libdbo_value_uint32(const libdbo_value_t* value);

/**
 * Get a pointer for the 64bit integer in a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a libdbo_type_int64_t pointer or NULL on error, if empty or not a 64bit
 * integer value.
 */
const libdbo_type_int64_t* libdbo_value_int64(const libdbo_value_t* value);

/**
 * Get a pointer for the unsigned 64bit integer in a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a libdbo_type_uint64_t pointer or NULL on error, if empty or not an
 * unsigned 64bit integer value.
 */
const libdbo_type_uint64_t* libdbo_value_uint64(const libdbo_value_t* value);

/**
 * Get a character pointer for the text in a database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a character pointer or NULL on error, if empty or not a text value.
 */
const char* libdbo_value_text(const libdbo_value_t* value);

/**
 * Sets `enum_value` with the integer value of an enumeration database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] enum_value an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_enum_value(const libdbo_value_t* value, int* enum_value);

/**
 * Get the character representation of the integer value of an enumeration
 * database value.
 * \param[in] value a libdbo_value_t pointer.
 * \return a character pointer or NULL on error, if empty or not an enum value.
 */
const char* libdbo_value_enum_text(const libdbo_value_t* value);

/**
 * Check if a database value is not empty.
 * \param[in] value a libdbo_value_t pointer.
 * \return LIBDBO_ERROR_* if empty, otherwise LIBDBO_OK.
 */
int libdbo_value_not_empty(const libdbo_value_t* value);

/**
 * Get the 32bit integer representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_int32 a libdbo_type_int32_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_int32(const libdbo_value_t* value, libdbo_type_int32_t* to_int32);

/**
 * Get the unsigned 32bit integer representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_uint32 a libdbo_type_uint32_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_uint32(const libdbo_value_t* value, libdbo_type_uint32_t* to_uint32);

/**
 * Get the 64bit integer representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_int64 a libdbo_type_int64_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_int64(const libdbo_value_t* value, libdbo_type_int64_t* to_int64);

/**
 * Get the unsigned 64bit integer representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_uint64 a libdbo_type_uint64_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_uint64(const libdbo_value_t* value, libdbo_type_uint64_t* to_uint64);

/**
 * Get the character representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_text a character pointer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_text(const libdbo_value_t* value, char** to_text);

/**
 * Get the integer enumeration representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_int an integer pointer.
 * \param[in] enum_set a libdbo_enum_t array that MUST end with NULL.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_enum_value(const libdbo_value_t* value, int* to_int, const libdbo_enum_t* enum_set);

/**
 * Get the character enumeration representation of the database value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[out] to_text a character pointer pointer.
 * \param[in] enum_set a libdbo_enum_t array that MUST end with NULL.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_to_enum_text(const libdbo_value_t* value, const char** to_text, const libdbo_enum_t* enum_set);

/**
 * Set the database value to a 32bit integer value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_int32 a libdbo_type_int32_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_int32(libdbo_value_t* value, libdbo_type_int32_t from_int32);

/**
 * Set the database value to an unsigned 32bit integer value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_uint32 a libdbo_type_uint32_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_uint32(libdbo_value_t* value, libdbo_type_uint32_t from_uint32);

/**
 * Set the database value to a 64bit integer value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_int64 a libdbo_type_int64_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_int64(libdbo_value_t* value, libdbo_type_int64_t from_int64);

/**
 * Set the database value to an unsigned 64bit integer value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_uint64 a libdbo_type_uint64_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_uint64(libdbo_value_t* value, libdbo_type_uint64_t from_uint64);

/**
 * Set the database value to a text value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_text a character pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_text(libdbo_value_t* value, const char* from_text);

/**
 * Set the database value to a text value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] from_text a character pointer.
 * \param[in] size a size_t.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_text2(libdbo_value_t* value, const char* from_text, size_t size);

/**
 * Set the database value to an enumeration value based on an integer value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] enum_value an integer pointer.
 * \param[in] enum_set a libdbo_enum_t array that MUST end with NULL.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_enum_value(libdbo_value_t* value, int enum_value, const libdbo_enum_t* enum_set);

/**
 * Set the database value to an enumeration value based on a text value.
 * \param[in] value a libdbo_value_t pointer.
 * \param[in] enum_text a character pointer.
 * \param[in] enum_set a libdbo_enum_t array that MUST end with NULL.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_from_enum_text(libdbo_value_t* value, const char* enum_text, const libdbo_enum_t* enum_set);

/**
 * Check if the database value is a primary key.
 * \param[in] value a libdbo_value_t pointer.
 * \return LIBDBO_ERROR_* if its not a primary key, otherwise LIBDBO_OK.
 */
int libdbo_value_primary_key(const libdbo_value_t* value);

/**
 * Mark the database as a primary key.
 * \param[in] value a libdbo_value_t pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_value_set_primary_key(libdbo_value_t* value);

/** \} */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_value_set {
    libdbo_value_t* values;
    size_t size;
};
#endif

/** \addtogroup libdbo_value_set */
/** \{ */

/**
 * Create a new set of database value.
 * \param[in] size a size_t.
 * \return a libdbo_value_set_t pointer or NULL on error.
 */
libdbo_value_set_t* libdbo_value_set_new(size_t size);

/**
 * Create a new set of database value that is a copy of another.
 * \param[in] from_value_set a libdbo_value_set_t pointer.
 * \return a libdbo_value_set_t pointer or NULL on error.
 */
libdbo_value_set_t* libdbo_value_set_new_copy(const libdbo_value_set_t* from_value_set);

/**
 * Delete a database value set and all values within the set.
 * \param[in] value_set a libdbo_value_set_t pointer.
 */
void libdbo_value_set_free(libdbo_value_set_t* value_set);

/**
 * Get the size of database value set.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \return a size_t.
 */
size_t libdbo_value_set_size(const libdbo_value_set_t* value_set);

/**
 * Get a read only database value at a position in a database value set.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] at a size_t.
 * \return a libdbo_value_t pointer or NULL on error.
 */
const libdbo_value_t* libdbo_value_set_at(const libdbo_value_set_t* value_set, size_t at);

/**
 * Get a writable database value at a position in a database value set.
 * \param[in] value_set a libdbo_value_set_t pointer.
 * \param[in] at a size_t.
 * \return a libdbo_value_t pointer or NULL on error.
 */
libdbo_value_t* libdbo_value_set_get(libdbo_value_set_t* value_set, size_t at);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_value_t libdbo_value_t
#define db_value_set_t libdbo_value_set_t
#define DB_VALUE_EMPTY { DB_TYPE_EMPTY, 0, NULL, 0, 0, 0, 0, 0, NULL }
#define db_value_new(...) libdbo_value_new(__VA_ARGS__)
#define db_value_new_copy(...) libdbo_value_new_copy(__VA_ARGS__)
#define db_value_free(...) libdbo_value_free(__VA_ARGS__)
#define db_value_reset(...) libdbo_value_reset(__VA_ARGS__)
#define db_value_copy(...) libdbo_value_copy(__VA_ARGS__)
#define db_value_cmp(...) libdbo_value_cmp(__VA_ARGS__)
#define db_value_type(...) libdbo_value_type(__VA_ARGS__)
#define db_value_int32(...) libdbo_value_int32(__VA_ARGS__)
#define db_value_uint32(...) libdbo_value_uint32(__VA_ARGS__)
#define db_value_int64(...) libdbo_value_int64(__VA_ARGS__)
#define db_value_uint64(...) libdbo_value_uint64(__VA_ARGS__)
#define db_value_text(...) libdbo_value_text(__VA_ARGS__)
#define db_value_enum_value(...) libdbo_value_enum_value(__VA_ARGS__)
#define db_value_enum_text(...) libdbo_value_enum_text(__VA_ARGS__)
#define db_value_not_empty(...) libdbo_value_not_empty(__VA_ARGS__)
#define db_value_to_int32(...) libdbo_value_to_int32(__VA_ARGS__)
#define db_value_to_uint32(...) libdbo_value_to_uint32(__VA_ARGS__)
#define db_value_to_int64(...) libdbo_value_to_int64(__VA_ARGS__)
#define db_value_to_uint64(...) libdbo_value_to_uint64(__VA_ARGS__)
#define db_value_to_text(...) libdbo_value_to_text(__VA_ARGS__)
#define db_value_to_enum_value(...) libdbo_value_to_enum_value(__VA_ARGS__)
#define db_value_to_enum_text(...) libdbo_value_to_enum_text(__VA_ARGS__)
#define db_value_from_int32(...) libdbo_value_from_int32(__VA_ARGS__)
#define db_value_from_uint32(...) libdbo_value_from_uint32(__VA_ARGS__)
#define db_value_from_int64(...) libdbo_value_from_int64(__VA_ARGS__)
#define db_value_from_uint64(...) libdbo_value_from_uint64(__VA_ARGS__)
#define db_value_from_text(...) libdbo_value_from_text(__VA_ARGS__)
#define db_value_from_text2(...) libdbo_value_from_text2(__VA_ARGS__)
#define db_value_from_enum_value(...) libdbo_value_from_enum_value(__VA_ARGS__)
#define db_value_from_enum_text(...) libdbo_value_from_enum_text(__VA_ARGS__)
#define db_value_primary_key(...) libdbo_value_primary_key(__VA_ARGS__)
#define db_value_set_primary_key(...) libdbo_value_set_primary_key(__VA_ARGS__)
#define db_value_set_new(...) libdbo_value_set_new(__VA_ARGS__)
#define db_value_set_new_copy(...) libdbo_value_set_new_copy(__VA_ARGS__)
#define db_value_set_free(...) libdbo_value_set_free(__VA_ARGS__)
#define db_value_set_size(...) libdbo_value_set_size(__VA_ARGS__)
#define db_value_set_at(...) libdbo_value_set_at(__VA_ARGS__)
#define db_value_set_get(...) libdbo_value_set_get(__VA_ARGS__)
#endif
#endif

#endif
