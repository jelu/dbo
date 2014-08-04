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

AC_DEFUN([AX_CUNIT],
[
    AC_ARG_WITH([cunit],
        AS_HELP_STRING(
            [--with-cunit=@<:@ARG@:>@],
            [use CUnit for testing @<:@default=yes@:>@, optionally specify the prefix for CUnit library]
        ),
        [
            if test "$withval" = "no"; then
                WANT_CUNIT="no"
            elif test "$withval" = "yes"; then
                WANT_CUNIT="yes"
                ac_cunit_path=""
            else
                WANT_CUNIT="yes"
                ac_cunit_path="$withval"
            fi
        ],
        [WANT_CUNIT="yes"]
    )

    CUNIT_CFLAGS=""
    CUNIT_LDFLAGS=""

    if test "x$WANT_CUNIT" = "xyes"; then

        ac_cunit_cflags="-I$CUNIT_PATH/include"
        ac_cunit_ldflags="-L$CUNIT_PATH/lib -lcunit"

        saved_CFLAGS="$CFLAGS"
        saved_LDFLAGS="$LDFLAGS"

        CFLAGS="$CFLAGS $ac_cunit_cflags"
        LDFLAGS="$LDFLAGS $ac_cunit_ldflags"

        AC_LANG_PUSH([C])
        AC_CHECK_LIB(cunit, CU_run_test,
            [success="yes"],
            [success="no"])
        AC_LANG_POP([C])

        CFLAGS="$saved_CFLAGS"
        LDFLAGS="$saved_LDFLAGS"

        if test "$success" = "yes"; then

            CUNIT_CFLAGS="$ac_cunit_cflags"
            CUNIT_LDFLAGS="$ac_cunit_ldflags"
            AM_CONDITIONAL(HAVE_CUNIT, true)

        else

            AM_CONDITIONAL(HAVE_CUNIT, false)

        fi
    fi

    AC_SUBST(CUNIT_CFLAGS)
    AC_SUBST(CUNIT_LDFLAGS)
])
