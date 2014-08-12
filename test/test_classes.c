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
 * Based on enforcer-ng/src/db/test/test_classes.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo_backend.h"
#include "libdbo_clause.h"
#include "libdbo_configuration.h"
#include "libdbo_connection.h"
#include "libdbo_join.h"
#include "libdbo_object.h"
#include "libdbo_result.h"
#include "libdbo_value.h"

#include "CUnit/Basic.h"

static int fake_pointer = 0;
static libdbo_backend_handle_t* backend_handle = NULL;
static libdbo_backend_t* backend = NULL;
static libdbo_backend_meta_data_t* backend_meta_data = NULL;
static libdbo_backend_meta_data_t* backend_meta_data2 = NULL;
static libdbo_backend_meta_data_t* backend_meta_data3 = NULL;
static libdbo_backend_meta_data_t* backend_meta_data4 = NULL;
static libdbo_backend_meta_data_list_t* backend_meta_data_list = NULL;
static libdbo_backend_meta_data_list_t* backend_meta_data_list2 = NULL;
static libdbo_clause_t* clause = NULL;
static libdbo_clause_t* clause2 = NULL;
static libdbo_clause_list_t* clause_list = NULL;
static libdbo_configuration_t* configuration = NULL;
static libdbo_configuration_t* configuration2 = NULL;
static libdbo_configuration_list_t* configuration_list = NULL;
static libdbo_connection_t* connection = NULL;
static libdbo_join_t* join = NULL;
static libdbo_join_t* join2 = NULL;
static libdbo_join_list_t* join_list = NULL;
static libdbo_object_field_t* object_field = NULL;
static libdbo_object_field_t* object_field2 = NULL;
static libdbo_object_field_list_t* object_field_list = NULL;
static libdbo_object_t* object = NULL;
static libdbo_value_set_t* value_set = NULL;
static libdbo_value_set_t* value_set2 = NULL;
static libdbo_result_t* result = NULL;
static libdbo_result_t* result2 = NULL;
static libdbo_result_list_t* result_list = NULL;
static libdbo_value_t* value = NULL;
static libdbo_value_t* value2 = NULL;
static const libdbo_enum_t enum_set[] = {
    { "enum1", 1 },
    { "enum2", 2 },
    { "enum3", 3 },
    { NULL, 0 }
};

int init_suite_classes(void) {
    if (backend_handle) {
        return 1;
    }
    if (backend) {
        return 1;
    }
    if (backend_meta_data) {
        return 1;
    }
    if (backend_meta_data2) {
        return 1;
    }
    if (backend_meta_data3) {
        return 1;
    }
    if (backend_meta_data4) {
        return 1;
    }
    if (backend_meta_data_list) {
        return 1;
    }
    if (backend_meta_data_list2) {
        return 1;
    }
    if (clause) {
        return 1;
    }
    if (clause2) {
        return 1;
    }
    if (clause_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (configuration2) {
        return 1;
    }
    if (configuration_list) {
        return 1;
    }
    if (connection) {
        return 1;
    }
    if (join) {
        return 1;
    }
    if (join2) {
        return 1;
    }
    if (join_list) {
        return 1;
    }
    if (object_field) {
        return 1;
    }
    if (object_field2) {
        return 1;
    }
    if (object_field_list) {
        return 1;
    }
    if (object) {
        return 1;
    }
    if (value_set) {
        return 1;
    }
    if (value_set2) {
        return 1;
    }
    if (result) {
        return 1;
    }
    if (result2) {
        return 1;
    }
    if (result_list) {
        return 1;
    }
    if (value) {
        return 1;
    }
    if (value2) {
        return 1;
    }
    return 0;
}

int clean_suite_classes(void) {
    libdbo_backend_handle_free(backend_handle);
    backend_handle = NULL;
    libdbo_backend_free(backend);
    backend = NULL;
    libdbo_backend_meta_data_free(backend_meta_data);
    backend_meta_data = NULL;
    libdbo_backend_meta_data_free(backend_meta_data2);
    backend_meta_data2 = NULL;
    libdbo_backend_meta_data_free(backend_meta_data3);
    backend_meta_data3 = NULL;
    libdbo_backend_meta_data_free(backend_meta_data4);
    backend_meta_data4 = NULL;
    libdbo_backend_meta_data_list_free(backend_meta_data_list);
    backend_meta_data_list = NULL;
    libdbo_backend_meta_data_list_free(backend_meta_data_list2);
    backend_meta_data_list2 = NULL;
    libdbo_clause_free(clause);
    clause = NULL;
    libdbo_clause_free(clause2);
    clause2 = NULL;
    libdbo_clause_list_free(clause_list);
    clause_list = NULL;
    libdbo_configuration_free(configuration);
    configuration = NULL;
    libdbo_configuration_free(configuration2);
    configuration2 = NULL;
    libdbo_configuration_list_free(configuration_list);
    configuration_list = NULL;
    libdbo_connection_free(connection);
    connection = NULL;
    libdbo_join_free(join);
    join = NULL;
    libdbo_join_free(join2);
    join2 = NULL;
    libdbo_join_list_free(join_list);
    join_list = NULL;
    libdbo_object_field_free(object_field);
    object_field = NULL;
    libdbo_object_field_free(object_field2);
    object_field2 = NULL;
    libdbo_object_field_list_free(object_field_list);
    object_field_list = NULL;
    libdbo_object_free(object);
    object = NULL;
    libdbo_value_set_free(value_set);
    value_set = NULL;
    libdbo_value_set_free(value_set2);
    value_set2 = NULL;
    libdbo_result_free(result);
    result = NULL;
    libdbo_result_free(result2);
    result2 = NULL;
    libdbo_result_list_free(result_list);
    result_list = NULL;
    libdbo_value_free(value);
    value = NULL;
    libdbo_value_free(value2);
    value2 = NULL;
    return 0;
}

int __libdbo_backend_handle_initialize(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_shutdown(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_connect(void* data, const libdbo_configuration_list_t* configuration_list) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)configuration_list == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_disconnect(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_create(void* data, const libdbo_object_t* _object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)_object == &fake_pointer || (object != NULL && _object == object));
    CU_ASSERT((void*)object_field_list == &fake_pointer);
    CU_ASSERT((void*)value_set == &fake_pointer);
    return 0;
}

libdbo_result_list_t* __libdbo_backend_handle_read(void* data, const libdbo_object_t* _object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)_object == &fake_pointer || (object != NULL && _object == object));
    CU_ASSERT((void*)join_list == &fake_pointer);
    CU_ASSERT((void*)clause_list == &fake_pointer);
    return (libdbo_result_list_t*)&fake_pointer;
}

