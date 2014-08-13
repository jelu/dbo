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
 * Based on enforcer-ng/src/db/test/test_database_operations.c source file from
 * the OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "config.h"

#include <libdbo/configuration.h>
#include <libdbo/connection.h>
#include <libdbo/object.h>

#include "users_rev.h"
#include "groups_rev.h"
#include "user_group_link_rev.h"

#include "CUnit/Basic.h"
#include <string.h>

typedef struct {
    libdbo_object_t* dbo;
    libdbo_value_t* id;
    char* name;
} test_t;

typedef struct {
    libdbo_object_t* dbo;
    libdbo_result_list_t* result_list;
    test_t* test;
} test_list_t;

static libdbo_configuration_list_t* configuration_list = NULL;
static libdbo_configuration_t* configuration = NULL;
static libdbo_connection_t* connection = NULL;
static test_t* test = NULL;
static test_list_t* test_list = NULL;
static libdbo_value_t object2_id, object3_id;

libdbo_object_t* __test_new_object(const libdbo_connection_t* connection) {
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_object_t* object;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object = libdbo_object_new()));

    CU_ASSERT_FATAL(!libdbo_object_set_connection(object, connection));
    CU_ASSERT_FATAL(!libdbo_object_set_table(object, "test"));
    CU_ASSERT_FATAL(!libdbo_object_set_primary_key_name(object, "id"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "id"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_PRIMARY_KEY));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));

    CU_ASSERT_FATAL(!libdbo_object_set_object_field_list(object, object_field_list));

    return object;
}

test_t* test_new(const libdbo_connection_t* connection) {
    test_t* test =
        (test_t*)calloc(1, sizeof(test_t));

    if (test) {
        CU_ASSERT_PTR_NOT_NULL_FATAL((test->dbo = __test_new_object(connection)));
        CU_ASSERT_PTR_NOT_NULL_FATAL((test->id = libdbo_value_new()));
    }

    return test;
}

void test_free(test_t* test) {
    if (test) {
        if (test->dbo) {
            libdbo_object_free(test->dbo);
        }
        if (test->id) {
            libdbo_value_free(test->id);
        }
        if (test->name) {
            free(test->name);
        }
        free(test);
    }
}

const libdbo_value_t* test_id(const test_t* test) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test);

    return test->id;
}

const char* test_name(const test_t* test) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test);

    return test->name;
}

int test_set_name(test_t* test, const char *name) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(name);

    if (test->name) {
        free(test->name);
    }
    test->name = strdup(name);
    CU_ASSERT_PTR_NOT_NULL_FATAL(test->name);
    return 0;
}

int test_from_result(test_t* test, const libdbo_result_t* result) {
    const libdbo_value_set_t* value_set;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(result);

    libdbo_value_reset(test->id);
    if (test->name) {
        free(test->name);
    }
    test->name = NULL;

    if (libdbo_result_backend_meta_data_list(result)) {
        libdbo_backend_meta_data_list_t* backend_meta_data_list;

        CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list = libdbo_backend_meta_data_list_new()));
        CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_copy(backend_meta_data_list, libdbo_result_backend_meta_data_list(result)));
        CU_ASSERT_FATAL(!libdbo_object_set_backend_meta_data_list(test->dbo, backend_meta_data_list));
    }

    value_set = libdbo_result_value_set(result);

    CU_ASSERT_PTR_NOT_NULL_FATAL(value_set);
    CU_ASSERT_FATAL(libdbo_value_set_size(value_set) == 2);
    CU_ASSERT_FATAL(!libdbo_value_copy(test->id, libdbo_value_set_at(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_to_text(libdbo_value_set_at(value_set, 1), &(test->name)));
    return 0;
}

int test_get_by_name(test_t* test, const char* name) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_result_list_t* result_list;
    const libdbo_result_t* result;
    int ret;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "name"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_from_text(libdbo_clause_get_value(clause), name));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    ret = 1;
    result_list = libdbo_object_read(test->dbo, NULL, clause_list);
    if (result_list) {
        result = libdbo_result_list_next(result_list);
        if (result) {
            test_from_result(test, result);
            ret = 0;
        }
        CU_ASSERT_PTR_NULL((result = libdbo_result_list_next(result_list)));
        if (result) {
            libdbo_result_list_free(result_list);
            libdbo_clause_list_free(clause_list);
            return 1;
        }
    }

    libdbo_result_list_free(result_list);
    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

