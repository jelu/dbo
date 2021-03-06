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
 * Based on enforcer-ng/src/db/db_configuration.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/configuration.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <stdlib.h>
#include <string.h>

/* DB CONFIGURATION */

static libdbo_mm_t __configuration_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_configuration_t));

libdbo_configuration_t* libdbo_configuration_new(void) {
    libdbo_configuration_t* configuration =
        (libdbo_configuration_t*)libdbo_mm_new0(&__configuration_alloc);

    return configuration;
}

void libdbo_configuration_free(libdbo_configuration_t* configuration) {
    if (configuration) {
        if (configuration->name) {
            free(configuration->name);
        }
        if (configuration->value) {
            free(configuration->value);
        }
        libdbo_mm_delete(&__configuration_alloc, configuration);
    }
}

const char* libdbo_configuration_name(const libdbo_configuration_t* configuration) {
    if (!configuration) {
        return NULL;
    }

    return configuration->name;
}

const char* libdbo_configuration_value(const libdbo_configuration_t* configuration) {
    if (!configuration) {
        return NULL;
    }

    return configuration->value;
}

int libdbo_configuration_set_name(libdbo_configuration_t* configuration, const char* name) {
    char* new_name;

    if (!configuration) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!name) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_name = strdup(name))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (configuration->name) {
        free(configuration->name);
    }
    configuration->name = new_name;
    return LIBDBO_OK;
}

int libdbo_configuration_set_value(libdbo_configuration_t* configuration, const char* value) {
    char* new_value;

    if (!configuration) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_value = strdup(value))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (configuration->value) {
        free(configuration->value);
    }
    configuration->value = new_value;
    return LIBDBO_OK;
}

int libdbo_configuration_not_empty(const libdbo_configuration_t* configuration) {
    if (!configuration) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration->name) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration->value) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    return LIBDBO_OK;
}

/* DB CONFIGURATION LIST */

static libdbo_mm_t __configuration_list_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_configuration_list_t));

libdbo_configuration_list_t* libdbo_configuration_list_new(void) {
    libdbo_configuration_list_t* configuration_list =
        (libdbo_configuration_list_t*)libdbo_mm_new0(&__configuration_list_alloc);

    return configuration_list;
}

void libdbo_configuration_list_free(libdbo_configuration_list_t* configuration_list) {
    if (configuration_list) {
        if (configuration_list->begin) {
            libdbo_configuration_t* this = configuration_list->begin;
            libdbo_configuration_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_configuration_free(this);
                this = next;
            }
        }
        libdbo_mm_delete(&__configuration_list_alloc, configuration_list);
    }
}

int libdbo_configuration_list_add(libdbo_configuration_list_t* configuration_list, libdbo_configuration_t* configuration) {
    if (!configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!configuration) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (libdbo_configuration_not_empty(configuration)) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (configuration->next) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (configuration_list->begin) {
        if (!configuration_list->end) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        configuration_list->end->next = configuration;
        configuration_list->end = configuration;
    }
    else {
        configuration_list->begin = configuration;
        configuration_list->end = configuration;
    }

    return LIBDBO_OK;
}

const libdbo_configuration_t* libdbo_configuration_list_find(const libdbo_configuration_list_t* configuration_list, const char* name) {
    libdbo_configuration_t* configuration;

    if (!configuration_list) {
        return NULL;
    }
    if (!name) {
        return NULL;
    }

    configuration = configuration_list->begin;
    while (configuration) {
        if (libdbo_configuration_not_empty(configuration)) {
            return NULL;
        }
        if (!strcmp(configuration->name, name)) {
            break;
        }
        configuration = configuration->next;
    }

    return configuration;
}