int __libdbo_backend_handle_update(void* data, const libdbo_object_t* _object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)_object == &fake_pointer || (object != NULL && _object == object));
    CU_ASSERT((void*)object_field_list == &fake_pointer);
    CU_ASSERT((void*)value_set == &fake_pointer);
    CU_ASSERT((void*)clause_list == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_delete(void* data, const libdbo_object_t* _object, const libdbo_clause_list_t* clause_list) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)_object == &fake_pointer || (object != NULL && _object == object));
    CU_ASSERT((void*)clause_list == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_count(void* data, const libdbo_object_t* _object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    CU_ASSERT(data == &fake_pointer);
    CU_ASSERT((void*)_object == &fake_pointer || (object != NULL && _object == object));
    CU_ASSERT((void*)join_list == &fake_pointer);
    CU_ASSERT((void*)clause_list == &fake_pointer);
    CU_ASSERT((void*)count == &fake_pointer);
    return 0;
}

void __libdbo_backend_handle_free(void* data) {
    CU_ASSERT(data == &fake_pointer);
}

int __libdbo_backend_handle_transaction_begin(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_transaction_commit(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

int __libdbo_backend_handle_transaction_rollback(void* data) {
    CU_ASSERT(data == &fake_pointer);
    return 0;
}

void test_class_libdbo_backend_handle(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_handle = libdbo_backend_handle_new()));

    CU_ASSERT(!libdbo_backend_handle_set_initialize(backend_handle, __libdbo_backend_handle_initialize));
    CU_ASSERT(!libdbo_backend_handle_set_shutdown(backend_handle, __libdbo_backend_handle_shutdown));
    CU_ASSERT(!libdbo_backend_handle_set_connect(backend_handle, __libdbo_backend_handle_connect));
    CU_ASSERT(!libdbo_backend_handle_set_disconnect(backend_handle, __libdbo_backend_handle_disconnect));
    CU_ASSERT(!libdbo_backend_handle_set_create(backend_handle, __libdbo_backend_handle_create));
    CU_ASSERT(!libdbo_backend_handle_set_read(backend_handle, __libdbo_backend_handle_read));
    CU_ASSERT(!libdbo_backend_handle_set_update(backend_handle, __libdbo_backend_handle_update));
    CU_ASSERT(!libdbo_backend_handle_set_delete(backend_handle, __libdbo_backend_handle_delete));
    CU_ASSERT(!libdbo_backend_handle_set_count(backend_handle, __libdbo_backend_handle_count));
    CU_ASSERT(!libdbo_backend_handle_set_free(backend_handle, __libdbo_backend_handle_free));
    CU_ASSERT(!libdbo_backend_handle_set_transaction_begin(backend_handle, __libdbo_backend_handle_transaction_begin));
    CU_ASSERT(!libdbo_backend_handle_set_transaction_commit(backend_handle, __libdbo_backend_handle_transaction_commit));
    CU_ASSERT(!libdbo_backend_handle_set_transaction_rollback(backend_handle, __libdbo_backend_handle_transaction_rollback));
    CU_ASSERT(!libdbo_backend_handle_set_data(backend_handle, &fake_pointer));

    CU_ASSERT(!libdbo_backend_handle_not_empty(backend_handle));
    CU_ASSERT(libdbo_backend_handle_data(backend_handle) == &fake_pointer);

    CU_ASSERT(!libdbo_backend_handle_initialize(backend_handle));
    CU_ASSERT(!libdbo_backend_handle_shutdown(backend_handle));
    CU_ASSERT(!libdbo_backend_handle_connect(backend_handle, (libdbo_configuration_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_handle_disconnect(backend_handle));
    CU_ASSERT(!libdbo_backend_handle_create(backend_handle, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer));
    CU_ASSERT(libdbo_backend_handle_read(backend_handle, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer) == (libdbo_result_list_t*)&fake_pointer);
    CU_ASSERT(!libdbo_backend_handle_update(backend_handle, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_handle_delete(backend_handle, (libdbo_object_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_handle_count(backend_handle, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer, (size_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_handle_transaction_begin(backend_handle));
    CU_ASSERT(!libdbo_backend_handle_transaction_commit(backend_handle));
    CU_ASSERT(!libdbo_backend_handle_transaction_rollback(backend_handle));
}

void test_class_libdbo_backend(void) {
    libdbo_backend_handle_t* local_backend_handle = backend_handle;

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend = libdbo_backend_new()));
    CU_ASSERT(!libdbo_backend_set_name(backend, "test"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_name(backend));
    CU_ASSERT(!strcmp(libdbo_backend_name(backend), "test"));
    CU_ASSERT_FATAL(!libdbo_backend_set_handle(backend, backend_handle));
    backend_handle = NULL;
    CU_ASSERT(libdbo_backend_handle(backend) == local_backend_handle);
    CU_ASSERT(!libdbo_backend_not_empty(backend));

    CU_ASSERT(!libdbo_backend_initialize(backend));
    CU_ASSERT(!libdbo_backend_shutdown(backend));
    CU_ASSERT(!libdbo_backend_connect(backend, (libdbo_configuration_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_disconnect(backend));
    CU_ASSERT(!libdbo_backend_create(backend, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer));
    CU_ASSERT(libdbo_backend_read(backend, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer) == (libdbo_result_list_t*)&fake_pointer);
    CU_ASSERT(!libdbo_backend_update(backend, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_delete(backend, (libdbo_object_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_count(backend, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer, (size_t*)&fake_pointer));
    CU_ASSERT(!libdbo_backend_transaction_begin(backend));
    CU_ASSERT(!libdbo_backend_transaction_commit(backend));
    CU_ASSERT(!libdbo_backend_transaction_rollback(backend));
}

void test_class_libdbo_backend_meta_data(void) {
    libdbo_value_t* local_value;

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data = libdbo_backend_meta_data_new()));
    CU_ASSERT(!libdbo_backend_meta_data_set_name(backend_meta_data, "name1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new()));
    CU_ASSERT(!libdbo_value_from_text(local_value, "value1"));
    CU_ASSERT(!libdbo_backend_meta_data_set_value(backend_meta_data, local_value));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data), "name1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data)), "value1"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data2 = libdbo_backend_meta_data_new()));
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_copy(backend_meta_data2, backend_meta_data));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data2), "name1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data2)), "value1"));
    libdbo_backend_meta_data_free(backend_meta_data2);
    backend_meta_data2 = NULL;
    CU_PASS("libdbo_backend_meta_data_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data2 = libdbo_backend_meta_data_new_copy(backend_meta_data)));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data2), "name1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data2)), "value1"));
    libdbo_backend_meta_data_free(backend_meta_data2);
    backend_meta_data2 = NULL;
    CU_PASS("libdbo_backend_meta_data_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data2 = libdbo_backend_meta_data_new()));
    CU_ASSERT(!libdbo_backend_meta_data_set_name(backend_meta_data2, "name2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new()));
    CU_ASSERT(!libdbo_value_from_text(local_value, "value2"));
    CU_ASSERT(!libdbo_backend_meta_data_set_value(backend_meta_data2, local_value));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data2), "name2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data2));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data2)), "value2"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data3 = libdbo_backend_meta_data_new()));
    CU_ASSERT(!libdbo_backend_meta_data_set_name(backend_meta_data3, "name3"));
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new()));
    CU_ASSERT(!libdbo_value_from_text(local_value, "value3"));
    CU_ASSERT(!libdbo_backend_meta_data_set_value(backend_meta_data3, local_value));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data3));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data3));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data3), "name3"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data3));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data3)), "value3"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data4 = libdbo_backend_meta_data_new()));
    CU_ASSERT(!libdbo_backend_meta_data_set_name(backend_meta_data4, "name4"));
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new()));
    CU_ASSERT(!libdbo_value_from_text(local_value, "value4"));
    CU_ASSERT(!libdbo_backend_meta_data_set_value(backend_meta_data4, local_value));
    CU_ASSERT(!libdbo_backend_meta_data_not_empty(backend_meta_data4));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_name(backend_meta_data4));
    CU_ASSERT(!strcmp(libdbo_backend_meta_data_name(backend_meta_data4), "name4"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_backend_meta_data_value(backend_meta_data4));
    CU_ASSERT(!strcmp(libdbo_value_text(libdbo_backend_meta_data_value(backend_meta_data4)), "value4"));
}

