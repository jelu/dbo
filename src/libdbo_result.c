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
 * Based on enforcer-ng/src/db/db_result.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/result.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

/* DB RESULT */

static libdbo_mm_t __result_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_result_t));

libdbo_result_t* libdbo_result_new(void) {
    libdbo_result_t* result =
        (libdbo_result_t*)libdbo_mm_new0(&__result_alloc);

    return result;
}

libdbo_result_t* libdbo_result_new_copy(const libdbo_result_t* from_result) {
    libdbo_result_t* result;

    if (!from_result) {
        return NULL;
    }

    if ((result = libdbo_result_new())) {
        if (libdbo_result_copy(result, from_result)) {
            libdbo_result_free(result);
            return NULL;
        }
    }

    return result;
}

void libdbo_result_free(libdbo_result_t* result) {
    if (result) {
        if (result->value_set) {
            libdbo_value_set_free(result->value_set);
        }
        if (result->backend_meta_data_list) {
            libdbo_backend_meta_data_list_free(result->backend_meta_data_list);
        }
        libdbo_mm_delete(&__result_alloc, result);
    }
}

int libdbo_result_copy(libdbo_result_t* result, const libdbo_result_t* from_result) {
    libdbo_value_set_t* value_set = NULL;
    libdbo_backend_meta_data_list_t* backend_meta_data_list = NULL;

    if (!result) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!from_result) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (from_result->value_set
        && !(value_set = libdbo_value_set_new_copy(from_result->value_set)))
    {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (from_result->backend_meta_data_list
        && !(backend_meta_data_list = libdbo_backend_meta_data_list_new_copy(from_result->backend_meta_data_list)))
    {
        libdbo_value_set_free(value_set);
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (result->value_set) {
        libdbo_value_set_free(result->value_set);
    }
    result->value_set = value_set;
    if (result->backend_meta_data_list) {
        libdbo_backend_meta_data_list_free(result->backend_meta_data_list);
    }
    result->backend_meta_data_list = backend_meta_data_list;

    return LIBDBO_OK;
}

const libdbo_value_set_t* libdbo_result_value_set(const libdbo_result_t* result) {
    if (!result) {
        return NULL;
    }

    return result->value_set;
}

const libdbo_backend_meta_data_list_t* libdbo_result_backend_meta_data_list(const libdbo_result_t* result) {
    if (!result) {
        return NULL;
    }

    return result->backend_meta_data_list;
}

int libdbo_result_set_value_set(libdbo_result_t* result, libdbo_value_set_t* value_set) {
    if (!result) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result->value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    result->value_set = value_set;
    return LIBDBO_OK;
}

int libdbo_result_set_backend_meta_data_list(libdbo_result_t* result, libdbo_backend_meta_data_list_t* backend_meta_data_list) {
    if (!result) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!backend_meta_data_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result->backend_meta_data_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    result->backend_meta_data_list = backend_meta_data_list;
    return LIBDBO_OK;
}

int libdbo_result_not_empty(const libdbo_result_t* result) {
    if (!result) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!result->value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    return LIBDBO_OK;
}

/* DB RESULT LIST */

static libdbo_mm_t __result_list_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_result_list_t));

libdbo_result_list_t* libdbo_result_list_new(void) {
    libdbo_result_list_t* result_list =
        (libdbo_result_list_t*)libdbo_mm_new0(&__result_list_alloc);

    return result_list;
}

libdbo_result_list_t* libdbo_result_list_new_copy(const libdbo_result_list_t* from_result_list) {
    libdbo_result_list_t* result_list;

    if (!from_result_list) {
        return NULL;
    }

    result_list = (libdbo_result_list_t*)libdbo_mm_new0(&__result_list_alloc);
    if (result_list) {
        if (libdbo_result_list_copy(result_list, from_result_list)) {
            libdbo_result_list_free(result_list);
            return NULL;
        }
    }

    return result_list;
}

