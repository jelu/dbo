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
 * Based on enforcer-ng/src/db/db_connection.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/connection.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <stdlib.h>

static libdbo_mm_t __connection_alloc = LIBDBO_MM_T_STATIC_NEW(sizeof(libdbo_connection_t));

libdbo_connection_t* libdbo_connection_new(void) {
    libdbo_connection_t* connection =
        (libdbo_connection_t*)libdbo_mm_new0(&__connection_alloc);

    return connection;
}

void libdbo_connection_free(libdbo_connection_t* connection) {
    if (connection) {
        if (connection->backend) {
            libdbo_backend_free(connection->backend);
        }
        libdbo_mm_delete(&__connection_alloc, connection);
    }
}

int libdbo_connection_set_configuration_list(libdbo_connection_t* connection, const libdbo_configuration_list_t* configuration_list) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (connection->configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    connection->configuration_list = configuration_list;
    return LIBDBO_OK;
}

int libdbo_connection_setup(libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    if (!connection->backend) {
        const libdbo_configuration_t* backend = libdbo_configuration_list_find(connection->configuration_list, "backend");
        if (!backend) {
            return LIBDBO_ERROR_UNKNOWN;
        }

        connection->backend = libdbo_backend_factory_get_backend(libdbo_configuration_value(backend));
        if (!connection->backend) {
            return LIBDBO_ERROR_UNKNOWN;
        }
    }
    return LIBDBO_OK;
}

int libdbo_connection_connect(const libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->configuration_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_connect(connection->backend, connection->configuration_list);
}

int libdbo_connection_disconnect(const libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_disconnect(connection->backend);
}

int libdbo_connection_create(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_create(connection->backend, object, object_field_list, value_set);
}

libdbo_result_list_t* libdbo_connection_read(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list) {
    if (!connection) {
        return NULL;
    }
    if (!object) {
        return NULL;
    }
    if (!connection->backend) {
        return NULL;
    }

    return libdbo_backend_read(connection->backend, object, join_list, clause_list);
}

int libdbo_connection_update(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_object_field_list_t* object_field_list, const libdbo_value_set_t* value_set, const libdbo_clause_list_t* clause_list) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object_field_list) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!value_set) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_update(connection->backend, object, object_field_list, value_set, clause_list);
}

int libdbo_connection_delete(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_clause_list_t* clause_list) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_delete(connection->backend, object, clause_list);
}

int libdbo_connection_count(const libdbo_connection_t* connection, const libdbo_object_t* object, const libdbo_join_list_t* join_list, const libdbo_clause_list_t* clause_list, size_t* count) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!object) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!count) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_count(connection->backend, object, join_list, clause_list, count);
}

int libdbo_connection_transaction_begin(const libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_transaction_begin(connection->backend);
}

int libdbo_connection_transaction_commit(const libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_transaction_commit(connection->backend);
}

int libdbo_connection_transaction_rollback(const libdbo_connection_t* connection) {
    if (!connection) {
        return LIBDBO_ERROR_UNKNOWN;
    }
    if (!connection->backend) {
        return LIBDBO_ERROR_UNKNOWN;
    }

    return libdbo_backend_transaction_rollback(connection->backend);
}
