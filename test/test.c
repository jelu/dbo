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
 * Based on enforcer-ng/src/db/test/test.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "config.h"

#include "test.h"

#include "test_groups.h"
#include "test_groups_rev.h"
#include "test_user_group_link.h"
#include "test_user_group_link_rev.h"
#include "test_users.h"
#include "test_users_rev.h"

#include "CUnit/Basic.h"

#include <libdbo/backend.h>
#include <libdbo/mm.h>

void test_libdbo_backend_factory_shutdown(void) {
    CU_ASSERT_FATAL(!libdbo_backend_factory_shutdown());
}

int main(void) {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    pSuite = CU_add_suite("MM", init_suite_classes, clean_suite_classes);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of libdbo_mm_init", test_libdbo_mm_init)
        || !CU_add_test(pSuite, "test of libdbo_mm", test_libdbo_mm))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("Classes", init_suite_classes, clean_suite_classes);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of libdbo_backend_handle", test_class_libdbo_backend_handle)
        || !CU_add_test(pSuite, "test of libdbo_backend", test_class_libdbo_backend)
        || !CU_add_test(pSuite, "test of libdbo_backend_meta_data", test_class_libdbo_backend_meta_data)
        || !CU_add_test(pSuite, "test of libdbo_backend_meta_data_list", test_class_libdbo_backend_meta_data_list)
        || !CU_add_test(pSuite, "test of libdbo_clause", test_class_libdbo_clause)
        || !CU_add_test(pSuite, "test of libdbo_clause_list", test_class_libdbo_clause_list)
        || !CU_add_test(pSuite, "test of libdbo_configuration", test_class_libdbo_configuration)
        || !CU_add_test(pSuite, "test of libdbo_configuration_list", test_class_libdbo_configuration_list)
        || !CU_add_test(pSuite, "test of libdbo_connection", test_class_libdbo_connection)
        || !CU_add_test(pSuite, "test of libdbo_join", test_class_libdbo_join)
        || !CU_add_test(pSuite, "test of libdbo_join_list", test_class_libdbo_join_list)
        || !CU_add_test(pSuite, "test of libdbo_object_field", test_class_libdbo_object_field)
        || !CU_add_test(pSuite, "test of libdbo_object_field_list", test_class_libdbo_object_field_list)
        || !CU_add_test(pSuite, "test of libdbo_object", test_class_libdbo_object)
        || !CU_add_test(pSuite, "test of libdbo_value_set", test_class_libdbo_value_set)
        || !CU_add_test(pSuite, "test of libdbo_result", test_class_libdbo_result)
        || !CU_add_test(pSuite, "test of libdbo_result_list", test_class_libdbo_result_list)
        || !CU_add_test(pSuite, "test of libdbo_value", test_class_libdbo_value)
        || !CU_add_test(pSuite, "test of libdbo_*_free", test_class_end))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("Classes (short names)", init_suite_classes_short_names, clean_suite_classes_short_names);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of db_backend_handle", test_class_short_names_db_backend_handle)
        || !CU_add_test(pSuite, "test of db_backend", test_class_short_names_db_backend)
        || !CU_add_test(pSuite, "test of db_backend_meta_data", test_class_short_names_db_backend_meta_data)
        || !CU_add_test(pSuite, "test of db_backend_meta_data_list", test_class_short_names_db_backend_meta_data_list)
        || !CU_add_test(pSuite, "test of db_clause", test_class_short_names_db_clause)
        || !CU_add_test(pSuite, "test of db_clause_list", test_class_short_names_db_clause_list)
        || !CU_add_test(pSuite, "test of db_configuration", test_class_short_names_db_configuration)
        || !CU_add_test(pSuite, "test of db_configuration_list", test_class_short_names_db_configuration_list)
        || !CU_add_test(pSuite, "test of db_connection", test_class_short_names_db_connection)
        || !CU_add_test(pSuite, "test of db_join", test_class_short_names_db_join)
        || !CU_add_test(pSuite, "test of db_join_list", test_class_short_names_db_join_list)
        || !CU_add_test(pSuite, "test of db_object_field", test_class_short_names_db_object_field)
        || !CU_add_test(pSuite, "test of db_object_field_list", test_class_short_names_db_object_field_list)
        || !CU_add_test(pSuite, "test of db_object", test_class_short_names_db_object)
        || !CU_add_test(pSuite, "test of db_value_set", test_class_short_names_db_value_set)
        || !CU_add_test(pSuite, "test of db_result", test_class_short_names_db_result)
        || !CU_add_test(pSuite, "test of db_result_list", test_class_short_names_db_result_list)
        || !CU_add_test(pSuite, "test of db_value", test_class_short_names_db_value)
        || !CU_add_test(pSuite, "test of db_*_free", test_class_short_names_end))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

