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

AC_PREREQ(2.68)

AC_INIT([dbo], [0.0.1], [https://github.com/jelu/dbo/issues])
DBO_LIB_VERSION="0:1:0"

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_LANG([C])

LT_INIT

AC_SUBST(DBO_LIB_VERSION)

#
# Check for doxygen
#

DX_INIT_DOXYGEN([dbo])

#
# Check for required/optional libraries
#

AX_PTHREAD([
    LIBS="$PTHREAD_LIBS $LIBS"
    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
    CC="$PTHREAD_CC"
])
AX_CUNIT

#
# Check for SQLite3
#

AX_LIB_SQLITE3

if test "$SQLITE3_VERSION" != ""; then
    AM_CONDITIONAL(HAVE_SQLITE3, true)
else
    AM_CONDITIONAL(HAVE_SQLITE3, false)
fi

#
# Check for MySQL
#

AX_LIB_MYSQL([5.0.0])

if test "$MYSQL_VERSION" != ""; then
    AM_CONDITIONAL(HAVE_MYSQL, true)
else
    AM_CONDITIONAL(HAVE_MYSQL, false)
fi

TEST_MYSQL=""
TEST_MYSQL_USER=""
TEST_MYSQL_PASS=""
TEST_MYSQL_HOST=""
TEST_MYSQL_PORT="0"
TEST_MYSQL_DB=""

AC_ARG_WITH([mysql-user],
    AS_HELP_STRING([--with-mysql-user=@<:@ARG@:>@],
        [optionally specify the database user to run tests as]
    ),
    [
        if test "$withval" != ""; then
            TEST_MYSQL="1"
            TEST_MYSQL_USER="$withval"
        fi
    ], [])
AC_ARG_WITH([mysql-pass],
    AS_HELP_STRING([--with-mysql-pass=@<:@ARG@:>@],
        [optionally specify the database password to run tests with]
    ),
    [
        if test "$withval" != ""; then
            TEST_MYSQL="1"
            TEST_MYSQL_PASS="$withval"
        fi
    ], [])
AC_ARG_WITH([mysql-host],
    AS_HELP_STRING([--with-mysql-host=@<:@ARG@:>@],
        [optionally specify the database host to run tests on]
    ),
    [
        if test "$withval" != ""; then
            TEST_MYSQL="1"
            TEST_MYSQL_HOST="$withval"
        fi
    ], [])
AC_ARG_WITH([mysql-port],
    AS_HELP_STRING([--with-mysql-port=@<:@ARG@:>@],
        [optionally specify the database port to run tests on]
    ),
    [
        if test "$withval" != ""; then
            TEST_MYSQL="1"
            TEST_MYSQL_PORT="$withval"
        fi
    ], [])
AC_ARG_WITH([mysql-db],
    AS_HELP_STRING([--with-mysql-db=@<:@ARG@:>@],
        [optionally specify the database to run tests on]
    ),
    [
        if test "$withval" != ""; then
            TEST_MYSQL="1"
            TEST_MYSQL_DB="$withval"
        fi
    ], [])

if test "$TEST_MYSQL" != ""; then
    AC_DEFINE_UNQUOTED(TEST_MYSQL, [1], [Specify if MySQL tests should be run])
    AM_CONDITIONAL(TEST_MYSQL, true)
else
    AM_CONDITIONAL(TEST_MYSQL, false)
fi

AC_SUBST(TEST_MYSQL_USER)
AC_SUBST(TEST_MYSQL_PASS)
AC_SUBST(TEST_MYSQL_HOST)
AC_SUBST(TEST_MYSQL_PORT)
AC_SUBST(TEST_MYSQL_DB)

AC_DEFINE_UNQUOTED(TEST_MYSQL_USER, ["$TEST_MYSQL_USER"], [For tests])
AC_DEFINE_UNQUOTED(TEST_MYSQL_PASS, ["$TEST_MYSQL_PASS"], [For tests])
AC_DEFINE_UNQUOTED(TEST_MYSQL_HOST, ["$TEST_MYSQL_HOST"], [For tests])
AC_DEFINE_UNQUOTED(TEST_MYSQL_PORT, [$TEST_MYSQL_PORT], [For tests])
AC_DEFINE_UNQUOTED(TEST_MYSQL_PORT_TXT, ["$TEST_MYSQL_PORT"], [For tests])
AC_DEFINE_UNQUOTED(TEST_MYSQL_DB, ["$TEST_MYSQL_DB"], [For tests])

#
# Check for PostgreSQL
#

AX_LIB_POSTGRESQL

#
# Output makefiles
#

AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_FILES([
	Makefile
    doc/Makefile
	src/Makefile
    src/libdbo/Makefile
    src/libdbo/backend/Makefile
    test/Makefile
    tools/Makefile
])

AC_OUTPUT