int test_get_by_id(test_t* test, const libdbo_value_t* id) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_result_list_t* result_list;
    const libdbo_result_t* result;
    int ret;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    ret = 1;
    result_list = libdbo_object_read(test->dbo, NULL, clause_list);
    if (result_list) {
        result = libdbo_result_list_next(result_list);
        if (result) {
            test_from_result(test, result);
            ret = 0;
        }
        CU_ASSERT_PTR_NULL((result = libdbo_result_list_next(result_list)));
        if (result) {
            libdbo_result_list_free(result_list);
            libdbo_clause_list_free(clause_list);
            return 1;
        }
    }

    libdbo_result_list_free(result_list);
    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

int test_create(test_t* test) {
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_value_set_t* value_set;
    libdbo_value_t* value;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_FATAL(libdbo_value_not_empty(test->id));
    CU_ASSERT_PTR_NOT_NULL_FATAL(test->name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));
    object_field = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(1)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((value = libdbo_value_set_get(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_from_text(value, test->name));

    if (libdbo_object_create(test->dbo, object_field_list, value_set)) {
        ret = 1;
    }

    libdbo_value_set_free(value_set);
    libdbo_object_field_free(object_field);
    libdbo_object_field_list_free(object_field_list);
    CU_ASSERT(!ret);
    return ret;
}

int test_update(test_t* test) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_value_set_t* value_set;
    libdbo_value_t* value;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test->id));
    CU_ASSERT_PTR_NOT_NULL_FATAL(test->name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test->id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));
    object_field = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(1)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((value = libdbo_value_set_get(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_from_text(value, test->name));

    if (libdbo_object_update(test->dbo, object_field_list, value_set, clause_list)) {
        ret = 1;
    }

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    libdbo_value_set_free(value_set);
    libdbo_object_field_free(object_field);
    libdbo_object_field_list_free(object_field_list);
    CU_ASSERT(!ret);
    return ret;
}

int test_delete(test_t* test) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test->id));

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test->id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    if (libdbo_object_delete(test->dbo, clause_list)) {
        ret = 1;
    }

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    CU_ASSERT(!ret);
    return ret;
}

size_t test_count_by_name(test_t* test, const char* name) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    size_t ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "name"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_from_text(libdbo_clause_get_value(clause), name));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    CU_ASSERT(!libdbo_object_count(test->dbo, NULL, clause_list, &ret));

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

size_t test_count_by_id(test_t* test, const libdbo_value_t* id) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    size_t ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    CU_ASSERT(!libdbo_object_count(test->dbo, NULL, clause_list, &ret));

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

test_list_t* test_list_new(const libdbo_connection_t* connection) {
    test_list_t* test_list =
        (test_list_t*)calloc(1, sizeof(test_list_t));

    if (test_list) {
        CU_ASSERT_PTR_NOT_NULL_FATAL((test_list->dbo = __test_new_object(connection)));
    }

    return test_list;
}

void test_list_free(test_list_t* test_list) {
    if (test_list) {
        if (test_list->dbo) {
            libdbo_object_free(test_list->dbo);
        }
        if (test_list->result_list) {
            libdbo_result_list_free(test_list->result_list);
        }
        if (test_list->test) {
            test_free(test_list->test);
        }
        free(test_list);
    }
}

int test_list_get(test_list_t* test_list) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test_list);
    CU_ASSERT_PTR_NOT_NULL_FATAL(test_list->dbo);

    if (test_list->result_list) {
        libdbo_result_list_free(test_list->result_list);
    }
    CU_ASSERT_PTR_NOT_NULL((test_list->result_list = libdbo_object_read(test_list->dbo, NULL, NULL)));
    if (!test_list->result_list) {
        return 1;
    }
    return 0;
}

