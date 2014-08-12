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
 * Based on enforcer-ng/src/db/db_backend_mysql.h header file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/backend/mysql.h */
/** \defgroup libdbo_backend_mysql libdbo_backend_mysql
 * Database Backend MySQL.
 * These are the functions for creating a MySQL backend handle.
 */

#ifndef libdbo_backend_mysql_h
#define libdbo_backend_mysql_h

#include <libdbo/backend.h>

/** \addtogroup libdbo_backend_mysql */
/** \{ */

/**
 * Default connection timeout for MySQL.
 */
#define DB_BACKEND_MYSQL_DEFAULT_TIMEOUT 30
/**
 * Minimal allocation size when fetching varchar, text or blobs.
 */
#define DB_BACKEND_MYSQL_STRING_MIN_SIZE 64
/**
 * Maximum allocation size when fetching varchar, text or blobs. If the value in
 * the database is larger then this then the fetch will fail.
 */
#define DB_BACKEND_MYSQL_STRING_MAX_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new database backend handle for MySQL.
 * \return a libdbo_backend_handle_t pointer or NULL on error.
 */
libdbo_backend_handle_t* libdbo_backend_mysql_new_handle(void);

/** \} */

#ifdef __cplusplus
}
#endif

#endif
