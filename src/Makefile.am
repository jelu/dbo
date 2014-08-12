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

lib_LTLIBRARIES = libdbo.la
libdbo_la_SOURCES = libdbo_mm.c libdbo_mm.h \
	libdbo_backend.c libdbo_backend.h \
	libdbo_clause.c libdbo_clause.h \
	libdbo_configuration.c libdbo_configuration.h \
	libdbo_connection.c libdbo_connection.h \
	libdbo_object.c libdbo_object.h \
	libdbo_result.c libdbo_result.h \
	libdbo_type.h \
	libdbo_value.c libdbo_value.h \
	libdbo_join.c libdbo_join.h \
	libdbo_error.h
if HAVE_SQLITE3
libdbo_la_SOURCES += libdbo_backend_sqlite.c libdbo_backend_sqlite.h
endif
if HAVE_MYSQL
libdbo_la_SOURCES += libdbo_backend_mysql.c libdbo_backend_mysql.h
endif
libdbo_la_CFLAGS = \
	@SQLITE3_CFLAGS@ \
	@MYSQL_CFLAGS@
libdbo_la_LDFLAGS = -version-info @DBO_LIB_VERSION@ \
	@SQLITE3_LDFLAGS@ \
	@MYSQL_LDFLAGS@