void libdbo_result_list_free(libdbo_result_list_t* result_list) {
    if (result_list) {
        if (result_list->begin) {
            libdbo_result_t* this = result_list->begin;
            libdbo_result_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_result_free(this);
                this = next;
            }
        }
        if (result_list->next_function) {
            (void)result_list->next_function(result_list->next_data, 1);
            if (result_list->current) {
                libdbo_result_free(result_list->current);
            }
        }
        libdbo_mm_delete(&__result_list_alloc, result_list);
    }
}

int libdbo_result_list_copy(libdbo_result_list_t* result_list, const libdbo_result_list_t* from_result_list) {
    libdbo_result_t* result;
    libdbo_result_t* result_copy;

    if (!result_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    /*
     * TODO: Should we be able to copy into a result list that already contains
     * data?
     */
    if (result_list->begin) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->end) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->current) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->size) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->next_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!from_result_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (from_result_list->next_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    result = from_result_list->begin;
    while (result) {
        if (!(result_copy = libdbo_result_new_copy(result))
            || libdbo_result_list_add(result_list, result_copy))
        {
            return LIBDBO_ERROR_UNKNOWN;
        }

        if (result == from_result_list->current) {
            result_list->current = result_copy;
        }

        result = result->next;
    }

    return LIBDBO_OK;
}

int libdbo_result_list_set_next(libdbo_result_list_t* result_list, libdbo_result_list_next_t next_function, void* next_data, size_t size) {
    if (!result_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->begin) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->next_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!next_data) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->next_data) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    result_list->next_function = next_function;
    result_list->next_data = next_data;
    result_list->size = size;
    return 0;
}

int libdbo_result_list_add(libdbo_result_list_t* result_list, libdbo_result_t* result) {
    if (!result_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!result) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (libdbo_result_not_empty(result)) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result->next) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (result_list->next_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (result_list->begin) {
        if (!result_list->end) {
            return LIBDBO_ERROR_UNKNOWN;
        }
        result_list->end->next = result;
        result_list->end = result;
    }
    else {
        result_list->begin = result;
        result_list->end = result;
    }
    result_list->size++;

    return LIBDBO_OK;
}

const libdbo_result_t* libdbo_result_list_begin(libdbo_result_list_t* result_list) {
    if (!result_list) {
        return NULL;
    }
    if (result_list->next_function) {
        /*
         * Can not start over a list that uses next function
         */
        if (result_list->current) {
            return NULL;
        }
        result_list->current = result_list->next_function(result_list->next_data, 0);
        return result_list->current;
    }

    result_list->current = result_list->begin;
    result_list->begun = 1;
    return result_list->current;
}

const libdbo_result_t* libdbo_result_list_next(libdbo_result_list_t* result_list) {
    if (!result_list) {
        return NULL;
    }

    if (result_list->next_function) {
        if (result_list->current) {
            libdbo_result_free(result_list->current);
        }
        result_list->current = result_list->next_function(result_list->next_data, 0);
        return result_list->current;
    }

    if (!result_list->begun) {
        result_list->begun = 1;
        result_list->current = result_list->begin;
    }
    else if (result_list->current) {
        result_list->current = result_list->current->next;
    }
    return result_list->current;
}

size_t libdbo_result_list_size(const libdbo_result_list_t* result_list) {
    if (!result_list) {
        return 0;
    }

    return result_list->size;
}

int libdbo_result_list_fetch_all(libdbo_result_list_t* result_list) {
    libdbo_result_t* result;
    libdbo_result_list_next_t next_function;

    if (!result_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (result_list->next_function) {
        if (result_list->current) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        next_function = result_list->next_function;
        result_list->next_function = NULL;
        result_list->size = 0;

        while ((result = next_function(result_list->next_data, 0))) {
            if (libdbo_result_list_add(result_list, result)) {
                next_function(result_list->next_data, 1);
                result_list->next_data = NULL;
                libdbo_result_free(result);
                return LIBDBO_ERROR_UNKNOWN;
            }
        }
        next_function(result_list->next_data, 1);
        result_list->next_data = NULL;
    }

    return LIBDBO_OK;
}
