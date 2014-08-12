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
 * Based on enforcer-ng/src/db/db_backend_sqlite.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/backend/sqlite.h"

#include "libdbo/error.h"
#include "libdbo/mm.h"

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

static int libdbo_backend_sqlite_transaction_rollback(void*);

/**
 * Keep track of if we have initialized the SQLite backend.
 */
static int __sqlite3_initialized = 0;

/**
 * A pthread mutex and cond to use for SQLite database locks / busy handler.
 *
 * SQLite may lock the database if a thread wants to write to the database then
 * all other threads needs to wait for that action to be completed and SQLite
 * only releases the lock when the statement is finalized. Our busy handler
 * function waits for a cond to be signaled instead of sleeping a fix amount of
 * time and in so speeds up access to the database once the lock is released.
 */
static pthread_mutex_t __sqlite_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t __sqlite_cond = PTHREAD_COND_INITIALIZER;

/**
 * The SQLite database backend specific data.
 */
typedef struct libdbo_backend_sqlite {
    sqlite3* db;
    int transaction;
    int timeout;
    int time;
    long usleep;
} libdbo_backend_sqlite_t;

static libdbo_mm_t __sqlite_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_sqlite_t));

/**
 * The SQLite database backend specific data for walking a result.
 */
typedef struct libdbo_backend_sqlite_statement {
    libdbo_backend_sqlite_t* backend_sqlite;
    sqlite3_stmt* statement;
    int fields;
    const libdbo_object_t* object;
} libdbo_backend_sqlite_statement_t;

static libdbo_mm_t __sqlite_statement_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_backend_sqlite_statement_t));

/**
 * The SQLite bust handler that is used to wait for database access.
 */
static int __db_backend_sqlite_busy_handler(void *data, int retry) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    struct timespec busy_ts;
    int rc;
    (void)retry;

    if (!backend_sqlite) {
        return 0;
    }

    /*ods_log_deeebug("libdbo_backend_sqlite_busy_handler: Database busy, waiting...");*/

    if (pthread_mutex_lock(&__sqlite_mutex)) {
        /*ods_log_error("libdbo_backend_sqlite_busy_handler: Mutex error");*/
        return 0;
    }
    if (clock_gettime(CLOCK_REALTIME, &busy_ts)) {
        pthread_mutex_unlock(&__sqlite_mutex);
        return 0;
    }

    busy_ts.tv_nsec += backend_sqlite->usleep * 1000;
    if (busy_ts.tv_nsec > 999999999) {
        busy_ts.tv_sec += (busy_ts.tv_nsec / 1000000000);
        busy_ts.tv_nsec -= (busy_ts.tv_nsec / 1000000000) * 1000000000;
    }

    rc = pthread_cond_timedwait(&__sqlite_cond, &__sqlite_mutex, &busy_ts);
    if (rc == ETIMEDOUT) {
        if (time(NULL) < (backend_sqlite->time + backend_sqlite->timeout)) {
            /*ods_log_deeebug("libdbo_backend_sqlite_busy_handler: Woke up, checking database...");*/
            pthread_mutex_unlock(&__sqlite_mutex);
            return 1;
        }
        pthread_mutex_unlock(&__sqlite_mutex);
        return 0;
    }
    else if (rc) {
        /*ods_log_error("libdbo_backend_sqlite_busy_handler: pthread_cond_timedwait() error %d", rc);*/
        pthread_mutex_unlock(&__sqlite_mutex);
        return 0;
    }

    /*ods_log_deeebug("libdbo_backend_sqlite_busy_handler: Woke up, checking database...");*/
    pthread_mutex_unlock(&__sqlite_mutex);
    return 1;
}

/**
 * SQLite prepare function.
 */