#if defined(HAVE_SQLITE3)
    pSuite = CU_add_suite("Initialization SQLite3", init_suite_initialization, clean_suite_initialization);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of configuration", test_initialization_configuration_sqlite3)
        || !CU_add_test(pSuite, "test of connection", test_initialization_connection))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif
#if defined(HAVE_COUCHDB)
    pSuite = CU_add_suite("Initialization CouchDB", init_suite_initialization, clean_suite_initialization);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of configuration", test_initialization_configuration_couchdb)
        || !CU_add_test(pSuite, "test of connection", test_initialization_connection))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif
#if defined(TEST_MYSQL)
    pSuite = CU_add_suite("Initialization MySQL", init_suite_initialization, clean_suite_initialization);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of configuration", test_initialization_configuration_mysql)
        || !CU_add_test(pSuite, "test of connection", test_initialization_connection))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif

#if defined(HAVE_SQLITE3)
    pSuite = CU_add_suite("SQLite database operations", init_suite_database_operations_sqlite, clean_suite_database_operations);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of read object 1", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 2", test_database_operations_create_object2)
        || !CU_add_test(pSuite, "test of read object 2", test_database_operations_read_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#2)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 3", test_database_operations_create_object3)
        || !CU_add_test(pSuite, "test of update object 2", test_database_operations_update_object2)
        || !CU_add_test(pSuite, "test of read all", test_database_operations_read_all)
        || !CU_add_test(pSuite, "test of count", test_database_operations_count)
        || !CU_add_test(pSuite, "test of delete object 3", test_database_operations_delete_object3)
        || !CU_add_test(pSuite, "test of read object 1 (#3)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of delete object 2", test_database_operations_delete_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#4)", test_database_operations_read_object1)

        || !CU_add_test(pSuite, "test of read object 1 (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 2 (REV)", test_database_operations_create_object2_2)
        || !CU_add_test(pSuite, "test of read object 2 (REV)", test_database_operations_read_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#2) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 3 (REV)", test_database_operations_create_object3_2)
        || !CU_add_test(pSuite, "test of update object 2 (REV)", test_database_operations_update_object2_2)
        || !CU_add_test(pSuite, "test of updates revisions (REV)", test_database_operations_update_objects_revisions)
        || !CU_add_test(pSuite, "test of delete object 3 (REV)", test_database_operations_delete_object3_2)
        || !CU_add_test(pSuite, "test of read object 1 (#3) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of delete object 2 (REV)", test_database_operations_delete_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#4) (REV)", test_database_operations_read_object1_2)

        || !CU_add_test(pSuite, "test of associated fetch", test_database_operations_associated_fetch))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif

#if defined(HAVE_COUCHDB)
    pSuite = CU_add_suite("CouchDB database operations", init_suite_database_operations_couchdb, clean_suite_database_operations);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of read object 1", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 2", test_database_operations_create_object2)
        || !CU_add_test(pSuite, "test of read object 2", test_database_operations_read_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#2)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 3", test_database_operations_create_object3)
        || !CU_add_test(pSuite, "test of update object 2", test_database_operations_update_object2)
        || !CU_add_test(pSuite, "test of read all", test_database_operations_read_all)
        || !CU_add_test(pSuite, "test of delete object 3", test_database_operations_delete_object3)
        || !CU_add_test(pSuite, "test of read object 1 (#3)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of delete object 2", test_database_operations_delete_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#4)", test_database_operations_read_object1)

        || !CU_add_test(pSuite, "test of read object 1 (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 2 (REV)", test_database_operations_create_object2_2)
        || !CU_add_test(pSuite, "test of read object 2 (REV)", test_database_operations_read_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#2) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 3 (REV)", test_database_operations_create_object3_2)
        || !CU_add_test(pSuite, "test of update object 2 (REV)", test_database_operations_update_object2_2)
        || !CU_add_test(pSuite, "test of updates revisions (REV)", test_database_operations_update_objects_revisions)
        || !CU_add_test(pSuite, "test of delete object 3 (REV)", test_database_operations_delete_object3_2)
        || !CU_add_test(pSuite, "test of read object 1 (#3) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of delete object 2 (REV)", test_database_operations_delete_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#4) (REV)", test_database_operations_read_object1_2)

        || !CU_add_test(pSuite, "test of associated fetch", test_database_operations_associated_fetch))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif

#if defined(TEST_MYSQL)
    pSuite = CU_add_suite("MySQL database operations", init_suite_database_operations_mysql, clean_suite_database_operations);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of read object 1", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 2", test_database_operations_create_object2)
        || !CU_add_test(pSuite, "test of read object 2", test_database_operations_read_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#2)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of create object 3", test_database_operations_create_object3)
        || !CU_add_test(pSuite, "test of update object 2", test_database_operations_update_object2)
        || !CU_add_test(pSuite, "test of read all", test_database_operations_read_all)
        || !CU_add_test(pSuite, "test of delete object 3", test_database_operations_delete_object3)
        || !CU_add_test(pSuite, "test of read object 1 (#3)", test_database_operations_read_object1)
        || !CU_add_test(pSuite, "test of delete object 2", test_database_operations_delete_object2)
        || !CU_add_test(pSuite, "test of read object 1 (#4)", test_database_operations_read_object1)

        || !CU_add_test(pSuite, "test of read object 1 (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 2 (REV)", test_database_operations_create_object2_2)
        || !CU_add_test(pSuite, "test of read object 2 (REV)", test_database_operations_read_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#2) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of create object 3 (REV)", test_database_operations_create_object3_2)
        || !CU_add_test(pSuite, "test of update object 2 (REV)", test_database_operations_update_object2_2)
        || !CU_add_test(pSuite, "test of updates revisions (REV)", test_database_operations_update_objects_revisions)
        || !CU_add_test(pSuite, "test of delete object 3 (REV)", test_database_operations_delete_object3_2)
        || !CU_add_test(pSuite, "test of read object 1 (#3) (REV)", test_database_operations_read_object1_2)
        || !CU_add_test(pSuite, "test of delete object 2 (REV)", test_database_operations_delete_object2_2)
        || !CU_add_test(pSuite, "test of read object 1 (#4) (REV)", test_database_operations_read_object1_2)

        || !CU_add_test(pSuite, "test of associated fetch", test_database_operations_associated_fetch))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif

    test_users_add_suite();
    test_groups_add_suite();
    test_user_group_link_add_suite();
    test_users_rev_add_suite();
    test_groups_rev_add_suite();
    test_user_group_link_rev_add_suite();

    pSuite = CU_add_suite("Shutdown", init_suite_classes, clean_suite_classes);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of libdbo_backend_factory_shutdown", test_libdbo_backend_factory_shutdown)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("MM extern", init_suite_classes, clean_suite_classes);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of libdbo_mm usage with external malloc/free", test_libdbo_mm_extern)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    pSuite = CU_add_suite("Classes (w/o MM)", init_suite_classes, clean_suite_classes);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite, "test of libdbo_backend_handle", test_class_libdbo_backend_handle)
        || !CU_add_test(pSuite, "test of libdbo_backend", test_class_libdbo_backend)
        || !CU_add_test(pSuite, "test of libdbo_backend_meta_data", test_class_libdbo_backend_meta_data)
        || !CU_add_test(pSuite, "test of libdbo_backend_meta_data_list", test_class_libdbo_backend_meta_data_list)
        || !CU_add_test(pSuite, "test of libdbo_clause", test_class_libdbo_clause)
        || !CU_add_test(pSuite, "test of libdbo_clause_list", test_class_libdbo_clause_list)
        || !CU_add_test(pSuite, "test of libdbo_configuration", test_class_libdbo_configuration)
        || !CU_add_test(pSuite, "test of libdbo_configuration_list", test_class_libdbo_configuration_list)
        || !CU_add_test(pSuite, "test of libdbo_connection", test_class_libdbo_connection)
        || !CU_add_test(pSuite, "test of libdbo_join", test_class_libdbo_join)
        || !CU_add_test(pSuite, "test of libdbo_join_list", test_class_libdbo_join_list)
        || !CU_add_test(pSuite, "test of libdbo_object_field", test_class_libdbo_object_field)
        || !CU_add_test(pSuite, "test of libdbo_object_field_list", test_class_libdbo_object_field_list)
        || !CU_add_test(pSuite, "test of libdbo_object", test_class_libdbo_object)
        || !CU_add_test(pSuite, "test of libdbo_value_set", test_class_libdbo_value_set)
        || !CU_add_test(pSuite, "test of libdbo_result", test_class_libdbo_result)
        || !CU_add_test(pSuite, "test of libdbo_result_list", test_class_libdbo_result_list)
        || !CU_add_test(pSuite, "test of libdbo_value", test_class_libdbo_value)
        || !CU_add_test(pSuite, "test of libdbo_*_free", test_class_end))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