const test_t* test_list_begin(test_list_t* test_list) {
    const libdbo_result_t* result;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test_list);
    CU_ASSERT_PTR_NOT_NULL_FATAL(test_list->result_list);

    result = libdbo_result_list_next(test_list->result_list);
    if (!result) {
        return NULL;
    }
    if (!test_list->test) {
        CU_ASSERT_PTR_NOT_NULL_FATAL((test_list->test = test_new(libdbo_object_connection(test_list->dbo))));
    }
    if (test_from_result(test_list->test, result)) {
        return NULL;
    }
    return test_list->test;
}

const test_t* test_list_next(test_list_t* test_list) {
    const libdbo_result_t* result;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test_list);

    result = libdbo_result_list_next(test_list->result_list);
    if (!result) {
        return NULL;
    }
    if (!test_list->test) {
        CU_ASSERT_PTR_NOT_NULL_FATAL((test_list->test = test_new(libdbo_object_connection(test_list->dbo))));
    }
    if (test_from_result(test_list->test, result)) {
        return NULL;
    }
    return test_list->test;
}

typedef struct {
    libdbo_object_t* dbo;
    libdbo_value_t* id;
    libdbo_value_t* rev;
    char* name;
} test2_t;

static test2_t* test2 = NULL;
static test2_t* test2_2 = NULL;

libdbo_object_t* __test2_new_object(const libdbo_connection_t* connection) {
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_object_t* object;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object = libdbo_object_new()));

    CU_ASSERT_FATAL(!libdbo_object_set_connection(object, connection));
    CU_ASSERT_FATAL(!libdbo_object_set_table(object, "test2"));
    CU_ASSERT_FATAL(!libdbo_object_set_primary_key_name(object, "id"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "id"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_PRIMARY_KEY));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "rev"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_REVISION));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));

    CU_ASSERT_FATAL(!libdbo_object_set_object_field_list(object, object_field_list));

    return object;
}

test2_t* test2_new(const libdbo_connection_t* connection) {
    test2_t* test2 =
        (test2_t*)calloc(1, sizeof(test2_t));

    if (test2) {
        CU_ASSERT_PTR_NOT_NULL_FATAL((test2->dbo = __test2_new_object(connection)));
        CU_ASSERT_PTR_NOT_NULL_FATAL((test2->id = libdbo_value_new()));
        CU_ASSERT_PTR_NOT_NULL_FATAL((test2->rev = libdbo_value_new()));
    }

    return test2;
}

void test2_free(test2_t* test2) {
    if (test2) {
        if (test2->dbo) {
            libdbo_object_free(test2->dbo);
        }
        if (test2->id) {
            libdbo_value_free(test2->id);
        }
        if (test2->rev) {
            libdbo_value_free(test2->rev);
        }
        if (test2->name) {
            free(test2->name);
        }
        free(test2);
    }
}

const libdbo_value_t* test2_id(const test2_t* test2) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);

    return test2->id;
}

const char* test2_name(const test2_t* test2) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);

    return test2->name;
}

int test2_set_name(test2_t* test2, const char *name) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(name);

    if (test2->name) {
        free(test2->name);
    }
    test2->name = strdup(name);
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2->name);
    return 0;
}

