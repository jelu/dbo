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
 * Based on enforcer-ng/src/db/test/test_initialization.c source file from the
 * OpenDNSSEC project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "config.h"

#include <libdbo/configuration.h>
#include <libdbo/connection.h>

#include "CUnit/Basic.h"

static libdbo_configuration_list_t* configuration_list = NULL;
static libdbo_configuration_t* configuration = NULL;
static libdbo_connection_t* connection = NULL;

int init_suite_initialization(void) {
    if (configuration_list) {
        return 1;
    }
    if (configuration) {
        return 1;
    }
    if (connection) {
        return 1;
    }
    return 0;
}

int clean_suite_initialization(void) {
    libdbo_connection_free(connection);
    connection = NULL;
    libdbo_configuration_free(configuration);
    configuration = NULL;
    libdbo_configuration_list_free(configuration_list);
    configuration_list = NULL;
    return 0;
}

void test_initialization_configuration_sqlite3(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration_list = libdbo_configuration_list_new()));

#if defined(HAVE_SQLITE3)
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "backend"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, "sqlite"));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "file"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, "test.db"));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;
#endif
}

void test_initialization_configuration_couchdb(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration_list = libdbo_configuration_list_new()));

#if defined(HAVE_COUCHDB)
    CU_ASSERT_FATAL(0);
#endif
}

void test_initialization_configuration_mysql(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration_list = libdbo_configuration_list_new()));

#if defined(HAVE_MYSQL)
    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "backend"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, "mysql"));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "host"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, TEST_MYSQL_HOST));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "port"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, TEST_MYSQL_PORT_TXT));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "user"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, TEST_MYSQL_USER));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "pass"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, TEST_MYSQL_PASS));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL((configuration = libdbo_configuration_new()));
    CU_ASSERT_FATAL(!libdbo_configuration_set_name(configuration, "db"));
    CU_ASSERT_FATAL(!libdbo_configuration_set_value(configuration, TEST_MYSQL_DB));
    CU_ASSERT_FATAL(!libdbo_configuration_list_add(configuration_list, configuration));
    configuration = NULL;
#endif
}

void test_initialization_connection(void) {
    CU_ASSERT_PTR_NOT_NULL_FATAL((connection = libdbo_connection_new()));
    CU_ASSERT_FATAL(!libdbo_connection_set_configuration_list(connection, configuration_list));
    CU_ASSERT_FATAL(!libdbo_connection_setup(connection));
    CU_ASSERT_FATAL(!libdbo_connection_connect(connection));

    CU_ASSERT_FATAL(!libdbo_connection_disconnect(connection));
}
