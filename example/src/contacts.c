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

#include "person.h"
#include "folder.h"

#include <libdbo/libdbo.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void usage(void) {
    fprintf(stderr, "Usage: example [command and arguments...]\n"
        "\n"
        "Commands:\n"
        "  person create <name> <birth year> <address> <phone> <email>\n"
        "  person put <name> <folder>\n"
        "  person list\n"
        "  person update <name> <field> <new value>\n"
        "    fields on update:\n"
        "      - name\n"
        "      - birth year\n"
        "      - address\n"
        "      - phone\n"
        "      - email\n"
        "  person delete <name>\n"
        "  folder create <name>\n"
        "  folder list [name]\n"
        "  folder delete <name>\n"
        );
}

int contacts_person_create(libdbo_connection_t* connection, const char* name,
    const char* birth_year, const char* address, const char* phone,
    const char* email)
{
    person_t* person;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }
    if (!birth_year) {
        return -1;
    }
    if (!address) {
        return -1;
    }
    if (!phone) {
        return -1;
    }
    if (!email) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(person = person_new(connection))
        || person_set_name(person, name)
        || person_set_birth_year(person, atoi(birth_year))
        || person_set_address(person, address)
        || person_set_phone(person, phone)
        || person_set_email(person, email)
        || person_create(person))
    {
        person_free(person);
        fprintf(stderr, "unable to create person!\n");
        return -6;
    }
    person_free(person);

    printf("Person created successfully.\n");

    return 0;
}

int contacts_person_put(libdbo_connection_t* connection, const char* name,
    const char* folder)
{
    person_t* person;
    folder_t* folder_object;
    folder_person_t* folder_person;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }
    if (!folder) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(person = person_new_get_by_name(connection, name))) {
        fprintf(stderr, "unable to find person!\n");
        return -6;
    }

    if (!(folder_object = folder_new_get_by_name(connection, folder))) {
        person_free(person);
        fprintf(stderr, "unable to find folder!\n");
        return -6;
    }

    if (!(folder_person = folder_person_new(connection))
        || folder_person_set_person_id(folder_person, person_id(person))
        || folder_person_set_folder_id(folder_person, folder_id(folder_object))
        || folder_person_create(folder_person))
    {
        folder_free(folder_object);
        person_free(person);
        fprintf(stderr, "unable to put person in folder!\n");
        return -6;
    }
    folder_free(folder_object);
    person_free(person);

    printf("Person has been put into folder successfully.\n");

    return 0;
}

int contacts_person_list(libdbo_connection_t* connection) {
    person_list_t* person_list;
    const person_t* person;

    if (!connection) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(person_list = person_list_new_get(connection))) {
        fprintf(stderr, "unable to list persons!\n");
        return -6;
    }

    person = person_list_next(person_list);
    if (person) {
        printf("Name\tBirth Year\tAddress\tPhone\tEmail\n");

        while (person) {
            printf("%s\t%u\t%s\t%s\t%s\n",
                person_name(person),
                person_birth_year(person),
                person_address(person),
                person_phone(person),
                person_email(person));

            person = person_list_next(person_list);
        }
    }
    else {
        printf("No persons found in database.\n");
    }
    person_list_free(person_list);

    return 0;
}

int contacts_person_update(libdbo_connection_t* connection, const char* name,
    const char* field, const char* new_value)
{
    person_t* person;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }
    if (!field) {
        return -1;
    }
    if (!new_value) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(person = person_new_get_by_name(connection, name))) {
        fprintf(stderr, "unable to find person!\n");
        return -6;
    }

    if (!strcmp(field, "name")) {
        if (person_set_name(person, new_value)) {
            person_free(person);
            fprintf(stderr, "unable to set field!\n");
            return -6;
        }
    }
    else if (!strcmp(field, "birth year")) {
        if (person_set_birth_year(person, atoi(new_value))) {
            person_free(person);
            fprintf(stderr, "unable to set field!\n");
            return -6;
        }
    }
    else if (!strcmp(field, "address")) {
        if (person_set_address(person, new_value)) {
            person_free(person);
            fprintf(stderr, "unable to set field!\n");
            return -6;
        }
    }
    else if (!strcmp(field, "phone")) {
        if (person_set_phone(person, new_value)) {
            person_free(person);
            fprintf(stderr, "unable to set field!\n");
            return -6;
        }
    }
    else if (!strcmp(field, "email")) {
        if (person_set_email(person, new_value)) {
            person_free(person);
            fprintf(stderr, "unable to set field!\n");
            return -6;
        }
    }
    else {
        person_free(person);
        fprintf(stderr, "invalid field!\n");
        return -6;
    }

    if (person_update(person)) {
        person_free(person);
        fprintf(stderr, "unable to update person!\n");
        return -6;
    }
    person_free(person);

    printf("Person updated successfully.\n");

    return 0;
}