int test2_from_result(test2_t* test2, const libdbo_result_t* result) {
    const libdbo_value_set_t* value_set;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(result);

    libdbo_value_reset(test2->id);
    libdbo_value_reset(test2->rev);
    if (test2->name) {
        free(test2->name);
    }
    test2->name = NULL;

    if (libdbo_result_backend_meta_data_list(result)) {
        libdbo_backend_meta_data_list_t* backend_meta_data_list;

        CU_ASSERT_PTR_NOT_NULL_FATAL((backend_meta_data_list = libdbo_backend_meta_data_list_new()));
        CU_ASSERT_FATAL(!libdbo_backend_meta_data_list_copy(backend_meta_data_list, libdbo_result_backend_meta_data_list(result)));
        CU_ASSERT_FATAL(!libdbo_object_set_backend_meta_data_list(test2->dbo, backend_meta_data_list));
    }

    value_set = libdbo_result_value_set(result);

    CU_ASSERT_PTR_NOT_NULL_FATAL(value_set);
    CU_ASSERT_FATAL(libdbo_value_set_size(value_set) == 3);
    CU_ASSERT_FATAL(!libdbo_value_copy(test2->id, libdbo_value_set_at(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_copy(test2->rev, libdbo_value_set_at(value_set, 1)));
    CU_ASSERT_FATAL(!libdbo_value_to_text(libdbo_value_set_at(value_set, 2), &(test2->name)));
    return 0;
}

int test2_get_by_name(test2_t* test2, const char* name) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_result_list_t* result_list;
    const libdbo_result_t* result;
    int ret;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "name"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_from_text(libdbo_clause_get_value(clause), name));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    ret = 1;
    result_list = libdbo_object_read(test2->dbo, NULL, clause_list);
    if (result_list) {
        result = libdbo_result_list_next(result_list);
        if (result) {
            test2_from_result(test2, result);
            ret = 0;
        }
        CU_ASSERT_PTR_NULL((result = libdbo_result_list_next(result_list)));
        if (result) {
            libdbo_result_list_free(result_list);
            libdbo_clause_list_free(clause_list);
            return 1;
        }
    }

    libdbo_result_list_free(result_list);
    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

int test2_get_by_id(test2_t* test2, const libdbo_value_t* id) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_result_list_t* result_list;
    const libdbo_result_t* result;
    int ret;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    ret = 1;
    result_list = libdbo_object_read(test2->dbo, NULL, clause_list);
    if (result_list) {
        result = libdbo_result_list_next(result_list);
        if (result) {
            test2_from_result(test2, result);
            ret = 0;
        }
        CU_ASSERT_PTR_NULL((result = libdbo_result_list_next(result_list)));
        if (result) {
            libdbo_result_list_free(result_list);
            libdbo_clause_list_free(clause_list);
            return 1;
        }
    }

    libdbo_result_list_free(result_list);
    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    return ret;
}

int test2_create(test2_t* test2) {
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_value_set_t* value_set;
    libdbo_value_t* value;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_FATAL(libdbo_value_not_empty(test2->id));
    CU_ASSERT_FATAL(libdbo_value_not_empty(test2->rev));
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2->name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));
    object_field = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(1)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((value = libdbo_value_set_get(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_from_text(value, test2->name));

    if (libdbo_object_create(test2->dbo, object_field_list, value_set)) {
        ret = 1;
    }

    libdbo_value_set_free(value_set);
    libdbo_object_field_free(object_field);
    libdbo_object_field_list_free(object_field_list);
    CU_ASSERT(!ret);
    return ret;
}

int test2_update(test2_t* test2) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    libdbo_object_field_list_t* object_field_list;
    libdbo_object_field_t* object_field;
    libdbo_value_set_t* value_set;
    libdbo_value_t* value;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test2->id));
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test2->rev));
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2->name);

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test2->id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "rev"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test2->rev));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field_list = libdbo_object_field_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((object_field = libdbo_object_field_new()));
    CU_ASSERT_FATAL(!libdbo_object_field_set_name(object_field, "name"));
    CU_ASSERT_FATAL(!libdbo_object_field_set_type(object_field, LIBDBO_TYPE_TEXT));
    CU_ASSERT_FATAL(!libdbo_object_field_list_add(object_field_list, object_field));
    object_field = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((value_set = libdbo_value_set_new(1)));
    CU_ASSERT_PTR_NOT_NULL_FATAL((value = libdbo_value_set_get(value_set, 0)));
    CU_ASSERT_FATAL(!libdbo_value_from_text(value, test2->name));

    if (libdbo_object_update(test2->dbo, object_field_list, value_set, clause_list)) {
        ret = 1;
    }

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    libdbo_value_set_free(value_set);
    libdbo_object_field_free(object_field);
    libdbo_object_field_list_free(object_field_list);
    return ret;
}

