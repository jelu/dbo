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
 * Based on enforcer-ng/src/db/db_backend_couchdb.h header file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/backend/couchdb.h */
/** \defgroup libdbo_backend_couchdb libdbo_backend_couchdb
 * Database Backend CouchDB.
 * These are the functions for creating a CouchDB backend handle.
 */

#ifndef libdbo_backend_couchdb_h
#define libdbo_backend_couchdb_h

#include <libdbo/backend.h>

/** \addtogroup libdbo_backend_couchdb */
/** \{ */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new database backend handle for CouchDB.
 * \return a libdbo_backend_handle_t pointer or NULL on error.
 */
libdbo_backend_handle_t* libdbo_backend_couchdb_new_handle(void);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define db_backend_couchdb_new_handle(...) libdbo_backend_couchdb_new_handle(__VA_ARGS__)
#endif
#endif

#endif