int contacts_person_delete(libdbo_connection_t* connection, const char* name) {
    person_t* person;
    folder_person_list_t* folder_person_list;
    folder_person_t* folder_person;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(person = person_new_get_by_name(connection, name))) {
        fprintf(stderr, "unable to find person!\n");
        return -6;
    }

    if (!(folder_person_list = folder_person_list_new_get_by_person_id(connection, person_id(person)))) {
        person_free(person);
        fprintf(stderr, "unable to get list of folders that person is in!\n");
        return -6;
    }

    for (folder_person = folder_person_list_get_next(folder_person_list);
        folder_person;
        folder_person_free(folder_person), folder_person = folder_person_list_get_next(folder_person_list))
    {
        if (folder_person_delete(folder_person)) {
            folder_person_free(folder_person);
            folder_person_list_free(folder_person_list);
            person_free(person);
            fprintf(stderr, "unable to remove person from a folder!\n");
            return -6;
        }
    }
    folder_person_list_free(folder_person_list);

    if (person_delete(person)) {
        person_free(person);
        fprintf(stderr, "unable to delete person!\n");
        return -6;
    }
    person_free(person);

    printf("Person deleted successfully.\n");

    return 0;
}

int contacts_folder_create(libdbo_connection_t* connection, const char* name) {
    folder_t* folder;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(folder = folder_new(connection))
        || folder_set_name(folder, name)
        || folder_create(folder))
    {
        folder_free(folder);
        fprintf(stderr, "unable to create folder!\n");
        return -6;
    }
    folder_free(folder);

    printf("Folder created successfully.\n");

    return 0;
}

int contacts_folder_list(libdbo_connection_t* connection, const char* name) {

    if (!connection) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!name) {
        folder_list_t* folder_list;
        const folder_t* folder;

        if (!(folder_list = folder_list_new_get(connection))) {
            fprintf(stderr, "unable to list folders!\n");
            return -6;
        }

        folder = folder_list_next(folder_list);
        if (folder) {
            printf("Name\n");

            while (folder) {
                printf("%s\n", folder_name(folder));

                folder = folder_list_next(folder_list);
            }
        }
        else {
            printf("No folders found in database.\n");
        }
        folder_list_free(folder_list);
    }
    else {
        folder_person_list_t* folder_person_list;
        const folder_person_t* folder_person;
        folder_t* folder;

        if (!(folder = folder_new_get_by_name(connection, name))) {
            fprintf(stderr, "unable to find folder!\n");
            return -6;
        }

        if (!(folder_person_list = folder_person_list_new(connection))
            || folder_person_list_associated_fetch(folder_person_list)
            || folder_person_list_get_by_folder_id(folder_person_list, folder_id(folder)))
        {
            folder_free(folder);
            fprintf(stderr, "unable to get persons in folder!\n");
            return -6;
        }

        folder_person = folder_person_list_next(folder_person_list);
        if (folder_person) {
            printf("Folder %s has the following persons:\n", folder_name(folder));

            while (folder_person) {
                printf("  %s\n", person_name(folder_person_person(folder_person)));

                folder_person = folder_person_list_next(folder_person_list);
            }
        }
        else {
            printf("No persons in that folder.\n");
        }
        folder_person_list_free(folder_person_list);
        folder_free(folder);
    }

    return 0;
}