int test2_delete(test2_t* test2) {
    libdbo_clause_list_t* clause_list;
    libdbo_clause_t* clause;
    int ret = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL(test2);
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test2->id));
    CU_ASSERT_FATAL(!libdbo_value_not_empty(test2->rev));

    CU_ASSERT_PTR_NOT_NULL_FATAL((clause_list = libdbo_clause_list_new()));
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "id"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test2->id));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;
    CU_ASSERT_PTR_NOT_NULL_FATAL((clause = libdbo_clause_new()));
    CU_ASSERT_FATAL(!libdbo_clause_set_field(clause, "rev"));
    CU_ASSERT_FATAL(!libdbo_clause_set_type(clause, LIBDBO_CLAUSE_EQUAL));
    CU_ASSERT_FATAL(!libdbo_value_copy(libdbo_clause_get_value(clause), test2->rev));
    CU_ASSERT_FATAL(!libdbo_clause_list_add(clause_list, clause));
    clause = NULL;

    if (libdbo_object_delete(test2->dbo, clause_list)) {
        ret = 1;
    }

    libdbo_clause_list_free(clause_list);
    libdbo_clause_free(clause);
    CU_ASSERT(!ret);
    return ret;
}

int init_suite_database_operations_sqlite(void) {
#if defined(HAVE_SQLITE3)
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }
    if (test) {
        return 1;
    }
    if (test2) {
        return 1;
    }
    if (test2_2) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = libdbo_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "backend")
        || libdbo_configuration_set_value(configuration, "sqlite")
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "file")
        || libdbo_configuration_set_value(configuration, "test.db")
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = libdbo_connection_new())
        || libdbo_connection_set_configuration_list(connection, configuration_list))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (libdbo_connection_setup(connection)
        || libdbo_connection_connect(connection))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        return 1;
    }

    return 0;
#else
    return 1;
#endif
}

int init_suite_database_operations_couchdb(void) {
#if defined(HAVE_COUCHDB)
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }
    if (test) {
        return 1;
    }
    if (test2) {
        return 1;
    }
    if (test2_2) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = libdbo_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "backend")
        || libdbo_configuration_set_value(configuration, "couchdb")
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "url")
        || libdbo_configuration_set_value(configuration, "http://127.0.0.1:5984/opendnssec")
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = libdbo_connection_new())
        || libdbo_connection_set_configuration_list(connection, configuration_list))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (libdbo_connection_setup(connection)
        || libdbo_connection_connect(connection))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        return 1;
    }

    return 0;
#else
    return 1;
#endif
}

int init_suite_database_operations_mysql(void) {
#if defined(HAVE_MYSQL)
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }
    if (test) {
        return 1;
    }
    if (test2) {
        return 1;
    }
    if (test2_2) {
        return 1;
    }

    /*
     * Setup the configuration for the connection
     */
    if (!(configuration_list = libdbo_configuration_list_new())) {
        return 1;
    }
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "backend")
        || libdbo_configuration_set_value(configuration, "mysql")
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "host")
        || libdbo_configuration_set_value(configuration, TEST_MYSQL_HOST)
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "port")
        || libdbo_configuration_set_value(configuration, TEST_MYSQL_PORT_TXT)
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "user")
        || libdbo_configuration_set_value(configuration, TEST_MYSQL_USER)
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "pass")
        || libdbo_configuration_set_value(configuration, TEST_MYSQL_PASS)
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;
    if (!(configuration = libdbo_configuration_new())
        || libdbo_configuration_set_name(configuration, "db")
        || libdbo_configuration_set_value(configuration, TEST_MYSQL_DB)
        || libdbo_configuration_list_add(configuration_list, configuration))
    {
        libdbo_configuration_free(configuration);
        configuration = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration = NULL;

    /*
     * Connect to the database
     */
    if (!(connection = libdbo_connection_new())
        || libdbo_connection_set_configuration_list(connection, configuration_list))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        libdbo_configuration_list_free(configuration_list);
        configuration_list = NULL;
        return 1;
    }
    configuration_list = NULL;

    if (libdbo_connection_setup(connection)
        || libdbo_connection_connect(connection))
    {
        libdbo_connection_free(connection);
        connection = NULL;
        return 1;
    }

    return 0;
