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

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

AM_CFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(srcdir) \
	-I$(builddir)

if HAVE_CUNIT
check_PROGRAMS = test

BUILT_SOURCES = groups.c groups.h \
	groups_ext.c groups_ext.h \
	groups_rev.c groups_rev.h \
	groups_rev_ext.c groups_rev_ext.h \
	users.c users.h \
	users_ext.c users_ext.h \
	user_group_link.c user_group_link.h \
	user_group_link_ext.c user_group_link_ext.h \
	user_group_link_rev.c user_group_link_rev.h \
	user_group_link_rev_ext.c user_group_link_rev_ext.h \
	users_rev.c users_rev.h \
	users_rev_ext.c users_rev_ext.h

BUILT_SOURCES += test_groups.c test_groups.h \
	test_groups_rev.c test_groups_rev.h \
	test_users.c test_users.h \
	test_user_group_link.c test_user_group_link.h \
	test_user_group_link_rev.c test_user_group_link_rev.h \
	test_users_rev.c test_users_rev.h

nodist_test_SOURCES = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

if HAVE_SQLITE3
BUILT_SOURCES += schema.sqlite
CLEANFILES += db_schema_sqlite.c db_schema_sqlite.h
endif
if HAVE_MYSQL
BUILT_SOURCES += drop.mysql schema.mysql
CLEANFILES += db_schema_mysql.c db_schema_mysql.h
endif

test_SOURCES = \
	test.c test.h \
	test_mm.c \
	test_classes.c \
	test_initialization.c \
	test_database_operations.c
test_LDADD = $(top_builddir)/src/libdbo.la
test_CFLAGS = @CUNIT_CFLAGS@ \
	@SQLITE3_CFLAGS@ \
	@MYSQL_CFLAGS@
test_LDFLAGS = -no-install @CUNIT_LDFLAGS@ \
	@SQLITE3_LDFLAGS@ \
	@MYSQL_LDFLAGS@

check-local: test
if HAVE_SQLITE3
	rm -f test.db
	sqlite3 test.db < $(srcdir)/test.sqlite
	sqlite3 test.db < $(builddir)/schema.sqlite
endif
	./test

groups.c groups.h \
groups_ext.c groups_ext.h \
groups_rev.c groups_rev.h \
groups_rev_ext.c groups_rev_ext.h \
users.c users.h \
users_ext.c users_ext.h \
user_group_link.c user_group_link.h \
user_group_link_ext.c user_group_link_ext.h \
user_group_link_rev.c user_group_link_rev.h \
user_group_link_rev_ext.c user_group_link_rev_ext.h \
users_rev.c users_rev.h \
users_rev_ext.c users_rev_ext.h:
	$(top_srcdir)/tools/dbo-generate-objects ./test.json

if HAVE_SQLITE3
schema.sqlite:
	$(top_srcdir)/tools/dbo-generate-schema --backend sqlite ./test.json
endif
if HAVE_MYSQL
drop.mysql schema.mysql:
	$(top_srcdir)/tools/dbo-generate-schema --backend mysql ./test.json
endif

test_groups.c test_groups.h \
test_groups_rev.c test_groups_rev.h \
test_users.c test_users.h \
test_user_group_link.c test_user_group_link.h \
test_user_group_link_rev.c test_user_group_link_rev.h \
test_users_rev.c test_users_rev.h:
	$(top_srcdir)/tools/dbo-generate-tests ./test.json

endif