void test_class_libdbo_backend_meta_data_list(void) {
    libdbo_backend_meta_data_t* local_backend_meta_data = backend_meta_data;
    libdbo_backend_meta_data_t* local_backend_meta_data2 = backend_meta_data2;
    libdbo_backend_meta_data_t* local_backend_meta_data3 = backend_meta_data3;
    libdbo_backend_meta_data_t* local_backend_meta_data4 = backend_meta_data4;

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list = libdbo_backend_meta_data_list_new()));
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_add(backend_meta_data_list, backend_meta_data));
    backend_meta_data = NULL;
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_add(backend_meta_data_list, backend_meta_data2));
    backend_meta_data2 = NULL;
    CU_ASSERT(libdbo_backend_meta_data_list_find(backend_meta_data_list, "name1") == local_backend_meta_data);
    CU_ASSERT(libdbo_backend_meta_data_list_find(backend_meta_data_list, "name2") == local_backend_meta_data2);

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list2 = libdbo_backend_meta_data_list_new()));
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_copy(backend_meta_data_list2, backend_meta_data_list));
    CU_ASSERT_PTR_NOT_NULL(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name1"));
    CU_ASSERT_PTR_NOT_NULL(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name2"));
    libdbo_backend_meta_data_list_free(backend_meta_data_list2);
    backend_meta_data_list2 = NULL;
    CU_PASS("libdbo_backend_meta_data_list_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list2 = libdbo_backend_meta_data_list_new_copy(backend_meta_data_list)));
    CU_ASSERT_PTR_NOT_NULL(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name1"));
    CU_ASSERT_PTR_NOT_NULL(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name2"));
    libdbo_backend_meta_data_list_free(backend_meta_data_list2);
    backend_meta_data_list2 = NULL;
    CU_PASS("libdbo_backend_meta_data_list_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list2 = libdbo_backend_meta_data_list_new()));
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_add(backend_meta_data_list2, backend_meta_data3));
    backend_meta_data3 = NULL;
    CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_add(backend_meta_data_list2, backend_meta_data4));
    backend_meta_data4 = NULL;
    CU_ASSERT(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name3") == local_backend_meta_data3);
    CU_ASSERT(libdbo_backend_meta_data_list_find(backend_meta_data_list2, "name4") == local_backend_meta_data4);
}

void test_class_libdbo_clause(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));

    CU_ASSERT(!libdbo_clause_set_table(clause, "table"));
    CU_ASSERT(!libdbo_clause_set_field(clause, "field"));
    CU_ASSERT(!libdbo_clause_set_type(clause, DB_CLAUSE_NOT_EQUAL));
    CU_ASSERT(!libdbo_clause_set_operator(clause, DB_CLAUSE_OPERATOR_OR));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_clause_get_value(clause));
    CU_ASSERT(!libdbo_value_from_int32(libdbo_clause_get_value(clause), 1));
    CU_ASSERT(!libdbo_clause_not_empty(clause));

    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_clause_table(clause));
    CU_ASSERT(!strcmp(libdbo_clause_table(clause), "table"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_clause_field(clause));
    CU_ASSERT(!strcmp(libdbo_clause_field(clause), "field"));
    CU_ASSERT(libdbo_clause_type(clause) == DB_CLAUSE_NOT_EQUAL);
    CU_ASSERT(libdbo_clause_operator(clause) == DB_CLAUSE_OPERATOR_OR);
    CU_ASSERT_PTR_NOT_NULL(libdbo_clause_value(clause));
    CU_ASSERT_PTR_NULL(libdbo_clause_next(clause));

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause2 = libdbo_clause_new()));

    CU_ASSERT(!libdbo_clause_set_type(clause2, DB_CLAUSE_NESTED));
    CU_ASSERT(!libdbo_clause_set_operator(clause2, DB_CLAUSE_OPERATOR_OR));
    CU_ASSERT(!libdbo_clause_set_list(clause2, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_clause_not_empty(clause2));

    CU_ASSERT(libdbo_clause_type(clause2) == DB_CLAUSE_NESTED);
    CU_ASSERT(libdbo_clause_operator(clause2) == DB_CLAUSE_OPERATOR_OR);
    CU_ASSERT(libdbo_clause_list(clause2) == (libdbo_clause_list_t*)&fake_pointer);
    CU_ASSERT_PTR_NOT_NULL(libdbo_clause_value(clause2));
}