#else
    return 1;
#endif
}

int clean_suite_database_operations(void) {
    test_free(test);
    test = NULL;
    test_list_free(test_list);
    test_list = NULL;
    test2_free(test2);
    test2 = NULL;
    test2_free(test2_2);
    test2_2 = NULL;
    libdbo_connection_free(connection);
    connection = NULL;
    libdbo_configuration_free(configuration);
    configuration = NULL;
    libdbo_configuration_list_free(configuration_list);
    configuration_list = NULL;
    libdbo_value_reset(&object2_id);
    libdbo_value_reset(&object3_id);
    return 0;
}

void __check_id(const libdbo_value_t* id, int id_int, const char* id_text) {
    libdbo_type_int32_t int32;
    libdbo_type_uint32_t uint32;
    libdbo_type_int64_t int64;
    libdbo_type_uint64_t uint64;
    const char* text;

    CU_ASSERT_PTR_NOT_NULL(id);
    switch (libdbo_value_type(id)) {
    case LIBDBO_TYPE_INT32:
        CU_ASSERT(!libdbo_value_to_int32(id, &int32));
        CU_ASSERT(int32 == (libdbo_type_int32_t)id_int);
        break;

    case LIBDBO_TYPE_UINT32:
        CU_ASSERT(!libdbo_value_to_uint32(id, &uint32));
        CU_ASSERT(uint32 == (libdbo_type_uint32_t)id_int);
        break;

    case LIBDBO_TYPE_INT64:
        CU_ASSERT(!libdbo_value_to_int64(id, &int64));
        CU_ASSERT(int64 == (libdbo_type_int64_t)id_int);
        break;

    case LIBDBO_TYPE_UINT64:
        CU_ASSERT(!libdbo_value_to_uint64(id, &uint64));
        CU_ASSERT(uint64 == (libdbo_type_uint64_t)id_int);
        break;

    case LIBDBO_TYPE_TEXT:
        CU_ASSERT_PTR_NOT_NULL_FATAL((text = libdbo_value_text(id)));
        CU_ASSERT(!strcmp(text, id_text));
        break;

    default:
        CU_FAIL("libdbo_value_type(id)");
    }
}

