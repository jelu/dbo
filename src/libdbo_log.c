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

#include "libdbo/log.h"

#include "libdbo/error.h"

#include <stdio.h>

static void __default_log_handler(libdbo_log_priority_t priority, const char* format, va_list ap) {
    switch (priority) {
    case LIBDBO_LOG_DEBUG:
        printf("DEBUG: ");
        break;

    case LIBDBO_LOG_INFO:
        printf("INFO: ");
        break;

    case LIBDBO_LOG_NOTICE:
        printf("NOTICE: ");
        break;

    case LIBDBO_LOG_WARNING:
        printf("WARNING: ");
        break;

    case LIBDBO_LOG_ERROR:
        printf("ERROR: ");
        break;

    case LIBDBO_LOG_CRITICAL:
        printf("CRITICAL: ");
        break;

    case LIBDBO_LOG_ALERT:
        printf("ALERT: ");
        break;

    case LIBDBO_LOG_FATAL:
        printf("FATAL: ");
        break;

    default:
        return;
    }

    vprintf(format, ap);
    printf("\n");
}

static libdbo_log_handler_t __log_handler = &__default_log_handler;

void libdbo_log(libdbo_log_priority_t priority, const char* format, ...) {
    va_list ap;

    va_start(ap, format);
    __log_handler(priority, format, ap);
    va_end(ap);
}

int libdbo_log_set_handler(libdbo_log_handler_t log_handler) {
    if (!log_handler) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    __log_handler = log_handler;

    return LIBDBO_OK;
}
