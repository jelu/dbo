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

#include "db_mm.h"

#include <strings.h>
#include <unistd.h>

/* TODO: keep list of blocks, add freeing functionality */

/* TODO: use page size * (something or option in struct) */
#define __db_mm_size 65536

static size_t __pagesize = __db_mm_size;

void db_mm_init(void) {
    /* TODO: will long => size_t be a problem somewhere? */
    /* TODO: This isn't working
#if defined(_SC_PAGESIZE)
    __pagesize = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    __pagesize = sysconf(_SC_PAGE_SIZE);
#endif
*/
}

void* db_mm_new(db_mm_t* alloc) {
    void* ptr;

    if (!alloc) {
        return NULL;
    }
    if (alloc->size < 1) {
        return NULL;
    }
    if (__pagesize < alloc->size) {
        return NULL;
    }
    if (pthread_mutex_lock(&(alloc->lock))) {
        return NULL;
    }

    if (!alloc->next) {
        unsigned int i;
        void* block;

        if (!(block = malloc(__pagesize))) {
            pthread_mutex_unlock(&(alloc->lock));
            return NULL;
        }

        for (i=0; i<(__db_mm_size / alloc->size); i++) {
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

void* db_mm_new0(db_mm_t* alloc) {
    void* ptr = db_mm_new(alloc);

    if (ptr) {
        bzero(ptr, alloc->size);
    }

    return ptr;
}

void db_mm_delete(db_mm_t* alloc, void* ptr) {
    if (!alloc) {
        return;
    }
    if (!ptr) {
        return;
    }
    if (pthread_mutex_lock(&(alloc->lock))) {
        return;
    }

    *(void**)ptr = alloc->next;
    alloc->next = ptr;

    pthread_mutex_unlock(&(alloc->lock));
}
