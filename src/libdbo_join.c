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
 * Based on enforcer-ng/src/db/db_join.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/join.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <stdlib.h>
#include <string.h>

/* DB JOIN */

static libdbo_mm_t __join_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_join_t));

libdbo_join_t* libdbo_join_new(void) {
    libdbo_join_t* join =
        (libdbo_join_t*)libdbo_mm_new0(&__join_alloc);

    return join;
}

void libdbo_join_free(libdbo_join_t* join) {
    if (join) {
        if (join->from_table) {
            free(join->from_table);
        }
        if (join->from_field) {
            free(join->from_field);
        }
        if (join->to_table) {
            free(join->to_table);
        }
        if (join->to_field) {
            free(join->to_field);
        }
        libdbo_mm_delete(&__join_alloc, join);
    }
}

const char* libdbo_join_from_table(const libdbo_join_t* join) {
    if (!join) {
        return NULL;
    }

    return join->from_table;
}

const char* libdbo_join_from_field(const libdbo_join_t* join) {
    if (!join) {
        return NULL;
    }

    return join->from_field;
}

const char* libdbo_join_to_table(const libdbo_join_t* join) {
    if (!join) {
        return NULL;
    }

    return join->to_table;
}

const char* libdbo_join_to_field(const libdbo_join_t* join) {
    if (!join) {
        return NULL;
    }

    return join->to_field;
}

int libdbo_join_set_from_table(libdbo_join_t* join, const char* from_table) {
    char* new_from_table;

    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_from_table = strdup(from_table))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (join->from_table) {
        free(join->from_table);
    }
    join->from_table = new_from_table;
    return LIBDBO_OK;
}

int libdbo_join_set_from_field(libdbo_join_t* join, const char* from_field) {
    char* new_from_field;

    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_from_field = strdup(from_field))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (join->from_field) {
        free(join->from_field);
    }
    join->from_field = new_from_field;
    return LIBDBO_OK;
}

int libdbo_join_set_to_table(libdbo_join_t* join, const char* to_table) {
    char* new_to_table;

    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_to_table = strdup(to_table))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (join->to_table) {
        free(join->to_table);
    }
    join->to_table = new_to_table;
    return LIBDBO_OK;
}

int libdbo_join_set_to_field(libdbo_join_t* join, const char* to_field) {
    char* new_to_field;

    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!(new_to_field = strdup(to_field))) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (join->to_field) {
        free(join->to_field);
    }
    join->to_field = new_to_field;
    return LIBDBO_OK;
}

int libdbo_join_not_empty(const libdbo_join_t* join) {
    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!join->from_table) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!join->from_field) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!join->to_table) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!join->to_field) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    return LIBDBO_OK;
}

const libdbo_join_t* libdbo_join_next(const libdbo_join_t* join) {
    if (!join) {
        return NULL;
    }

    return join->next;
}

/* DB JOIN LIST */

static libdbo_mm_t __join_list_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_join_list_t));

libdbo_join_list_t* libdbo_join_list_new(void) {
    libdbo_join_list_t* join_list =
        (libdbo_join_list_t*)libdbo_mm_new0(&__join_list_alloc);

    return join_list;
}

void libdbo_join_list_free(libdbo_join_list_t* join_list) {
    if (join_list) {
        if (join_list->begin) {
            libdbo_join_t* this = join_list->begin;
            libdbo_join_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_join_free(this);
                this = next;
            }
        }
        libdbo_mm_delete(&__join_list_alloc, join_list);
    }
}

int libdbo_join_list_add(libdbo_join_list_t* join_list, libdbo_join_t* join) {
    if (!join_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!join) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (libdbo_join_not_empty(join)) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (join->next) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (join_list->begin) {
        if (!join_list->end) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        join_list->end->next = join;
        join_list->end = join;
    }
    else {
        join_list->begin = join;
        join_list->end = join;
    }

    return LIBDBO_OK;
}

const libdbo_join_t* libdbo_join_list_begin(const libdbo_join_list_t* join_list) {
    if (!join_list) {
        return NULL;
    }

    return join_list->begin;
}