void test_class_libdbo_clause_list(void) {
    libdbo_clause_t* local_clause = clause;
    libdbo_clause_t* local_clause2 = clause2;
    const libdbo_clause_t* clause_walk;

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));

    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause2));
    clause2 = NULL;

    CU_ASSERT((clause_walk = libdbo_clause_list_begin(clause_list)) == local_clause);
    CU_ASSERT(libdbo_clause_next(clause_walk) == local_clause2);

    libdbo_clause_list_free(clause_list);
    clause_list = NULL;
    CU_PASS("libdbo_clause_list_free");
    CU_PASS("libdbo_clause_free");
}

void test_class_libdbo_configuration(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT(!libdbo_configuration_set_name(configuration, "name1"));
    CU_ASSERT(!libdbo_configuration_set_value(configuration, "value1"));
    CU_ASSERT(!libdbo_configuration_not_empty(configuration));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_configuration_name(configuration));
    CU_ASSERT(!strcmp(libdbo_configuration_name(configuration), "name1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_configuration_value(configuration));
    CU_ASSERT(!strcmp(libdbo_configuration_value(configuration), "value1"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration2 = libdbo_configuration_new()));
    CU_ASSERT(!libdbo_configuration_set_name(configuration2, "name2"));
    CU_ASSERT(!libdbo_configuration_set_value(configuration2, "value2"));
    CU_ASSERT(!libdbo_configuration_not_empty(configuration2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_configuration_name(configuration2));
    CU_ASSERT(!strcmp(libdbo_configuration_name(configuration2), "name2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_configuration_value(configuration2));
    CU_ASSERT(!strcmp(libdbo_configuration_value(configuration2), "value2"));
}

void test_class_libdbo_configuration_list(void) {
    libdbo_configuration_t* local_configuration = configuration;
    libdbo_configuration_t* local_configuration2 = configuration2;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration_list = libdbo_configuration_list_new()));

    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration2));
    configuration2 = NULL;

    CU_ASSERT(libdbo_configuration_list_find(configuration_list, "name1") == local_configuration);
    CU_ASSERT(libdbo_configuration_list_find(configuration_list, "name2") == local_configuration2);

    libdbo_configuration_list_free(configuration_list);
    configuration_list = NULL;
    CU_PASS("libdbo_configuration_list_free");
    CU_PASS("libdbo_configuration_free");
}

