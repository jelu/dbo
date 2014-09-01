# Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MAINTAINERCLEANFILES = $(srcdir)/Makeperson.in

bin_PROGRAMS = contacts

BUILT_SOURCES = person.c person.h \
	person_ext.c person_ext.h \
	folder.c folder.h \
	folder_ext.c folder_ext.h \
	folder_person.c folder_person.h \
	folder_person_ext.c folder_person_ext.h \
	schema.sqlite

EXTRA_DIST = contacts.json

CLEANFILES = $(BUILT_SOURCES) stamp-objects schema.sqlite

contacts_SOURCES = contacts.c
nodist_contacts_SOURCES = person.c person.h \
	person_ext.c person_ext.h \
	folder.c folder.h \
	folder_ext.c folder_ext.h \
	folder_person.c folder_person.h \
	folder_person_ext.c folder_person_ext.h
contacts_CFLAGS = @DBO_CFLAGS@
contacts_LDFLAGS = @DBO_LDFLAGS@

person.c person.h \
person_ext.c person_ext.h \
folder.c folder.h \
folder_ext.c folder_ext.h \
folder_person.c folder_person.h \
folder_person_ext.c folder_person_ext.h: stamp-objects

stamp-objects: contacts.json
	@DBO_GENERATE_OBJECTS@ contacts.json
	touch stamp-objects

schema.sqlite: contacts.json
	@DBO_GENERATE_SCHEMA@ --backend sqlite contacts.json