static inline int __db_backend_sqlite_prepare(libdbo_backend_sqlite_t* backend_sqlite, sqlite3_stmt** statement, const char* sql, size_t size) {
    int ret;

    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (*statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!sql) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*ods_log_debug("%s", sql);*/
    backend_sqlite->time = time(NULL);
    ret = sqlite3_prepare_v2(backend_sqlite->db,
        sql,
        size,
        statement,
        NULL);
    if (ret != SQLITE_OK) {
        /*ods_log_info("DB prepare SQL %s", sql);
        ods_log_info("DB prepare Err %d", ret);*/
        if (*statement) {
            sqlite3_finalize(*statement);
        }
        *statement = NULL;
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

/**
 * SQLite step function.
 */
static inline int __db_backend_sqlite_step(libdbo_backend_sqlite_t* backend_sqlite, sqlite3_stmt* statement) {
    /*
    struct timespec busy_ts;
    int rc, ret, been_busy = 0;
    */
    int ret;

    if (!backend_sqlite) {
        return SQLITE_INTERNAL;
    }
    if (!statement) {
        return SQLITE_INTERNAL;
    }

    backend_sqlite->time = time(NULL);
    ret = sqlite3_step(statement);
    /*
    if (ret == SQLITE_BUSY) {
        ods_log_deeebug("libdbo_backend_sqlite_step: Database busy, waiting...");
    }
    while (ret == SQLITE_BUSY) {
        if (pthread_mutex_lock(&__sqlite_mutex)) {
            ods_log_error("libdbo_backend_sqlite_step: Mutex error");
            return ret;
        }
        if (clock_gettime(CLOCK_REALTIME, &busy_ts)) {
            pthread_mutex_unlock(&__sqlite_mutex);
            return ret;
        }

        busy_ts.tv_sec += backend_sqlite->timeout;

        rc = pthread_cond_timedwait(&__sqlite_cond, &__sqlite_mutex, &busy_ts);
        if (rc == ETIMEDOUT) {
            pthread_mutex_unlock(&__sqlite_mutex);
            return ret;
        }
        else if (rc) {
            ods_log_error("libdbo_backend_sqlite_step: pthread_cond_timedwait() error %d", rc);
            pthread_mutex_unlock(&__sqlite_mutex);
            return ret;
        }

        ods_log_deeebug("libdbo_backend_sqlite_step: Woke up, checking database...");
        ret = sqlite3_step(statement);
        pthread_mutex_unlock(&__sqlite_mutex);
    }
    if (been_busy) {
        ods_log_deeebug("libdbo_backend_sqlite_step: Got lock or failed/timed out");
    }
    */

    return ret;
}

/**
 * SQLite finalize function.
 *
 * This will also signal the pthread cond that is used for busy handler.
 */
static inline int __db_backend_sqlite_finalize(sqlite3_stmt* statement) {
    int ret;

    ret = sqlite3_finalize(statement);
    pthread_cond_broadcast(&__sqlite_cond);

    return ret;
}

static int libdbo_backend_sqlite_initialize(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;

    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!__sqlite3_initialized) {
        int ret = sqlite3_initialize();
        if (ret != SQLITE_OK) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        __sqlite3_initialized = 1;
    }
    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_shutdown(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;

    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__sqlite3_initialized) {
        int ret = sqlite3_shutdown();
        if (ret != SQLITE_OK) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        __sqlite3_initialized = 0;
    }
    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_connect(void* data, const libdbo_configuration_list_t* configuration_list) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    const libdbo_configuration_t* file;
    const libdbo_configuration_t* timeout;
    const libdbo_configuration_t* usleep;
    int ret;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_sqlite->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(file = libdbo_configuration_list_find(configuration_list, "file"))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    backend_sqlite->timeout = LIBDBO_BACKEND_SQLITE_DEFAULT_TIMEOUT;
    if ((timeout = libdbo_configuration_list_find(configuration_list, "timeout"))) {
        backend_sqlite->timeout = atoi(libdbo_configuration_value(timeout));
        if (backend_sqlite->timeout < 1) {
            backend_sqlite->timeout = LIBDBO_BACKEND_SQLITE_DEFAULT_TIMEOUT;
        }
    }

    backend_sqlite->usleep = LIBDBO_BACKEND_SQLITE_DEFAULT_USLEEP;
    if ((usleep = libdbo_configuration_list_find(configuration_list, "usleep"))) {
        backend_sqlite->usleep = atoi(libdbo_configuration_value(usleep));
        if (backend_sqlite->usleep < 1) {
            backend_sqlite->usleep = LIBDBO_BACKEND_SQLITE_DEFAULT_TIMEOUT;
        }
    }

    ret = sqlite3_open_v2(
        libdbo_configuration_value(file),
        &(backend_sqlite->db),
        SQLITE_OPEN_READWRITE
        | SQLITE_OPEN_FULLMUTEX,
        NULL);
    if (ret != SQLITE_OK) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if ((ret = sqlite3_busy_handler(backend_sqlite->db, __db_backend_sqlite_busy_handler, backend_sqlite)) != SQLITE_OK) {
        /*ods_log_error("libdbo_backend_sqlite: sqlite3_busy_handler() error %d", ret);*/
        sqlite3_close(backend_sqlite->db);
        backend_sqlite->db = NULL;
        return LIBDBO_ERROR_UNKNOWN;
    }

    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_disconnect(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    int ret;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite->db) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (backend_sqlite->transaction) {
        libdbo_backend_sqlite_transaction_rollback(backend_sqlite);
    }
    ret = sqlite3_close(backend_sqlite->db);
    if (ret != SQLITE_OK) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    backend_sqlite->db = NULL;
    return LIBDBO_OK;
}

/**
 * Build the clause/WHERE SQL and append it to `sqlp`, how much that is left in
 * the buffer pointed by `sqlp` is specified by `left`.
 * \param[in] object a libdbo_object_t pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[in] sqlp a character pointer pointer.
 * \param[in] left an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_sqlite_build_clause(const libdbo_object_t* object, const libdbo_clause_list_t* clause_list, char** sqlp, int* left) {
    const libdbo_clause_t* clause;
    int first, ret;

    if (!clause_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!sqlp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*sqlp) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (*left < 1) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    first = 1;
    while (clause) {
        if (first) {
            first = 0;
        }
        else {
            switch (libdbo_clause_operator(clause)) {
            case LIBDBO_CLAUSE_OPERATOR_AND:
                if ((ret = snprintf(*sqlp, *left, " AND")) >= *left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_CLAUSE_OPERATOR_OR:
                if ((ret = snprintf(*sqlp, *left, " OR")) >= *left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            default:
                return LIBDBO_ERROR_UNKNOWN;
            }
            *sqlp += ret;
            *left -= ret;
        }

        switch (libdbo_clause_type(clause)) {
        case LIBDBO_CLAUSE_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s = ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_NOT_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s != ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_THEN:
            if ((ret = snprintf(*sqlp, *left, " %s.%s < ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_LESS_OR_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s <= ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_OR_EQUAL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s >= ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_GREATER_THEN:
            if ((ret = snprintf(*sqlp, *left, " %s.%s > ?",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NULL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s IS NULL",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NOT_NULL:
            if ((ret = snprintf(*sqlp, *left, " %s.%s IS NOT NULL",
                (libdbo_clause_table(clause) ? libdbo_clause_table(clause) : libdbo_object_table(object)),
                libdbo_clause_field(clause))) >= *left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_NESTED:
            if ((ret = snprintf(*sqlp, *left, " (")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            *sqlp += ret;
            *left -= ret;
            if (__db_backend_sqlite_build_clause(object, libdbo_clause_list(clause), sqlp, left)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            if ((ret = snprintf(*sqlp, *left, " )")) >= *left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        *sqlp += ret;
        *left -= ret;

        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

/**
 * Bind values from the clause list to the SQLite statement, `bind` contains the
 * position of the bind value.
 * \param[in] statement a sqlite3_stmt pointer.
 * \param[in] clause_list a libdbo_clause_list_t pointer.
 * \param[in] bind an integer pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
static int __db_backend_sqlite_bind_clause(sqlite3_stmt* statement, const libdbo_clause_list_t* clause_list, int* bind) {
    const libdbo_clause_t* clause;
    int ret;
    int to_int;
    sqlite3_int64 to_int64;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;

    if (!statement) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!clause_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!*bind) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    clause = libdbo_clause_list_begin(clause_list);
    while (clause) {
        switch (libdbo_clause_type(clause)) {
        case LIBDBO_CLAUSE_EQUAL:
        case LIBDBO_CLAUSE_NOT_EQUAL:
        case LIBDBO_CLAUSE_LESS_THEN:
        case LIBDBO_CLAUSE_LESS_OR_EQUAL:
        case LIBDBO_CLAUSE_GREATER_OR_EQUAL:
        case LIBDBO_CLAUSE_GREATER_THEN:
            switch (libdbo_value_type(libdbo_clause_value(clause))) {
            case LIBDBO_TYPE_PRIMARY_KEY:
            case LIBDBO_TYPE_INT32:
                if (libdbo_value_to_int32(libdbo_clause_value(clause), &int32)) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                to_int = int32;
                ret = sqlite3_bind_int(statement, (*bind)++, to_int);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_UINT32:
                if (libdbo_value_to_uint32(libdbo_clause_value(clause), &uint32)) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                to_int = uint32;
                ret = sqlite3_bind_int(statement, (*bind)++, to_int);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_INT64:
                if (libdbo_value_to_int64(libdbo_clause_value(clause), &int64)) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                to_int64 = int64;
                ret = sqlite3_bind_int64(statement, (*bind)++, to_int64);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_UINT64:
                if (libdbo_value_to_uint64(libdbo_clause_value(clause), &uint64)) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                to_int64 = uint64;
                ret = sqlite3_bind_int64(statement, (*bind)++, to_int64);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_TEXT:
                ret = sqlite3_bind_text(statement, (*bind)++, libdbo_value_text(libdbo_clause_value(clause)), -1, SQLITE_TRANSIENT);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            case LIBDBO_TYPE_ENUM:
                if (libdbo_value_enum_value(libdbo_clause_value(clause), &to_int)) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                ret = sqlite3_bind_int(statement, (*bind)++, to_int);
                if (ret != SQLITE_OK) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                break;

            default:
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_CLAUSE_IS_NULL:
        case LIBDBO_CLAUSE_IS_NOT_NULL:
            break;

        case LIBDBO_CLAUSE_NESTED:
            if (__db_backend_sqlite_bind_clause(statement, libdbo_clause_list(clause), bind)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
        clause = libdbo_clause_next(clause);
    }
    return LIBDBO_OK;
}

static libdbo_result_t* libdbo_backend_sqlite_next(void* data, int finish) {
    libdbo_backend_sqlite_statement_t* statement = (libdbo_backend_sqlite_statement_t*)data;
    int ret;
    int bind;
    libdbo_result_t* result = NULL;
    libdbo_value_set_t* value_set = NULL;
    const libdbo_object_field_t* object_field;
    int from_int;
    sqlite3_int64 from_int64;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    const char* text;

    if (!statement) {
        return NULL;
    }
    if (!statement->object) {
        return NULL;
    }
    if (!statement->statement) {
        return NULL;
    }

    if (finish) {
        __db_backend_sqlite_finalize(statement->statement);
        libdbo_mm_delete(&__sqlite_statement_alloc, statement);
        return NULL;
    }

    if (__db_backend_sqlite_step(statement->backend_sqlite, statement->statement) != SQLITE_ROW) {
        return NULL;
    }

    if (!(result = libdbo_result_new())
        || !(value_set = libdbo_value_set_new(statement->fields))
        || libdbo_result_set_value_set(result, value_set))
    {
        libdbo_result_free(result);
        libdbo_value_set_free(value_set);
        return NULL;
    }
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(statement->object));
    bind = 0;
    while (object_field) {
        switch (libdbo_object_field_type(object_field)) {
        case LIBDBO_TYPE_PRIMARY_KEY:
            from_int = sqlite3_column_int(statement->statement, bind);
            int32 = from_int;
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_int32(libdbo_value_set_get(value_set, bind), int32)
                || libdbo_value_set_primary_key(libdbo_value_set_get(value_set, bind)))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_ENUM:
            /*
             * Enum needs to be handled elsewhere since we don't know the
             * enum_set_t here.
             */
        case LIBDBO_TYPE_INT32:
            from_int = sqlite3_column_int(statement->statement, bind);
            int32 = from_int;
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_int32(libdbo_value_set_get(value_set, bind), int32))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            from_int = sqlite3_column_int(statement->statement, bind);
            uint32 = from_int;
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_uint32(libdbo_value_set_get(value_set, bind), uint32))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_INT64:
            from_int64 = sqlite3_column_int64(statement->statement, bind);
            int64 = from_int64;
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_int64(libdbo_value_set_get(value_set, bind), int64))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            from_int64 = sqlite3_column_int64(statement->statement, bind);
            uint64 = from_int64;
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_uint64(libdbo_value_set_get(value_set, bind), uint64))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            text = (const char*)sqlite3_column_text(statement->statement, bind);
            ret = sqlite3_errcode(statement->backend_sqlite->db);
            if (!text
                || (ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                || libdbo_value_from_text(libdbo_value_set_get(value_set, bind), text))
            {
                libdbo_result_free(result);
                return NULL;
            }
            break;

        case LIBDBO_TYPE_ANY:
        case LIBDBO_TYPE_REVISION:
            switch (sqlite3_column_type(statement->statement, bind)) {
            case SQLITE_INTEGER:
                from_int64 = sqlite3_column_int64(statement->statement, bind);
                int64 = from_int64;
                ret = sqlite3_errcode(statement->backend_sqlite->db);
                if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                    || libdbo_value_from_int64(libdbo_value_set_get(value_set, bind), int64))
                {
                    libdbo_result_free(result);
                    return NULL;
                }
                break;

            case SQLITE_TEXT:
                text = (const char*)sqlite3_column_text(statement->statement, bind);
                ret = sqlite3_errcode(statement->backend_sqlite->db);
                if (!text
                    || (ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)
                    || libdbo_value_from_text(libdbo_value_set_get(value_set, bind), text))
                {
                    libdbo_result_free(result);
                    return NULL;
                }
                break;

            default:
                libdbo_result_free(result);
                return NULL;
            }
            break;

        default:
            libdbo_result_free(result);
            return NULL;
        }
        object_field = libdbo_object_field_next(object_field);
        bind++;
    }
    return result;
}