void test_class_libdbo_connection(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((connection = libdbo_connection_new()));

    CU_ASSERT_FATAL(!libdbo_connection_set_configuration_list(connection, (libdbo_configuration_list_t*)&fake_pointer));

    connection->backend = backend;
    backend = NULL;

    CU_ASSERT_FATAL(!libdbo_connection_setup(connection));
    CU_ASSERT(!libdbo_connection_connect(connection));
    CU_ASSERT(!libdbo_connection_disconnect(connection));
    CU_ASSERT(!libdbo_connection_create(connection, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer));
    CU_ASSERT(libdbo_connection_read(connection, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer) == (libdbo_result_list_t*)&fake_pointer);
    CU_ASSERT(!libdbo_connection_update(connection, (libdbo_object_t*)&fake_pointer, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_connection_delete(connection, (libdbo_object_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_connection_count(connection, (libdbo_object_t*)&fake_pointer, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer, (size_t*)&fake_pointer));
    CU_ASSERT(!libdbo_connection_transaction_begin(connection));
    CU_ASSERT(!libdbo_connection_transaction_commit(connection));
    CU_ASSERT(!libdbo_connection_transaction_rollback(connection));
}

void test_class_libdbo_join(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((join = libdbo_join_new()));
    CU_ASSERT(!libdbo_join_set_from_table(join, "from_table1"));
    CU_ASSERT(!libdbo_join_set_from_field(join, "from_field1"));
    CU_ASSERT(!libdbo_join_set_to_table(join, "to_table1"));
    CU_ASSERT(!libdbo_join_set_to_field(join, "to_field1"));
    CU_ASSERT(!libdbo_join_not_empty(join));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_from_table(join));
    CU_ASSERT(!strcmp(libdbo_join_from_table(join), "from_table1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_from_field(join));
    CU_ASSERT(!strcmp(libdbo_join_from_field(join), "from_field1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_to_table(join));
    CU_ASSERT(!strcmp(libdbo_join_to_table(join), "to_table1"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_to_field(join));
    CU_ASSERT(!strcmp(libdbo_join_to_field(join), "to_field1"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((join2 = libdbo_join_new()));
    CU_ASSERT(!libdbo_join_set_from_table(join2, "from_table2"));
    CU_ASSERT(!libdbo_join_set_from_field(join2, "from_field2"));
    CU_ASSERT(!libdbo_join_set_to_table(join2, "to_table2"));
    CU_ASSERT(!libdbo_join_set_to_field(join2, "to_field2"));
    CU_ASSERT(!libdbo_join_not_empty(join2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_from_table(join2));
    CU_ASSERT(!strcmp(libdbo_join_from_table(join2), "from_table2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_from_field(join2));
    CU_ASSERT(!strcmp(libdbo_join_from_field(join2), "from_field2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_to_table(join2));
    CU_ASSERT(!strcmp(libdbo_join_to_table(join2), "to_table2"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_join_to_field(join2));
    CU_ASSERT(!strcmp(libdbo_join_to_field(join2), "to_field2"));
}

void test_class_libdbo_join_list(void) {
    libdbo_join_t* local_join = join;
    libdbo_join_t* local_join2 = join2;
    const libdbo_join_t* join_walk;

    CU_ASSERT_PTR_NOT_NULL_FATAL((join_list = libdbo_join_list_new()));

    CU_ASSERT_FATAL(!libdbo_join_list_add(join_list, join));
    join = NULL;
    CU_ASSERT_FATAL(!libdbo_join_list_add(join_list, join2));
    join2 = NULL;

    CU_ASSERT((join_walk = libdbo_join_list_begin(join_list)) == local_join);
    CU_ASSERT(libdbo_join_next(join_walk) == local_join2);

    libdbo_join_list_free(join_list);
    join_list = NULL;
    CU_PASS("libdbo_join_list_free");
    CU_PASS("libdbo_join_free");
}

void test_class_libdbo_object_field(void) {
    libdbo_object_field_t* local_object_field;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT(!libdbo_object_field_set_name(object_field, "field1"));
    CU_ASSERT(!libdbo_object_field_set_type(object_field, DB_TYPE_INT32));
    CU_ASSERT(!libdbo_object_field_not_empty(object_field));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(object_field));
    CU_ASSERT(!strcmp(libdbo_object_field_name(object_field), "field1"));
    CU_ASSERT(libdbo_object_field_type(object_field) == DB_TYPE_INT32);

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_object_field = libdbo_object_field_new()));
    CU_ASSERT(!libdbo_object_field_copy(local_object_field, object_field));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(local_object_field));
    CU_ASSERT(!strcmp(libdbo_object_field_name(local_object_field), "field1"));
    CU_ASSERT(libdbo_object_field_type(local_object_field) == DB_TYPE_INT32);
    libdbo_object_field_free(local_object_field);
    local_object_field = NULL;
    CU_PASS("libdbo_object_field_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_object_field = libdbo_object_field_new_copy(object_field)));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(local_object_field));
    CU_ASSERT(!strcmp(libdbo_object_field_name(local_object_field), "field1"));
    CU_ASSERT(libdbo_object_field_type(local_object_field) == DB_TYPE_INT32);
    libdbo_object_field_free(local_object_field);
    local_object_field = NULL;
    CU_PASS("libdbo_object_field_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field2 = libdbo_object_field_new()));
    CU_ASSERT(!libdbo_object_field_set_name(object_field2, "field2"));
    CU_ASSERT(!libdbo_object_field_set_type(object_field2, DB_TYPE_ENUM));
    CU_ASSERT(!libdbo_object_field_set_enum_set(object_field2, (libdbo_enum_t*)&fake_pointer));
    CU_ASSERT(!libdbo_object_field_not_empty(object_field2));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(object_field2));
    CU_ASSERT(!strcmp(libdbo_object_field_name(object_field2), "field2"));
    CU_ASSERT(libdbo_object_field_type(object_field2) == DB_TYPE_ENUM);
    CU_ASSERT(libdbo_object_field_enum_set(object_field2) == (libdbo_enum_t*)&fake_pointer);
}

void test_class_libdbo_object_field_list(void) {
    libdbo_object_field_t* local_object_field = object_field;
    libdbo_object_field_t* local_object_field2 = object_field2;
    const libdbo_object_field_t* object_field_walk;
    libdbo_object_field_list_t* local_object_field_list;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));

    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));
    object_field = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_object_field_list = libdbo_object_field_list_new()));
    CU_ASSERT(!libdbo_object_field_list_copy(local_object_field_list, object_field_list));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_walk = libdbo_object_field_list_begin(object_field_list)));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(object_field_walk));
    CU_ASSERT(!strcmp(libdbo_object_field_name(object_field_walk), "field1"));
    CU_ASSERT(libdbo_object_field_type(object_field_walk) == DB_TYPE_INT32);
    libdbo_object_field_list_free(local_object_field_list);
    local_object_field_list = NULL;
    CU_PASS("libdbo_object_field_list_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_object_field_list = libdbo_object_field_list_new_copy(object_field_list)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_walk = libdbo_object_field_list_begin(object_field_list)));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_field_name(object_field_walk));
    CU_ASSERT(!strcmp(libdbo_object_field_name(object_field_walk), "field1"));
    CU_ASSERT(libdbo_object_field_type(object_field_walk) == DB_TYPE_INT32);
    libdbo_object_field_list_free(local_object_field_list);
    local_object_field_list = NULL;
    CU_PASS("libdbo_object_field_list_free");

    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field2));
    object_field2 = NULL;

    CU_ASSERT((object_field_walk = libdbo_object_field_list_begin(object_field_list)) == local_object_field);
    CU_ASSERT(libdbo_object_field_next(object_field_walk) == local_object_field2);
}

void test_class_libdbo_object(void) {
    libdbo_object_field_list_t* local_object_field_list = object_field_list;
    libdbo_backend_meta_data_list_t* local_backend_meta_data_list = backend_meta_data_list;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object = libdbo_object_new()));

    CU_ASSERT(!libdbo_object_set_connection(object, connection));
    CU_ASSERT(!libdbo_object_set_table(object, "table"));
    CU_ASSERT(!libdbo_object_set_primary_key_name(object, "primary_key"));
    CU_ASSERT(!libdbo_object_set_object_field_list(object, object_field_list));
    object_field_list = NULL;
    CU_ASSERT(!libdbo_object_set_backend_meta_data_list(object, backend_meta_data_list));
    backend_meta_data_list = NULL;

    CU_ASSERT(libdbo_object_connection(object) == connection);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_table(object));
    CU_ASSERT(!strcmp(libdbo_object_table(object), "table"));
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_object_primary_key_name(object));
    CU_ASSERT(!strcmp(libdbo_object_primary_key_name(object), "primary_key"));
    CU_ASSERT(libdbo_object_object_field_list(object) == local_object_field_list);
    CU_ASSERT(libdbo_object_backend_meta_data_list(object) == local_backend_meta_data_list);

    CU_ASSERT(!libdbo_object_create(object, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer));
    CU_ASSERT(libdbo_object_read(object, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer) == (libdbo_result_list_t*)&fake_pointer);
    CU_ASSERT(!libdbo_object_update(object, (libdbo_object_field_list_t*)&fake_pointer, (libdbo_value_set_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_object_delete(object, (libdbo_clause_list_t*)&fake_pointer));
    CU_ASSERT(!libdbo_object_count(object, (libdbo_join_list_t*)&fake_pointer, (libdbo_clause_list_t*)&fake_pointer, (size_t*)&fake_pointer));

    libdbo_object_free(object);
    object = NULL;
    CU_PASS("libdbo_object_free");
}

void test_class_libdbo_value_set(void) {
    libdbo_value_set_t* local_value_set;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(2)));
    CU_ASSERT(libdbo_value_set_size(value_set) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set, 1));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(value_set, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set, 1));
    CU_ASSERT_PTR_NULL(libdbo_value_set_get(value_set, 2));

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set2 = libdbo_value_set_new(6)));
    CU_ASSERT(libdbo_value_set_size(value_set2) == 6);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 1));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 3));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 4));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set2, 5));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(value_set2, 6));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 1));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 3));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 4));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set2, 5));
    CU_ASSERT_PTR_NULL(libdbo_value_set_get(value_set2, 6));

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value_set = libdbo_value_set_new_copy(value_set2)));
    CU_ASSERT(libdbo_value_set_size(local_value_set) == 6);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 1));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 3));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 4));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(local_value_set, 5));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(local_value_set, 6));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 1));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 3));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 4));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(local_value_set, 5));
    CU_ASSERT_PTR_NULL(libdbo_value_set_get(local_value_set, 6));
    libdbo_value_set_free(local_value_set);
    CU_PASS("libdbo_value_set_free");
}

