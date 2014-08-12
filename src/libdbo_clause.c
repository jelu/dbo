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
 * Based on enforcer-ng/src/db/db_clause.c source file from the OpenDNSSEC
 * project.
 *
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 */

#include "libdbo/clause.h"
#include "libdbo/error.h"

#include "libdbo/mm.h"

#include <stdlib.h>
#include <string.h>

/* DB CLAUSE */

static libdbo_mm_t __clause_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_clause_t));

/* TODO: add more check for type and what value/list is set, maybe add type to new */

libdbo_clause_t* libdbo_clause_new(void) {
    libdbo_clause_t* clause =
        (libdbo_clause_t*)libdbo_mm_new0(&__clause_alloc);

    if (clause) {
        clause->type = DB_CLAUSE_UNKNOWN;
        clause->clause_operator = DB_CLAUSE_OPERATOR_AND;
        libdbo_value_reset(&(clause->value));
    }

    return clause;
}

void libdbo_clause_free(libdbo_clause_t* clause) {
    if (clause) {
        if (clause->table) {
            free(clause->table);
        }
        if (clause->field) {
            free(clause->field);
        }
        libdbo_value_reset(&(clause->value));
        if (clause->clause_list) {
            libdbo_clause_list_free(clause->clause_list);
        }
        libdbo_mm_delete(&__clause_alloc, clause);
    }
}

const char* libdbo_clause_table(const libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }

    return clause->table;
}

const char* libdbo_clause_field(const libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }

    return clause->field;
}

libdbo_clause_type_t libdbo_clause_type(const libdbo_clause_t* clause) {
    if (!clause) {
        return DB_CLAUSE_UNKNOWN;
    }

    return clause->type;
}

const libdbo_value_t* libdbo_clause_value(const libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }

    return &(clause->value);
}

libdbo_clause_operator_t libdbo_clause_operator(const libdbo_clause_t* clause) {
    if (!clause) {
        return DB_CLAUSE_OPERATOR_UNKNOWN;
    }

    return clause->clause_operator;
}

const libdbo_clause_list_t* libdbo_clause_list(const libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }

    return clause->clause_list;
}

int libdbo_clause_set_table(libdbo_clause_t* clause, const char* table) {
    char* new_table;

    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->clause_list) {
        return DB_ERROR_UNKNOWN;
    }

    if (!(new_table = strdup(table))) {
        return DB_ERROR_UNKNOWN;
    }

    if (clause->table) {
        free(clause->table);
    }
    clause->table = new_table;
    return DB_OK;
}

int libdbo_clause_set_field(libdbo_clause_t* clause, const char* field) {
    char* new_field;

    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->clause_list) {
        return DB_ERROR_UNKNOWN;
    }

    if (!(new_field = strdup(field))) {
        return DB_ERROR_UNKNOWN;
    }

    if (clause->field) {
        free(clause->field);
    }
    clause->field = new_field;
    return DB_OK;
}

int libdbo_clause_set_type(libdbo_clause_t* clause, libdbo_clause_type_t type) {
    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (type == DB_CLAUSE_UNKNOWN) {
        return DB_ERROR_UNKNOWN;
    }

    clause->type = type;
    return DB_OK;
}

int libdbo_clause_set_operator(libdbo_clause_t* clause, libdbo_clause_operator_t clause_operator) {
    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause_operator == DB_CLAUSE_OPERATOR_UNKNOWN) {
        return DB_ERROR_UNKNOWN;
    }

    clause->clause_operator = clause_operator;
    return DB_OK;
}

int libdbo_clause_set_list(libdbo_clause_t* clause, libdbo_clause_list_t* clause_list) {
    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->table) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->field) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->clause_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (libdbo_value_type(&(clause->value)) != DB_TYPE_EMPTY) {
        return DB_ERROR_UNKNOWN;
    }

    clause->clause_list = clause_list;
    return DB_OK;
}

int libdbo_clause_not_empty(const libdbo_clause_t* clause) {
    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->type == DB_CLAUSE_UNKNOWN) {
        return DB_ERROR_UNKNOWN;
    }

    if (clause->type == DB_CLAUSE_NESTED) {
        if (!clause->clause_list) {
            return DB_ERROR_UNKNOWN;
        }
    }
    else {
        if (!clause->field) {
            return DB_ERROR_UNKNOWN;
        }
        if (libdbo_value_type(&(clause->value)) == DB_TYPE_EMPTY) {
            return DB_ERROR_UNKNOWN;
        }
    }

    return DB_OK;
}

const libdbo_clause_t* libdbo_clause_next(const libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }

    return clause->next;
}

libdbo_value_t* libdbo_clause_get_value(libdbo_clause_t* clause) {
    if (!clause) {
        return NULL;
    }
    if (clause->clause_list) {
        return NULL;
    }

    return &(clause->value);
}

/* DB CLAUSE LIST */

static libdbo_mm_t __clause_list_alloc = DB_MM_T_STATIC_NEW(sizeof(libdbo_clause_list_t));

libdbo_clause_list_t* libdbo_clause_list_new(void) {
    libdbo_clause_list_t* clause_list =
        (libdbo_clause_list_t*)libdbo_mm_new0(&__clause_list_alloc);

    return clause_list;
}

void libdbo_clause_list_free(libdbo_clause_list_t* clause_list) {
    if (clause_list) {
        if (clause_list->begin) {
            libdbo_clause_t* this = clause_list->begin;
            libdbo_clause_t* next = NULL;

            while (this) {
                next = this->next;
                libdbo_clause_free(this);
                this = next;
            }
        }
        libdbo_mm_delete(&__clause_list_alloc, clause_list);
    }
}

int libdbo_clause_list_add(libdbo_clause_list_t* clause_list, libdbo_clause_t* clause) {
    if (!clause_list) {
        return DB_ERROR_UNKNOWN;
    }
    if (!clause) {
        return DB_ERROR_UNKNOWN;
    }
    if (libdbo_clause_not_empty(clause)) {
        return DB_ERROR_UNKNOWN;
    }
    if (clause->next) {
        return DB_ERROR_UNKNOWN;
    }

    if (clause_list->begin) {
        if (!clause_list->end) {
            return DB_ERROR_UNKNOWN;
        }
        clause_list->end->next = clause;
        clause_list->end = clause;
    }
    else {
        clause_list->begin = clause;
        clause_list->end = clause;
    }

    return DB_OK;
}

const libdbo_clause_t* libdbo_clause_list_begin(const libdbo_clause_list_t* clause_list) {
    if (!clause_list) {
        return NULL;
    }

    return clause_list->begin;
}