void test_database_operations_read_object1(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_name(test, "test"));
    __check_id(test_id(test), 1, "1");
    CU_ASSERT_PTR_NOT_NULL_FATAL(test_name(test));
    CU_ASSERT(!strcmp(test_name(test), "test"));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_create_object2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_set_name(test, "name 2"));
    CU_ASSERT(!strcmp(test_name(test), "name 2"));
    CU_ASSERT_FATAL(!test_create(test));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_name(test, "name 2"));
    libdbo_value_reset(&object2_id);
    CU_ASSERT(!libdbo_value_copy(&object2_id, test_id(test)));
    CU_ASSERT(!strcmp(test_name(test), "name 2"));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_read_object2(void) {
    int cmp = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_id(test, &object2_id));
    CU_ASSERT(!libdbo_value_cmp(test_id(test), &object2_id, &cmp));
    CU_ASSERT(!cmp);
    CU_ASSERT(!strcmp(test_name(test), "name 2"));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_update_object2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_id(test, &object2_id));
    CU_ASSERT_FATAL(!test_set_name(test, "name 3"));
    CU_ASSERT(!strcmp(test_name(test), "name 3"));
    CU_ASSERT_FATAL(!test_update(test));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_id(test, &object2_id));
    CU_ASSERT(!strcmp(test_name(test), "name 3"));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_delete_object2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_id(test, &object2_id));
    CU_ASSERT_FATAL(!test_delete(test));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(test_get_by_id(test, &object2_id));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_create_object3(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_set_name(test, "name 3"));
    CU_ASSERT(!strcmp(test_name(test), "name 3"));
    CU_ASSERT_FATAL(!test_create(test));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_name(test, "name 3"));
    libdbo_value_reset(&object3_id);
    CU_ASSERT(!libdbo_value_copy(&object3_id, test_id(test)));
    CU_ASSERT(!strcmp(test_name(test), "name 3"));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_delete_object3(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(!test_get_by_id(test, &object3_id));
    CU_ASSERT_FATAL(!test_delete(test));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT_FATAL(test_get_by_id(test, &object3_id));

    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_read_all(void) {
    const test_t* local_test;
    int count = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL((test_list = test_list_new(connection)));
    CU_ASSERT_FATAL(!test_list_get(test_list));
    local_test = test_list_begin(test_list);
    while (local_test) {
        count++;
        local_test = test_list_next(test_list);
    }
    CU_ASSERT(count == 3);

    test_list_free(test_list);
    test_list = NULL;
    CU_PASS("test_list_free");
}

void test_database_operations_count(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test = test_new(connection)));
    CU_ASSERT(test_count_by_name(test, "test") == 1);
    CU_ASSERT(test_count_by_id(test, &object2_id) == 1);
    CU_ASSERT(test_count_by_id(test, &object3_id) == 1);
    CU_ASSERT(test_count_by_name(test, "name 3") == 2);
    test_free(test);
    test = NULL;
    CU_PASS("test_free");
}