void test_class_libdbo_result(void) {
    libdbo_value_set_t* local_value_set = value_set;
    libdbo_value_set_t* local_value_set2 = value_set2;
    libdbo_backend_meta_data_list_t* local_backend_meta_data_list2 = backend_meta_data_list2;
    libdbo_result_t* local_result;

    CU_ASSERT_PTR_NOT_NULL_FATAL((result = libdbo_result_new()));
    CU_ASSERT(!libdbo_result_set_value_set(result, value_set));
    value_set = NULL;
    CU_ASSERT(libdbo_result_value_set(result) == local_value_set);
    CU_ASSERT(!libdbo_result_set_backend_meta_data_list(result, backend_meta_data_list2));
    backend_meta_data_list2 = NULL;
    CU_ASSERT(libdbo_result_backend_meta_data_list(result) == local_backend_meta_data_list2);
    CU_ASSERT(!libdbo_result_not_empty(result));

    CU_ASSERT_PTR_NOT_NULL_FATAL((result2 = libdbo_result_new()));
    CU_ASSERT(!libdbo_result_set_value_set(result2, value_set2));
    value_set2 = NULL;
    CU_ASSERT(libdbo_result_value_set(result2) == local_value_set2);
    CU_ASSERT(!libdbo_result_not_empty(result2));

    /*
     * TODO: test deep value copy success.
     */

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_result = libdbo_result_new()));
    CU_ASSERT_FATAL(!libdbo_result_copy(local_result, result));
    CU_ASSERT(libdbo_value_set_size(libdbo_result_value_set(local_result)) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 1));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 2));
    libdbo_result_free(local_result);
    CU_PASS("libdbo_result_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_result = libdbo_result_new_copy(result2)));
    CU_ASSERT(libdbo_value_set_size(libdbo_result_value_set(local_result)) == 6);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 1));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 3));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 4));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 5));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(libdbo_result_value_set(local_result), 6));
    libdbo_result_free(local_result);
    CU_PASS("libdbo_result_free");
}

