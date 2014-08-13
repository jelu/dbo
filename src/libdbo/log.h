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

/** \file libdbo/log.h */
/** \defgroup libdbo_log libdbo_log
 * Database Log Handling.
 * These are the functions to manage the log messages that the database layer
 * can generate.
 */

#ifndef libdbo_log_h
#define libdbo_log_h

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup libdbo_log */
/** \{ */

/**
 * The log level for the log messages.
 */
typedef enum {
    LIBDBO_LOG_DEBUG,
    LIBDBO_LOG_INFO,
    LIBDBO_LOG_NOTICE,
    LIBDBO_LOG_WARNING,
    LIBDBO_LOG_ERROR,
    LIBDBO_LOG_CRITICAL,
    LIBDBO_LOG_ALERT,
    LIBDBO_LOG_FATAL
} libdbo_log_priority_t;

/**
 * Function pointer to the log handler.
 * \param[in] priority a libdbo_log_priority_t with the priority of the message.
 * \param[in] format a format string for the message.
 * \param[in] ap a va_list with the arguments to the format string.
 */
typedef void (*libdbo_log_handler_t)(libdbo_log_priority_t priority, const char* format, va_list ap);

/**
 * Log a message via the database layers log handler.
 * \param[in] priority a libdbo_log_priority_t with the priority of the message.
 * \param[in] format a format string for the message.
 * \param[in] ... arguments to the format string.
 */
void libdbo_log(libdbo_log_priority_t priority, const char* format, ...);

/**
 * Set the log hander for the database layer.
 * \param[in] log_handler a libdbo_log_handler_t function pointer to the new log
 * handler for the database layer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_log_set_handler(libdbo_log_handler_t log_handler);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define DB_LOG_DEBUG LIBDBO_LOG_DEBUG
#define DB_LOG_INFO LIBDBO_LOG_INFO
#define DB_LOG_NOTICE LIBDBO_LOG_NOTICE
#define DB_LOG_WARNING LIBDBO_LOG_WARNING
#define DB_LOG_ERROR LIBDBO_LOG_ERROR
#define DB_LOG_CRITICAL LIBDBO_LOG_CRITICAL
#define DB_LOG_ALERT LIBDBO_LOG_ALERT
#define DB_LOG_FATAL LIBDBO_LOG_FATAL
#define db_log_handler_t libdbo_log_handler_t
#define db_log(...) libdbo_log(__VA_ARGS__)
#define db_log_set_handler(...) libdbo_log_set_handler(__VA_ARGS__)
#endif
#endif

#endif
