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
 * Based on enforcer-ng/src/db/db_type.h header file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/type.h */
/** \defgroup libdbo_type libdbo_type
 * Database Types.
 * These are the supported database values that can be used to retrieve or store
 * values in the backends. We also define our own 32/64bits types to use with
 * the database layer. \see libdbo_value
 */

#ifndef libdbo_type_h
#define libdbo_type_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup libdbo_type */
/** \{ */

/**
 * A signed 32bit integer.
 */
typedef int32_t libdbo_type_int32_t;
/**
 * An unsigned 32bit integer.
 */
typedef uint32_t libdbo_type_uint32_t;
/**
 * A signed 64bit integer.
 */
typedef int64_t libdbo_type_int64_t;
/**
 * An unsigned 64bit integer.
 */
typedef uint64_t libdbo_type_uint64_t;
/**
 * The type of a database value.
 */
typedef enum {
    /**
     * No value, empty, not set.
     */
    LIBDBO_TYPE_EMPTY,
    /**
     * This will make the value a primary key / ID that can be any type.
     */
    LIBDBO_TYPE_PRIMARY_KEY,
    /**
     * A libdbo_type_int32_t.
     */
    LIBDBO_TYPE_INT32,
    /**
     * A libdbo_type_uint32_t.
     */
    LIBDBO_TYPE_UINT32,
    /**
     * A libdbo_type_int64_t.
     */
    LIBDBO_TYPE_INT64,
    /**
     * A libdbo_type_uint64_t.
     */
    LIBDBO_TYPE_UINT64,
    /**
     * A null terminated character string.
     */
    LIBDBO_TYPE_TEXT,
    /**
     * A enumerate value that can be represented as an integer or string.
     */
    LIBDBO_TYPE_ENUM,
    /**
     * This can be any type, primarily used for ID fields.
     */
    LIBDBO_TYPE_ANY,
    /**
     * This is a special revision type that can be used to track revisions of
     * objects and only do changes against the current revision and in so will
     * fail if someone else has changed the object. The revision type can be
     * any type.
     */
    LIBDBO_TYPE_REVISION
} libdbo_type_t;

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_type_int32_t libdbo_type_int32_t
#define db_type_uint32_t libdbo_type_uint32_t
#define db_type_int64_t libdbo_type_int64_t
#define db_type_uint64_t libdbo_type_uint64_t
#define DB_TYPE_EMPTY LIBDBO_TYPE_EMPTY
#define DB_TYPE_PRIMARY_KEY LIBDBO_TYPE_PRIMARY_KEY
#define DB_TYPE_INT32 LIBDBO_TYPE_INT32
#define DB_TYPE_UINT32 LIBDBO_TYPE_UINT32
#define DB_TYPE_INT64 LIBDBO_TYPE_INT64
#define DB_TYPE_UINT64 LIBDBO_TYPE_UINT64
#define DB_TYPE_TEXT LIBDBO_TYPE_TEXT
#define DB_TYPE_ENUM LIBDBO_TYPE_ENUM
#define DB_TYPE_ANY LIBDBO_TYPE_ANY
#define DB_TYPE_REVISION LIBDBO_TYPE_REVISION
#endif
#endif

#endif