int contacts_folder_delete(libdbo_connection_t* connection, const char* name) {
    folder_t* folder;
    folder_person_list_t* folder_person_list;
    folder_person_t* folder_person;

    if (!connection) {
        return -1;
    }
    if (!name) {
        return -1;
    }

    if (libdbo_connection_connect(connection)) {
        fprintf(stderr, "unable to connect to database!\n");
        return -5;
    }

    if (!(folder = folder_new_get_by_name(connection, name))) {
        fprintf(stderr, "unable to find folder!\n");
        return -6;
    }

    if (!(folder_person_list = folder_person_list_new_get_by_folder_id(connection, folder_id(folder)))) {
        folder_free(folder);
        fprintf(stderr, "unable to get list of persons in the folder!\n");
        return -6;
    }

    for (folder_person = folder_person_list_get_next(folder_person_list);
        folder_person;
        folder_person_free(folder_person), folder_person = folder_person_list_get_next(folder_person_list))
    {
        if (folder_person_delete(folder_person)) {
            folder_person_free(folder_person);
            folder_person_list_free(folder_person_list);
            folder_free(folder);
            fprintf(stderr, "unable to remove person from a folder!\n");
            return -6;
        }
    }
    folder_person_list_free(folder_person_list);

    if (folder_delete(folder)) {
        folder_free(folder);
        fprintf(stderr, "unable to delete folder!\n");
        return -6;
    }
    folder_free(folder);

    printf("Folder deleted successfully.\n");

    return 0;
}

int main(int argc, char* argv[]) {
    int opt, ret;
    libdbo_configuration_list_t* configuration_list = NULL;
    libdbo_configuration_t* configuration = NULL;
    libdbo_connection_t* connection = NULL;

    libdbo_mm_init();

    if (!(configuration_list = libdbo_configuration_list_new())) {
        fprintf(stderr, "memory allocation error!\n");
        return -2;
    }

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        char* name = NULL;
        char* value = NULL;

        switch (opt) {
        case 'o':
            if (!(name = strdup(optarg))
                || !(value = strchr(name, '='))
                || (*value = 0) != 0
                || !(*(value + 1))
                || !(configuration = libdbo_configuration_new())
                || libdbo_configuration_set_name(configuration, name)
                || libdbo_configuration_set_value(configuration, value + 1)
                || libdbo_configuration_list_add(configuration_list, configuration))
            {
                free(name);
                libdbo_configuration_free(configuration);
                libdbo_configuration_list_free(configuration_list);
                fprintf(stderr, "unable to add configuration option!\n");
                return -3;
            }
            configuration = NULL;
            free(name);
            break;

        default:
            usage();
            return -1;
        }
    }

    if (!(connection = libdbo_connection_new())
        || libdbo_connection_set_configuration_list(connection, configuration_list)
        || libdbo_connection_setup(connection))
    {
        libdbo_connection_free(connection);
        libdbo_configuration_list_free(configuration_list);
        fprintf(stderr, "unable to setup database connection!\n");
        return -4;
    }

    ret = -1;
    if ((argc - optind) && !strcmp(argv[optind], "person")) {
        if ((argc - optind) == 7 && !strcmp(argv[optind + 1], "create")) {
            ret = contacts_person_create(connection, argv[optind + 2],
                argv[optind + 3], argv[optind + 4], argv[optind + 5],
                argv[optind + 6]);
        }
        else if ((argc - optind) == 4 && !strcmp(argv[optind + 1], "put")) {
            ret = contacts_person_put(connection, argv[optind + 2],
                argv[optind + 3]);
        }
        else if ((argc - optind) == 2 && !strcmp(argv[optind + 1], "list")) {
            ret = contacts_person_list(connection);
        }
        else if ((argc - optind) == 5 && !strcmp(argv[optind + 1], "update")) {
            ret = contacts_person_update(connection, argv[optind + 2],
                argv[optind + 3], argv[optind + 4]);
        }
        else if ((argc - optind) == 3 && !strcmp(argv[optind + 1], "delete")) {
            ret = contacts_person_delete(connection, argv[optind + 2]);
        }
        else {
            usage();
        }
    }
    else if ((argc - optind) && !strcmp(argv[optind], "folder")) {
        if ((argc - optind) == 3 && !strcmp(argv[optind + 1], "create")) {
            ret = contacts_folder_create(connection, argv[optind + 2]);
        }
        else if ((argc - optind) > 1 && !strcmp(argv[optind + 1], "list")) {
            ret = contacts_folder_list(connection,
                (argc - optind) > 2 ? argv[optind + 2] : NULL);
        }
        else if ((argc - optind) == 3 && !strcmp(argv[optind + 1], "delete")) {
            ret = contacts_folder_delete(connection, argv[optind + 2]);
        }
        else {
            usage();
        }
    }
    else {
        usage();
    }

    libdbo_connection_free(connection);
    libdbo_configuration_list_free(configuration_list);
    return ret;
}
