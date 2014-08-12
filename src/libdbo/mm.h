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
 * Based on mm/src/mm.h header file from the OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

/** \file libdbo/mm.h */
/** \defgroup libdbo_mm libdbo_mm
 * Database Memory Management.
 * These functions handles the internal memory of the database layer. It can be
 * replaced with external memory management via the functions libdbo_mm_set_malloc()
 * and libdbo_mm_set_free().
 */

#ifndef libdbo_mm_h
#define libdbo_mm_h

#include <stdlib.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup libdbo_mm */
/** \{ */

/**
 * A libdbo_mm_t static allocation for a memory pool.
 */
#define LIBDBO_MM_T_STATIC_NEW(object_size) { NULL, NULL, object_size, PTHREAD_MUTEX_INITIALIZER }

/**
 * A memory pool handle.
 */
typedef struct libdbo_mm libdbo_mm_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct libdbo_mm
{
    void *block;
    void *next;
    size_t size;
    pthread_mutex_t lock;
};
#endif

/**
 * Function pointer for allocating memory, used with libdbo_mm_set_malloc() and
 * libdbo_mm_set_free() to disable the database layer memory management.
 * \param[in] size a size_t with the size being allocated.
 * \return a pointer to the memory allocated or NULL on error.
 */
typedef void* (*libdbo_mm_malloc_t)(size_t size);

/**
 * Function pointer for freeing memory, used with libdbo_mm_set_malloc() and
 * libdbo_mm_set_free() to disable the database layer memory management.
 * \param[in] ptr a pointer to the memory being freed.
 */
typedef void (*libdbo_mm_free_t)(void* ptr);

/**
 * Initiate the database layer memory management.
 */
void libdbo_mm_init(void);

/**
 * Set a custom malloc function for the database layer. Both malloc and free
 * function pointers needs to be set in other for them to be called instead of
 * the database layer memory management code.
 * \param[in] malloc_function a libdbo_mm_malloc_t function pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_mm_set_malloc(libdbo_mm_malloc_t malloc_function);

/**
 * Set a custom free function for the database layer. Both malloc and free
 * function pointers needs to be set in other for them to be called instead of
 * the database layer memory management code.
 * \param[in] free_function a libdbo_mm_free_t function pointer.
 * \return LIBDBO_ERROR_* on failure, otherwise LIBDBO_OK.
 */
int libdbo_mm_set_free(libdbo_mm_free_t free_function);

/**
 * Allocate a new object from the managed pool.
 * \param[in] alloc a libdbo_mm_t pointer.
 * \return A pointer to the new object or NULL on error.
 */
void* libdbo_mm_new(libdbo_mm_t* alloc);

/**
 * Allocate a new object from the managed pool, this also zeros the memory
 * before returning it.
 * \param[in] alloc a libdbo_mm_t pointer.
 * \return A pointer to the new object or NULL on error.
 */
void* libdbo_mm_new0(libdbo_mm_t* alloc);

/**
 * Free an object in a managed pool, returning it to the pool and making it
 * available for others to allocate.
 * \param[in] alloc a libdbo_mm_t pointer.
 * \param[in] ptr a pointer to the object that is being freed.
 */
void libdbo_mm_delete(libdbo_mm_t* alloc, void* ptr);

/**
 * Release all free memory in a managed pool, all memory in the pool MUST have
 * been delete with libdbo_mm_delete() before calling this function. Any access to
 * the memory areas previusly given will result in segfaults and/or memory
 * corruption.
 * \param[in] alloc a libdbo_mm_t pointer.
 */
void libdbo_mm_release(libdbo_mm_t* alloc);

/** \} */

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef LIBDBO_SHORT_NAMES
#define DB_MM_T_STATIC_NEW(object_size) { NULL, NULL, object_size, PTHREAD_MUTEX_INITIALIZER }
#define db_mm_t libdbo_mm_t
#define db_mm_malloc_t libdbo_mm_malloc_t
#define db_mm_free_t libdbo_mm_free_t
#define db_mm_init(...) libdbo_mm_init(__VA_ARGS__)
#define db_mm_set_malloc(...) libdbo_mm_set_malloc(__VA_ARGS__)
#define db_mm_set_free(...) libdbo_mm_set_free(__VA_ARGS__)
#define db_mm_new(...) libdbo_mm_new(__VA_ARGS__)
#define db_mm_new0(...) libdbo_mm_new0(__VA_ARGS__)
#define db_mm_delete(...) libdbo_mm_delete(__VA_ARGS__)
#define db_mm_release(...) libdbo_mm_release(__VA_ARGS__)
#endif
#endif

#endif