static int libdbo_backend_sqlite_create(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_object_field_t* revision_field = NULL;
    const libdbo_value_t* value;
    char sql[4*1024];
    char* sqlp;
    int ret, left, bind, first;
    sqlite3_stmt* statement = NULL;
    size_t value_pos;
    int to_int;
    sqlite3_int64 to_int64;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }

    left = sizeof(sql);
    sqlp = sql;

    if (!libdbo_object_field_list_begin(object_field_list) && !revision_field) {
        /*
         * Special case when tables has no fields except maybe a primary key.
         */
        if ((ret = snprintf(sqlp, left, "INSERT INTO %s DEFAULT VALUES", libdbo_object_table(object))) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;
    }
    else {
        if ((ret = snprintf(sqlp, left, "INSERT INTO %s (", libdbo_object_table(object))) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;

        /*
         * Add the fields from the given object_field_list.
         */
        object_field = libdbo_object_field_list_begin(object_field_list);
        first = 1;
        while (object_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " %s", libdbo_object_field_name(object_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", %s", libdbo_object_field_name(object_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;

            object_field = libdbo_object_field_next(object_field);
        }

        /*
         * Add the revision field if we have one.
         */
        if (revision_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " %s", libdbo_object_field_name(revision_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", %s", libdbo_object_field_name(revision_field))) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;
        }

        if ((ret = snprintf(sqlp, left, " ) VALUES (")) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;

        /*
         * Mark all the fields for binding from the object_field_list.
         */
        object_field = libdbo_object_field_list_begin(object_field_list);
        first = 1;
        while (object_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;

            object_field = libdbo_object_field_next(object_field);
        }

        /*
         * Mark revision field for binding if we have one.
         */
        if (revision_field) {
            if (first) {
                if ((ret = snprintf(sqlp, left, " ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
                first = 0;
            }
            else {
                if ((ret = snprintf(sqlp, left, ", ?")) >= left) {
                    return LIBDBO_ERROR_UNKNOWN;
                }
            }
            sqlp += ret;
            left -= ret;
        }

        if ((ret = snprintf(sqlp, left, " )")) >= left) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        sqlp += ret;
        left -= ret;
    }

    /*
     * Prepare the SQL, create a SQLite statement.
     */
    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Bind all the values from value_set.
     */
    bind = 1;
    for (value_pos = 0; value_pos < libdbo_value_set_size(value_set); value_pos++) {
        if (!(value = libdbo_value_set_at(value_set, value_pos))) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }

        switch (libdbo_value_type(value)) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(value, &int32)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int = int32;
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(value, &uint32)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int = uint32;
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(value, &int64)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int64 = int64;
            ret = sqlite3_bind_int64(statement, bind++, to_int64);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(value, &uint64)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int64 = uint64;
            ret = sqlite3_bind_int64(statement, bind++, to_int64);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            ret = sqlite3_bind_text(statement, bind++, libdbo_value_text(value), -1, SQLITE_TRANSIENT);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_ENUM:
            if (libdbo_value_enum_value(value, &to_int)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Bind the revision field value if we have one.
     */
    if (revision_field) {
        ret = sqlite3_bind_int(statement, bind++, 1);
        if (ret != SQLITE_OK) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Execute the SQL.
     */
    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    return LIBDBO_OK;
}

static libdbo_result_list_t* libdbo_backend_sqlite_read(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_join_t* join;
    char sql[4*1024];
    char* sqlp;
    int ret, left, first, fields, bind;
    libdbo_result_list_t* result_list;
    libdbo_backend_sqlite_statement_t* statement;

    if (!__sqlite3_initialized) {
        return NULL;
    }
    if (!backend_sqlite) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "SELECT")) >= left) {
        return NULL;
    }
    sqlp += ret;
    left -= ret;

    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    first = 1;
    fields = 0;
    while (object_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s.%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                return NULL;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s.%s", libdbo_object_table(object), libdbo_object_field_name(object_field))) >= left) {
                return NULL;
            }
        }
        sqlp += ret;
        left -= ret;

        object_field = libdbo_object_field_next(object_field);
        fields++;
    }

    if ((ret = snprintf(sqlp, left, " FROM %s", libdbo_object_table(object))) >= left) {
        return NULL;
    }
    sqlp += ret;
    left -= ret;

    if (join_list) {
        join = libdbo_join_list_begin(join_list);
        while (join) {
            if ((ret = snprintf(sqlp, left, " INNER JOIN %s ON %s.%s = %s.%s",
                libdbo_join_to_table(join),
                libdbo_join_to_table(join),
                libdbo_join_to_field(join),
                libdbo_join_from_table(join),
                libdbo_join_from_field(join))) >= left)
            {
                return NULL;
            }
            sqlp += ret;
            left -= ret;
            join = libdbo_join_next(join);
        }
    }

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return NULL;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_sqlite_build_clause(object, clause_list, &sqlp, &left)) {
            return NULL;
        }
    }

    statement = libdbo_mm_new0(&__sqlite_statement_alloc);
    if (!statement) {
        return NULL;
    }
    statement->backend_sqlite = backend_sqlite;
    statement->object = object;
    statement->fields = fields;
    statement->statement = NULL;

    if (__db_backend_sqlite_prepare(backend_sqlite, &(statement->statement), sql, sizeof(sql))) {
        libdbo_mm_delete(&__sqlite_statement_alloc, statement);
        return NULL;
    }

    if (clause_list) {
        bind = 1;
        if (__db_backend_sqlite_bind_clause(statement->statement, clause_list, &bind)) {
            __db_backend_sqlite_finalize(statement->statement);
            libdbo_mm_delete(&__sqlite_statement_alloc, statement);
            return NULL;
        }
    }

    if (!(result_list = libdbo_result_list_new())
        || libdbo_result_list_set_next(result_list, libdbo_backend_sqlite_next, statement, 0))
    {
        libdbo_result_list_free(result_list);
        __db_backend_sqlite_finalize(statement->statement);
        libdbo_mm_delete(&__sqlite_statement_alloc, statement);
        return NULL;
    }
    return result_list;
}

static int libdbo_backend_sqlite_update(void* data, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    const libdbo_object_field_t* object_field;
    const libdbo_object_field_t* revision_field = NULL;
    const libdbo_clause_t* clause;
    const libdbo_clause_t* revision_clause = NULL;
    sqlite3_int64 revision_number = -1;
    const libdbo_value_t* value;
    char sql[4*1024];
    char* sqlp;
    int ret, left, bind, first;
    sqlite3_stmt* statement = NULL;
    size_t value_pos;
    int to_int;
    sqlite3_int64 to_int64;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }
    if (revision_field) {
        /*
         * If we have a revision field we should also have it in the clause,
         * find it and get the value for later use or return error if not found.
         */
        clause = libdbo_clause_list_begin(clause_list);
        while (clause) {
            if (!strcmp(libdbo_clause_field(clause), libdbo_object_field_name(revision_field))) {
                revision_clause = clause;
                break;
            }
            clause = libdbo_clause_next(clause);
        }
        if (!revision_clause) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        switch (libdbo_value_type(libdbo_clause_value(revision_clause))) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(libdbo_clause_value(revision_clause), &int32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = int32;
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(libdbo_clause_value(revision_clause), &uint32)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = uint32;
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(libdbo_clause_value(revision_clause), &int64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = int64;
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(libdbo_clause_value(revision_clause), &uint64)) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            revision_number = uint64;
            break;

        default:
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "UPDATE %s SET", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    /*
     * Build the update SQL from the object_field_list.
     */
    object_field = libdbo_object_field_list_begin(object_field_list);
    first = 1;
    while (object_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s = ?", libdbo_object_field_name(object_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s = ?", libdbo_object_field_name(object_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        sqlp += ret;
        left -= ret;

        object_field = libdbo_object_field_next(object_field);
    }

    /*
     * Add a new revision if we have any.
     */
    if (revision_field) {
        if (first) {
            if ((ret = snprintf(sqlp, left, " %s = ?", libdbo_object_field_name(revision_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            first = 0;
        }
        else {
            if ((ret = snprintf(sqlp, left, ", %s = ?", libdbo_object_field_name(revision_field))) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        sqlp += ret;
        left -= ret;
    }

    /*
     * Build the clauses.
     */
    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_sqlite_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Prepare the SQL.
     */
    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Bind all the values from value_set.
     */
    bind = 1;
    for (value_pos = 0; value_pos < libdbo_value_set_size(value_set); value_pos++) {
        if (!(value = libdbo_value_set_at(value_set, value_pos))) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }

        switch (libdbo_value_type(value)) {
        case LIBDBO_TYPE_INT32:
            if (libdbo_value_to_int32(value, &int32)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int = int32;
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT32:
            if (libdbo_value_to_uint32(value, &uint32)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int = uint32;
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_INT64:
            if (libdbo_value_to_int64(value, &int64)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int64 = int64;
            ret = sqlite3_bind_int64(statement, bind++, to_int64);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_UINT64:
            if (libdbo_value_to_uint64(value, &uint64)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            to_int64 = uint64;
            ret = sqlite3_bind_int64(statement, bind++, to_int64);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_TEXT:
            ret = sqlite3_bind_text(statement, bind++, libdbo_value_text(value), -1, SQLITE_TRANSIENT);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        case LIBDBO_TYPE_ENUM:
            if (libdbo_value_enum_value(value, &to_int)) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            ret = sqlite3_bind_int(statement, bind++, to_int);
            if (ret != SQLITE_OK) {
                __db_backend_sqlite_finalize(statement);
                return LIBDBO_ERROR_UNKNOWN;
            }
            break;

        default:
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Bind the new revision if we have any.
     */
    if (revision_field) {
        ret = sqlite3_bind_int64(statement, bind++, revision_number + 1);
        if (ret != SQLITE_OK) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Bind the clauses values.
     */
    if (clause_list) {
        if (__db_backend_sqlite_bind_clause(statement, clause_list, &bind)) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    /*
     * Execute the SQL.
     */
    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    /*
     * If we are using revision we have to have a positive number of changes
     * otherwise its a failure.
     */
    if (revision_field) {
        if (sqlite3_changes(backend_sqlite->db) < 1) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_delete(void* data, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    char sql[4*1024];
    char* sqlp;
    int ret, left, bind;
    sqlite3_stmt* statement = NULL;
    const libdbo_object_field_t* revision_field = NULL;
    const libdbo_object_field_t* object_field;
    const libdbo_clause_t* clause;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    /*
     * Check if the object has a revision field and keep it for later use.
     */
    object_field = libdbo_object_field_list_begin(libdbo_object_object_field_list(object));
    while (object_field) {
        if (libdbo_object_field_type(object_field) == LIBDBO_TYPE_REVISION) {
            if (revision_field) {
                /*
                 * We do not support multiple revision fields.
                 */
                return LIBDBO_ERROR_UNKNOWN;
            }

            revision_field = object_field;
        }
        object_field = libdbo_object_field_next(object_field);
    }
    if (revision_field) {
        /*
         * If we have a revision field we should also have it in the clause,
         * find it or return error if not found.
         */
        clause = libdbo_clause_list_begin(clause_list);
        while (clause) {
            if (!strcmp(libdbo_clause_field(clause), libdbo_object_field_name(revision_field))) {
                break;
            }
            clause = libdbo_clause_next(clause);
        }
        if (!clause) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "DELETE FROM %s", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_sqlite_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (clause_list) {
        bind = 1;
        if (__db_backend_sqlite_bind_clause(statement, clause_list, &bind)) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    /*
     * If we are using revision we have to have a positive number of changes
     * otherwise its a failure.
     */
    if (revision_field) {
        if (sqlite3_changes(backend_sqlite->db) < 1) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_count(void* data, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    const libdbo_join_t* join;
    char sql[4*1024];
    char* sqlp;
    int ret, left, bind;
    sqlite3_stmt* statement = NULL;
    int sqlite_count;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!count) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    left = sizeof(sql);
    sqlp = sql;

    if ((ret = snprintf(sqlp, left, "SELECT COUNT(*)")) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if ((ret = snprintf(sqlp, left, " FROM %s", libdbo_object_table(object))) >= left) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    sqlp += ret;
    left -= ret;

    if (join_list) {
        join = libdbo_join_list_begin(join_list);
        while (join) {
            if ((ret = snprintf(sqlp, left, " INNER JOIN %s ON %s.%s = %s.%s",
                libdbo_join_to_table(join),
                libdbo_join_to_table(join),
                libdbo_join_to_field(join),
                libdbo_join_from_table(join),
                libdbo_join_from_field(join))) >= left)
            {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
            join = libdbo_join_next(join);
        }
    }

    if (clause_list) {
        if (libdbo_clause_list_begin(clause_list)) {
            if ((ret = snprintf(sqlp, left, " WHERE")) >= left) {
                return LIBDBO_ERROR_UNKNOWN;
            }
            sqlp += ret;
            left -= ret;
        }
        if (__db_backend_sqlite_build_clause(object, clause_list, &sqlp, &left)) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (clause_list) {
        bind = 1;
        if (__db_backend_sqlite_bind_clause(statement, clause_list, &bind)) {
            __db_backend_sqlite_finalize(statement);
            return LIBDBO_ERROR_UNKNOWN;
        }
    }

    ret = __db_backend_sqlite_step(backend_sqlite, statement);
    if (ret != SQLITE_DONE && ret != SQLITE_ROW) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    sqlite_count = sqlite3_column_int(statement, 0);
    ret = sqlite3_errcode(backend_sqlite->db);
    if ((ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE)) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }

    *count = sqlite_count;
    __db_backend_sqlite_finalize(statement);
    return LIBDBO_OK;
}

static void libdbo_backend_sqlite_free(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;

    if (backend_sqlite) {
        if (backend_sqlite->db) {
            (void)libdbo_backend_sqlite_disconnect(backend_sqlite);
        }
        libdbo_mm_delete(&__sqlite_alloc, backend_sqlite);
    }
}

static int libdbo_backend_sqlite_transaction_begin(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    static const char* sql = "BEGIN TRANSACTION";
    sqlite3_stmt* statement = NULL;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (backend_sqlite->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    backend_sqlite->transaction = 1;
    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_transaction_commit(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    static const char* sql = "COMMIT TRANSACTION";
    sqlite3_stmt* statement = NULL;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    backend_sqlite->transaction = 0;
    return LIBDBO_OK;
}

static int libdbo_backend_sqlite_transaction_rollback(void* data) {
    libdbo_backend_sqlite_t* backend_sqlite = (libdbo_backend_sqlite_t*)data;
    static const char* sql = "ROLLBACK TRANSACTION";
    sqlite3_stmt* statement = NULL;

    if (!__sqlite3_initialized) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_sqlite->transaction) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_prepare(backend_sqlite, &statement, sql, sizeof(sql))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_backend_sqlite_step(backend_sqlite, statement) != SQLITE_DONE) {
        __db_backend_sqlite_finalize(statement);
        return LIBDBO_ERROR_UNKNOWN;
    }
    __db_backend_sqlite_finalize(statement);

    backend_sqlite->transaction = 0;
    return LIBDBO_OK;
}

libdbo_backend_handle_t* libdbo_backend_sqlite_new_handle(void) {
    libdbo_backend_handle_t* backend_handle = NULL;
    libdbo_backend_sqlite_t* backend_sqlite =
        (libdbo_backend_sqlite_t*)libdbo_mm_new0(&__sqlite_alloc);

    if (backend_sqlite && (backend_handle = libdbo_backend_handle_new())) {
        if (libdbo_backend_handle_set_data(backend_handle, (void*)backend_sqlite)
            || libdbo_backend_handle_set_initialize(backend_handle, libdbo_backend_sqlite_initialize)
            || libdbo_backend_handle_set_shutdown(backend_handle, libdbo_backend_sqlite_shutdown)
            || libdbo_backend_handle_set_connect(backend_handle, libdbo_backend_sqlite_connect)
            || libdbo_backend_handle_set_disconnect(backend_handle, libdbo_backend_sqlite_disconnect)
            || libdbo_backend_handle_set_create(backend_handle, libdbo_backend_sqlite_create)
            || libdbo_backend_handle_set_read(backend_handle, libdbo_backend_sqlite_read)
            || libdbo_backend_handle_set_update(backend_handle, libdbo_backend_sqlite_update)
            || libdbo_backend_handle_set_delete(backend_handle, libdbo_backend_sqlite_delete)
            || libdbo_backend_handle_set_count(backend_handle, libdbo_backend_sqlite_count)
            || libdbo_backend_handle_set_free(backend_handle, libdbo_backend_sqlite_free)
            || libdbo_backend_handle_set_transaction_begin(backend_handle, libdbo_backend_sqlite_transaction_begin)
            || libdbo_backend_handle_set_transaction_commit(backend_handle, libdbo_backend_sqlite_transaction_commit)
            || libdbo_backend_handle_set_transaction_rollback(backend_handle, libdbo_backend_sqlite_transaction_rollback))
        {
            libdbo_backend_handle_free(backend_handle);
            libdbo_mm_delete(&__sqlite_alloc, backend_sqlite);
            return NULL;
        }
    }
    return backend_handle;
}
