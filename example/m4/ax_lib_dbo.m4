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

#serial 1

AC_DEFUN([AX_LIB_DBO],
[
    AC_ARG_WITH([dbo],
        AS_HELP_STRING(
            [--with-dbo=@<:@ARG@:>@],
            [use DBO @<:@default=yes@:>@, optionally specify the prefix for DBO library]
        ),
        [
            if test "$withval" = "no"; then
                WANT_DBO="no"
            elif test "$withval" = "yes"; then
                WANT_DBO="yes"
                ac_dbo_path=""
            else
                WANT_DBO="yes"
                ac_dbo_path="$withval"
            fi
        ],
        [WANT_DBO="yes"]
    )

    DBO_CFLAGS=""
    DBO_LDFLAGS=""

    if test "x$WANT_DBO" = "xyes"; then

        if test "$ac_dbo_path" != ""; then
            ac_dbo_cflags="-I$ac_dbo_path/include"
            ac_dbo_ldflags="-L$ac_dbo_path/lib -ldbo"
            ac_dbo_path="$PATH:$ac_dbo_path/bin:$ac_dbo_path/sbin"
        else
            ac_dbo_cflags=""
            ac_dbo_ldflags=""
            ac_dbo_path="$PATH"
        fi

        saved_CFLAGS="$CFLAGS"
        saved_LDFLAGS="$LDFLAGS"

        CFLAGS="$CFLAGS $ac_dbo_cflags"
        LDFLAGS="$LDFLAGS $ac_dbo_ldflags"

        AC_LANG_PUSH([C])
        AC_CHECK_LIB(dbo, libdbo_mm_init,
            [success="yes"],
            [success="no"])
        AC_LANG_POP([C])

        CFLAGS="$saved_CFLAGS"
        LDFLAGS="$saved_LDFLAGS"

        AC_PATH_PROG([DBO_GENERATE_OBJECTS], [dbo-generate-objects], [false], [$ac_dbo_path])
        AC_PATH_PROG([DBO_GENERATE_SCHEMA], [dbo-generate-schema], [false], [$ac_dbo_path])
        AC_PATH_PROG([DBO_GENERATE_TESTS], [dbo-generate-tests], [false], [$ac_dbo_path])

        if test "$success" = "yes"; then

            DBO_CFLAGS="$ac_dbo_cflags"
            DBO_LDFLAGS="$ac_dbo_ldflags"
            AC_DEFINE([HAVE_LIBDBO], [1],
                      [Define to 1 if DBO libraries are available])

        fi
    fi

    AC_SUBST(DBO_CFLAGS)
    AC_SUBST(DBO_LDFLAGS)
])
