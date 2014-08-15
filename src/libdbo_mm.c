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
 * Based on mm/src/mm.c source file from the OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/mm.h"

#include "libdbo/error.h"

#include <strings.h>
#include <unistd.h>

/* TODO: keep list of blocks, add freeing functionality */

static size_t __db_mm_pagesize = 0;
static libdbo_mm_malloc_t __db_mm_malloc = NULL;
static libdbo_mm_free_t __db_mm_free = NULL;

void libdbo_mm_init(void) {
    /* TODO: will long => size_t be a problem somewhere? */
}

int libdbo_mm_set_malloc(libdbo_mm_malloc_t malloc_function) {
    if (!malloc_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_mm_malloc) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    __db_mm_malloc = malloc_function;

    return LIBDBO_OK;
}

int libdbo_mm_set_free(libdbo_mm_free_t free_function) {
    if (!free_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__db_mm_free) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    __db_mm_free = free_function;

    return LIBDBO_OK;
}

void* libdbo_mm_new(libdbo_mm_t* alloc) {
    void* ptr;

    if (!alloc) {
        return NULL;
    }

    if (__db_mm_malloc && __db_mm_free) {
        return __db_mm_malloc(alloc->size);
    }

    if (pthread_mutex_lock(&(alloc->lock))) {
        return NULL;
    }

    if (!alloc->next) {
        unsigned int i;
        void* block;

        if (alloc->size < sizeof(void*)) {
            alloc->size = sizeof(void*);
        }
        if (!alloc->block_size) {
            /*
             * TODO: Add more logic here, if we have very large objects we should
             * dynamically increase the page size.
             */
            alloc->block_size = ((alloc->size / libdbo_mm_pagesize()) + 1) * libdbo_mm_pagesize();
            if ((alloc->block_size - alloc->size) < sizeof(void*)) {
                alloc->block_size += libdbo_mm_pagesize();
            }
            if (alloc->block_size < (alloc->size + sizeof(void*))) {
                alloc->block_size = 0;
                pthread_mutex_unlock(&(alloc->lock));
                return NULL;
            }
        }

        if (!(block = malloc(alloc->block_size))) {
            pthread_mutex_unlock(&(alloc->lock));
            return NULL;
        }

        *(void**)block = alloc->block;
        alloc->block = block;
        block = block++;

        for (i=0; i<((alloc->block_size - sizeof(void*)) / alloc->size); i++) {
            *(void**)block = alloc->next;
            alloc->next = block;
            block = ((char*)block + alloc->size);
        }
    }

    ptr = alloc->next;
    alloc->next = *(void**)ptr;
    *(void**)ptr = NULL;

    pthread_mutex_unlock(&(alloc->lock));
    return ptr;
}

void* libdbo_mm_new0(libdbo_mm_t* alloc) {
    void* ptr = libdbo_mm_new(alloc);

    if (ptr) {
        bzero(ptr, alloc->size);
    }

    return ptr;
}

void libdbo_mm_delete(libdbo_mm_t* alloc, void* ptr) {
    if (!alloc) {
        return;
    }
    if (!ptr) {
        return;
    }

    if (__db_mm_malloc && __db_mm_free) {
        return __db_mm_free(ptr);
    }

    if (pthread_mutex_lock(&(alloc->lock))) {
        return;
    }

    *(void**)ptr = alloc->next;
    alloc->next = ptr;

    pthread_mutex_unlock(&(alloc->lock));
}

void libdbo_mm_release(libdbo_mm_t* alloc) {
    void* block;

    if (!alloc) {
        return;
    }

    if (pthread_mutex_lock(&(alloc->lock))) {
        return;
    }

    while (alloc->block) {
        block = alloc->block;
        alloc->block = *(void**)block;
        free(block);
    }

    pthread_mutex_unlock(&(alloc->lock));
}

size_t libdbo_mm_pagesize(void) {
    if (!__db_mm_pagesize) {
        long pagesize;

        if ((pagesize = sysconf(_SC_PAGESIZE)) > 0) {
            __db_mm_pagesize = (size_t)pagesize;
        }
        else {
            __db_mm_pagesize = LIBDBO_MM_DEFAULT_PAGESIZE;
        }
    }

    return __db_mm_pagesize;
}
