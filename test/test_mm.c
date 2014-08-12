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

#include <libdbo/mm.h>

#include "CUnit/Basic.h"

#include <stdlib.h>

int init_suite_mm(void) {
    return 0;
}

int clean_suite_mm(void) {
    return 0;
}

void test_libdbo_mm_init(void) {
    libdbo_mm_init();
}

void test_libdbo_mm(void) {
    libdbo_mm_t mm = LIBDBO_MM_T_STATIC_NEW(128);
    void* ptr;

    CU_ASSERT_PTR_NOT_NULL_FATAL((ptr = libdbo_mm_new0(&mm)));
    libdbo_mm_delete(&mm, ptr);
    CU_PASS("libdbo_mm_delete");

    libdbo_mm_release(&mm);
    CU_PASS("libdbo_mm_release");
}

void test_libdbo_mm_extern(void) {
    libdbo_mm_t mm = LIBDBO_MM_T_STATIC_NEW(128);
    void* ptr;

    CU_ASSERT_FATAL(!libdbo_mm_set_malloc(&malloc));
    CU_ASSERT_FATAL(!libdbo_mm_set_free(&free));
    CU_ASSERT_PTR_NOT_NULL_FATAL((ptr = libdbo_mm_new0(&mm)));
    libdbo_mm_delete(&mm, ptr);
    CU_PASS("libdbo_mm_delete");
}
