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

#include "config.h"

#include "libdbo/mm.h"

#include "libdbo/error.h"

#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#if defined(USE_LIBDBO_MM_CHECKS)
#include <assert.h>
#endif

/* TODO: keep list of blocks, add freeing functionality */

static size_t __pagesize = 0;
#if defined(USE_LIBDBO_MM)
static libdbo_mm_malloc_t __malloc = NULL;
static libdbo_mm_free_t __free = NULL;
#else
static libdbo_mm_malloc_t __malloc = &malloc;
static libdbo_mm_free_t __free = &free;
#endif

void libdbo_mm_init(void) {
}

int libdbo_mm_set_malloc(libdbo_mm_malloc_t malloc_function) {
    if (!malloc_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__malloc) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    __malloc = malloc_function;

    return LIBDBO_OK;
}

int libdbo_mm_set_free(libdbo_mm_free_t free_function) {
    if (!free_function) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (__free) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    __free = free_function;

    return LIBDBO_OK;
}

void* libdbo_mm_new(libdbo_mm_t* alloc) {
    void* ptr;

    if (!alloc) {
        return NULL;
    }

    if (__malloc && __free) {
        return __malloc(alloc->size);
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
#if defined(USE_LIBDBO_MM_CHECKS)
        alloc->size += sizeof(void*);
#endif
        if (!alloc->block_size) {
            if (((libdbo_mm_pagesize() - sizeof(void*)) / alloc->size) < alloc->num_objects) {
                /*
                 * Calculate the block size so we have enough to allocate the
                 * minimum number of objects.
                 *
                 * We take the object size * minimum number of objects + a void
                 * pointer for the block chain. Divide it by the page size and
                 * add one to the result to get the number of pages needed for
                 * the objects then * that with the page size to get the number
                 * of bytes per block.
                 */
                alloc->block_size = ((((alloc->size * alloc->num_objects) + sizeof(void*)) / libdbo_mm_pagesize()) + 1)
                    * libdbo_mm_pagesize();
            }
            else {
                /*
                 * We can allocate enough objects within one page size so we set
                 * the block size to a page size.
                 */
                alloc->block_size = libdbo_mm_pagesize();
            }
        }

        if (!(block = malloc(alloc->block_size))) {
            pthread_mutex_unlock(&(alloc->lock));
            return NULL;
        }

        *(void**)block = alloc->block;
        alloc->block = block;
        block = (char*)block + sizeof(void*);

        for (i=0; i<((alloc->block_size - sizeof(void*)) / alloc->size); i++) {
#if defined(USE_LIBDBO_MM_CHECKS)
            *(void**)block = (void*)1L;
            *(void**)((char*)block + sizeof(void*)) = alloc->next;
#else
            *(void**)block = alloc->next;
#endif
            alloc->next = block;
            block = ((char*)block + alloc->size);
            alloc->total_allocs++;
        }
    }

    if ((ptr = alloc->next)) {
#if defined(USE_LIBDBO_MM_CHECKS)
        assert(*(void**)ptr == (void*)1L);
        *(void**)ptr = NULL;
        ptr = (char*)ptr + sizeof(void*);
#endif
        alloc->next = *(void**)ptr;
        *(void**)ptr = NULL;
        alloc->current_allocs++;
#if defined(USE_LIBDBO_MM_CHECKS)
        assert(alloc->current_allocs <= alloc->total_allocs);
#endif
    }

    pthread_mutex_unlock(&(alloc->lock));
    return ptr;
}

void* libdbo_mm_new0(libdbo_mm_t* alloc) {
    void* ptr = libdbo_mm_new(alloc);

    if (ptr) {
#if defined(USE_LIBDBO_MM_CHECKS)
        bzero(ptr, alloc->size - sizeof(void*));
#else
        bzero(ptr, alloc->size);
#endif
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

    if (__malloc && __free) {
        return __free(ptr);
    }

    if (pthread_mutex_lock(&(alloc->lock))) {
        return;
    }

    *(void**)ptr = alloc->next;
#if defined(USE_LIBDBO_MM_CHECKS)
    assert(alloc->current_allocs > 0);
    ptr = (char*)ptr - sizeof(void*);
    assert(*(void**)ptr == NULL);
    *(void**)ptr = (void*)1L;
#endif
    alloc->next = ptr;
    alloc->current_allocs--;

    /*
     * Automatically release all blocks if we have more then one block and there
     * are no current allocations.
     */
    if (!alloc->current_allocs
        && alloc->block
        && *(void**)(alloc->block))
    {
        void* block;

        while (alloc->block) {
            block = alloc->block;
            alloc->block = *(void**)block;
            free(block);
        }
        alloc->total_allocs = 0;
        alloc->current_allocs = 0;
        alloc->next = NULL;
    }

    pthread_mutex_unlock(&(alloc->lock));
}

void libdbo_mm_release(libdbo_mm_t* alloc) {
    void* block;

    if (!alloc) {
        return;
    }

    if (__malloc && __free) {
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
    alloc->total_allocs = 0;
    alloc->current_allocs = 0;
    alloc->next = NULL;

    pthread_mutex_unlock(&(alloc->lock));
}

size_t libdbo_mm_pagesize(void) {
    if (!__pagesize) {
        long pagesize;

        if ((pagesize = sysconf(_SC_PAGESIZE)) > 0) {
            /* TODO: will long => size_t be a problem somewhere? */
            __pagesize = (size_t)pagesize;
        }
        else {
            __pagesize = LIBDBO_MM_DEFAULT_PAGESIZE;
        }
    }

    return __pagesize;
}