void test_database_operations_read_object1_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_name(test2, "test"));
    __check_id(test2_id(test2), 1, "1");
    CU_ASSERT_PTR_NOT_NULL_FATAL(test2_name(test2));
    CU_ASSERT(!strcmp(test2_name(test2), "test"));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_create_object2_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_set_name(test2, "name 2"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 2"));
    CU_ASSERT_FATAL(!test2_create(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_name(test2, "name 2"));
    libdbo_value_reset(&object2_id);
    CU_ASSERT(!libdbo_value_copy(&object2_id, test2_id(test2)));
    CU_ASSERT(!strcmp(test2_name(test2), "name 2"));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_read_object2_2(void) {
    int cmp = 0;

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_id(test2, &object2_id));
    CU_ASSERT(!libdbo_value_cmp(test2_id(test2), &object2_id, &cmp));
    CU_ASSERT(!cmp);
    CU_ASSERT(!strcmp(test2_name(test2), "name 2"));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_update_object2_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_id(test2, &object2_id));
    CU_ASSERT_FATAL(!test2_set_name(test2, "name 3"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 3"));
    CU_ASSERT_FATAL(!test2_update(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_id(test2, &object2_id));
    CU_ASSERT(!strcmp(test2_name(test2), "name 3"));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_update_objects_revisions(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_set_name(test2, "name 4"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 4"));
    CU_ASSERT_FATAL(!test2_create(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_name(test2, "name 4"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 4"));

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2_2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_name(test2_2, "name 4"));
    CU_ASSERT(!strcmp(test2_name(test2_2), "name 4"));

    CU_ASSERT_FATAL(!test2_set_name(test2_2, "name 5"));
    CU_ASSERT(!strcmp(test2_name(test2_2), "name 5"));
    CU_ASSERT_FATAL(!test2_update(test2_2));

    CU_ASSERT_FATAL(!test2_set_name(test2, "name 5"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 5"));
    CU_ASSERT_FATAL(test2_update(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    test2_free(test2_2);
    test2_2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_delete_object2_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_id(test2, &object2_id));
    CU_ASSERT_FATAL(!test2_delete(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(test2_get_by_id(test2, &object2_id));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_create_object3_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_set_name(test2, "name 3"));
    CU_ASSERT(!strcmp(test2_name(test2), "name 3"));
    CU_ASSERT_FATAL(!test2_create(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_name(test2, "name 3"));
    libdbo_value_reset(&object3_id);
    CU_ASSERT(!libdbo_value_copy(&object3_id, test2_id(test2)));
    CU_ASSERT(!strcmp(test2_name(test2), "name 3"));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_delete_object3_2(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(!test2_get_by_id(test2, &object3_id));
    CU_ASSERT_FATAL(!test2_delete(test2));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((test2 = test2_new(connection)));
    CU_ASSERT_FATAL(test2_get_by_id(test2, &object3_id));

    test2_free(test2);
    test2 = NULL;
    CU_PASS("test2_free");
}

void test_database_operations_associated_fetch(void) {
    users_rev_t* user;
    groups_rev_t* group;
    user_group_link_rev_t* user_group;
    user_group_link_rev_list_t* user_group_list;

    users_rev_list_t* user_list;
    const users_rev_t* user2;

    CU_ASSERT_PTR_NOT_NULL_FATAL((group = groups_rev_new(connection)));
    CU_ASSERT(!groups_rev_set_name(group, "group 1"));
    CU_ASSERT_FATAL(!groups_rev_create(group));
    groups_rev_free(group);
    CU_PASS("groups_rev_free");
    CU_ASSERT_PTR_NOT_NULL_FATAL((group = groups_rev_new_get_by_name(connection, "group 1")));

    CU_ASSERT_PTR_NOT_NULL_FATAL((user = users_rev_new(connection)));
    CU_ASSERT(!users_rev_set_name(user, "user 1"));
    CU_ASSERT(!users_rev_set_group_id(user, groups_rev_id(group)));
    CU_ASSERT_FATAL(!users_rev_create(user));
    users_rev_free(user);
    CU_PASS("users_rev_free");
    CU_ASSERT_PTR_NOT_NULL_FATAL((user = users_rev_new_get_by_name(connection, "user 1")));

    CU_ASSERT_PTR_NOT_NULL_FATAL((user_group = user_group_link_rev_new(connection)));
    CU_ASSERT(!user_group_link_rev_set_user_id(user_group, users_rev_id(user)));
    CU_ASSERT(!user_group_link_rev_set_group_id(user_group, groups_rev_id(group)));
    CU_ASSERT_FATAL(!user_group_link_rev_create(user_group));
    user_group_link_rev_free(user_group);
    CU_PASS("user_group_link_rev_free");
    CU_ASSERT_PTR_NOT_NULL_FATAL((user_group_list = user_group_link_rev_list_new_get_by_user_id(connection, users_rev_id(user))));
    CU_ASSERT_PTR_NOT_NULL_FATAL((user_group = user_group_link_rev_list_get_begin(user_group_list)));
    user_group_link_rev_list_free(user_group_list);
    CU_PASS("user_group_link_rev_list_free");

    CU_ASSERT_PTR_NOT_NULL_FATAL((user_list = users_rev_list_new(connection)));
    CU_ASSERT(!users_rev_list_associated_fetch(user_list));
    CU_ASSERT(!users_rev_list_get_by_group_id(user_list, groups_rev_id(group)));
    CU_ASSERT_PTR_NOT_NULL((user2 = users_rev_list_begin(user_list)));
    while (user2) {
        CU_ASSERT_PTR_NOT_NULL(users_rev_group(user2));

        user2 = users_rev_list_next(user_list);
    }
    users_rev_list_free(user_list);
    CU_PASS("users_rev_list_free");

    /* TODO: test groups association */

    /* TODO: test user_group_link association */

    CU_ASSERT(!user_group_link_rev_delete(user_group));
    user_group_link_rev_free(user_group);
    CU_PASS("user_group_link_rev_free");

    CU_ASSERT(!users_rev_delete(user));
    users_rev_free(user);
    CU_PASS("users_rev_free");

    CU_ASSERT(!groups_rev_delete(group));
    groups_rev_free(group);
    CU_PASS("groups_rev_free");
}