static int __libdbo_result_list_next_count = 0;
libdbo_result_t* __libdbo_result_list_next(void* data, int finish) {
    libdbo_value_set_t* value_set;
    libdbo_result_t* result;

    CU_ASSERT_FATAL(data == &fake_pointer);

    if (finish) {
        return NULL;
    }

    if (__libdbo_result_list_next_count > 2) {
        return NULL;
    }

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(2)));
    CU_ASSERT(libdbo_value_set_size(value_set) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_at(value_set, 1));
    CU_ASSERT_PTR_NULL(libdbo_value_set_at(value_set, 2));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set, 0));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_set_get(value_set, 1));
    CU_ASSERT_PTR_NULL(libdbo_value_set_get(value_set, 2));

    CU_ASSERT_PTR_NOT_NULL_FATAL((result = libdbo_result_new()));
    CU_ASSERT(!libdbo_result_set_value_set(result, value_set));
    CU_ASSERT(!libdbo_result_not_empty(result));

    __libdbo_result_list_next_count++;

    return result;
}

void test_class_libdbo_result_list(void) {
    libdbo_result_t* local_result = result;
    libdbo_result_t* local_result2 = result2;
    libdbo_result_list_t* local_result_list;

    CU_ASSERT_PTR_NOT_NULL_FATAL((result_list = libdbo_result_list_new()));

    CU_ASSERT_FATAL(!libdbo_result_list_add(result_list, result));
    result = NULL;
    CU_ASSERT_FATAL(!libdbo_result_list_add(result_list, result2));
    result2 = NULL;

    CU_ASSERT(libdbo_result_list_size(result_list) == 2);
    CU_ASSERT(libdbo_result_list_begin(result_list) == local_result);
    CU_ASSERT(libdbo_result_list_next(result_list) == local_result2);

    /*
     * TODO: test deep result copy success.
     */

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_result_list = libdbo_result_list_new()));
    CU_ASSERT(!libdbo_result_list_copy(local_result_list, result_list));
    CU_ASSERT(libdbo_result_list_size(local_result_list) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_begin(local_result_list));
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_next(local_result_list));
    CU_ASSERT_PTR_NULL(libdbo_result_list_next(local_result_list));
    libdbo_result_list_free(local_result_list);
    local_result_list = NULL;
    CU_PASS("libdbo_result_list_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((local_result_list = libdbo_result_list_new_copy(result_list)));
    CU_ASSERT(libdbo_result_list_size(local_result_list) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_begin(local_result_list));
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_next(local_result_list));
    CU_ASSERT_PTR_NULL(libdbo_result_list_next(local_result_list));
    libdbo_result_list_free(local_result_list);
    local_result_list = NULL;
    CU_PASS("libdbo_result_list_free");

    libdbo_result_list_free(result_list);
    result_list = NULL;
    CU_PASS("libdbo_result_list_free");
    CU_PASS("libdbo_result_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((result_list = libdbo_result_list_new()));

    CU_ASSERT_FATAL(!libdbo_result_list_set_next(result_list, __libdbo_result_list_next, &fake_pointer, 2));

    CU_ASSERT(libdbo_result_list_size(result_list) == 2);
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_begin(result_list));
    CU_ASSERT_PTR_NOT_NULL(libdbo_result_list_next(result_list));

    libdbo_result_list_free(result_list);
    result_list = NULL;
    CU_PASS("libdbo_result_list_free");
    CU_PASS("libdbo_result_free");
}

void test_class_libdbo_value(void) {
    char* text = NULL;
    const char* enum_text = NULL;
    int ret;
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    libdbo_value_t* local_value;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value2 = libdbo_value_new()));

    CU_ASSERT_PTR_NOT_NULL_FATAL((value = libdbo_value_new()));
    CU_ASSERT(!libdbo_value_from_text(value, "test"));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_TEXT);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_text(value));
    CU_ASSERT_PTR_NULL(libdbo_value_int32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_int64(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint64(value));
    CU_ASSERT(!strcmp(libdbo_value_text(value), "test"));
    CU_ASSERT(!libdbo_value_to_text(value, &text));
    CU_ASSERT_PTR_NOT_NULL(text);
    free(text);
    text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_TEXT);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_text(value2), "test"));
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_TEXT);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_text(local_value));
    CU_ASSERT(!strcmp(libdbo_value_text(local_value), "test"));
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_set_primary_key(value));
    CU_ASSERT(libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_int32(value, -12345));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_int32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_int64(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint64(value));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_INT32);
    CU_ASSERT(!libdbo_value_to_int32(value, &int32));
    CU_ASSERT(int32 == -12345);
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_INT32);
    CU_ASSERT(!libdbo_value_to_int32(value2, &int32));
    CU_ASSERT(int32 == -12345);
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_INT32);
    CU_ASSERT(!libdbo_value_to_int32(local_value, &int32));
    CU_ASSERT(int32 == -12345);
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_set_primary_key(value));
    CU_ASSERT(libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_uint32(value, 12345));
    CU_ASSERT_PTR_NULL(libdbo_value_int32(value));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_uint32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_int64(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint64(value));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_UINT32);
    CU_ASSERT(!libdbo_value_to_uint32(value, &uint32));
    CU_ASSERT(uint32 == 12345);
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_UINT32);
    CU_ASSERT(!libdbo_value_to_uint32(value2, &uint32));
    CU_ASSERT(uint32 == 12345);
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_UINT32);
    CU_ASSERT(!libdbo_value_to_uint32(local_value, &uint32));
    CU_ASSERT(uint32 == 12345);
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_set_primary_key(value));
    CU_ASSERT(libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_int64(value, -9223372036854775800));
    CU_ASSERT_PTR_NULL(libdbo_value_int32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint32(value));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_int64(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint64(value));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_INT64);
    CU_ASSERT(!libdbo_value_to_int64(value, &int64));
    CU_ASSERT(int64 == -9223372036854775800);
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_INT64);
    CU_ASSERT(!libdbo_value_to_int64(value2, &int64));
    CU_ASSERT(int64 == -9223372036854775800);
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_INT64);
    CU_ASSERT(!libdbo_value_to_int64(local_value, &int64));
    CU_ASSERT(int64 == -9223372036854775800);
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_set_primary_key(value));
    CU_ASSERT(libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_uint64(value, 17446744073709551615UL));
    CU_ASSERT_PTR_NULL(libdbo_value_int32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_uint32(value));
    CU_ASSERT_PTR_NULL(libdbo_value_int64(value));
    CU_ASSERT_PTR_NOT_NULL(libdbo_value_uint64(value));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_UINT64);
    CU_ASSERT(!libdbo_value_to_uint64(value, &uint64));
    CU_ASSERT(uint64 == 17446744073709551615UL);
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_UINT64);
    CU_ASSERT(!libdbo_value_to_uint64(value2, &uint64));
    CU_ASSERT(uint64 == 17446744073709551615UL);
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_UINT64);
    CU_ASSERT(!libdbo_value_to_uint64(local_value, &uint64));
    CU_ASSERT(uint64 == 17446744073709551615UL);
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_set_primary_key(value));
    CU_ASSERT(libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_value(value, 2, enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum2"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 2);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum2"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum2"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 2);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum2"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT_PTR_NOT_NULL_FATAL((local_value = libdbo_value_new_copy(value)));
    CU_ASSERT(libdbo_value_type(local_value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(local_value, &ret));
    CU_ASSERT(ret == 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(local_value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(local_value), "enum2"));
    CU_ASSERT(!libdbo_value_to_enum_value(local_value, &ret, enum_set));
    CU_ASSERT(ret == 2);
    CU_ASSERT(!libdbo_value_to_enum_text(local_value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum2"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, local_value, &ret));
    CU_ASSERT(!ret);
    libdbo_value_reset(local_value);
    local_value = NULL;
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_text(value, "enum2", enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum2"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 2);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum2"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum2"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 2);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum2"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_value(value, 3, enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 3);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum3"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 3);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum3"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 3);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum3"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 3);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum3"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_text(value, "enum3", enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 3);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum3"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 3);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum3"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 3);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum3"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 3);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum3"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_value(value, 1, enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum1"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 1);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum1"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum1"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 1);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum1"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_reset(value);
    CU_PASS("libdbo_value_reset");

    CU_ASSERT(!libdbo_value_from_enum_text(value, "enum1", enum_set));
    CU_ASSERT(libdbo_value_type(value) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value, &ret));
    CU_ASSERT(ret == 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value), "enum1"));
    CU_ASSERT(!libdbo_value_to_enum_value(value, &ret, enum_set));
    CU_ASSERT(ret == 1);
    CU_ASSERT(!libdbo_value_to_enum_text(value, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum1"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_not_empty(value));
    libdbo_value_reset(value2);
    CU_PASS("libdbo_value_reset");
    CU_ASSERT(!libdbo_value_copy(value2, value));
    CU_ASSERT(libdbo_value_type(value2) == DB_TYPE_ENUM);
    CU_ASSERT(!libdbo_value_enum_value(value2, &ret));
    CU_ASSERT(ret == 1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(libdbo_value_enum_text(value2));
    CU_ASSERT(!strcmp(libdbo_value_enum_text(value2), "enum1"));
    CU_ASSERT(!libdbo_value_to_enum_value(value2, &ret, enum_set));
    CU_ASSERT(ret == 1);
    CU_ASSERT(!libdbo_value_to_enum_text(value2, &enum_text, enum_set));
    CU_ASSERT_PTR_NOT_NULL_FATAL(enum_text);
    CU_ASSERT(!strcmp(enum_text, "enum1"));
    enum_text = NULL;
    CU_ASSERT(!libdbo_value_cmp(value, value2, &ret));
    CU_ASSERT(!ret);
    CU_ASSERT(libdbo_value_set_primary_key(value));
    CU_ASSERT(!libdbo_value_primary_key(value));

    libdbo_value_free(value);
    value = NULL;
    CU_PASS("libdbo_value_free");
    libdbo_value_free(value2);
    value2 = NULL;
    CU_PASS("libdbo_value_free");
}

void test_class_end(void) {
    libdbo_result_free(result);
    result = NULL;
    libdbo_result_free(result2);
    result2 = NULL;
    CU_PASS("libdbo_result_free");

    libdbo_value_set_free(value_set);
    value_set = NULL;
    libdbo_value_set_free(value_set2);
    value_set2 = NULL;
    CU_PASS("libdbo_value_set_free");

    libdbo_object_field_list_free(object_field_list);
    object_field_list = NULL;
    CU_PASS("libdbo_object_field_list_free");
    CU_PASS("libdbo_object_field_free");

    libdbo_connection_free(connection);
    connection = NULL;
    CU_PASS("libdbo_connection_free");

    libdbo_backend_free(backend);
    backend = NULL;
    CU_PASS("libdbo_backend_handle_free");
    CU_PASS("libdbo_backend_free");
}
