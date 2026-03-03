#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])

m4_define([swrenderlib_version], [0.1.0.0])

AC_INIT([syndwars-software-render-library], [swrenderlib_version], [mefistotelis@gmail.com], [swrenderlib])

AC_CONFIG_SRCDIR([src/drawtext.c])
AC_CONFIG_HEADERS([include/swrendprivconf.h])
AX_PREFIX_CONFIG_H([include/swrendconf.h], [SWR])
#AC_CONFIG_LINKS([include:include])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])

# Define arguments.

# Checks for programs.

AC_PROG_CC
AC_PROG_CXX
PKG_PROG_PKG_CONFIG
AC_PROG_RANLIB

# Checks for libraries.

# libraries from within this project are only built after `make` is executed
AC_SUBST(BFLIBRARY_LIBS, "-L\"\$(top_builddir)/../lib\" -lbullfrog")
AC_SUBST(BFLIBRARY_CFLAGS, "-I\"\$(top_builddir)/../include/bullfrog\"")


# Checks for header files.
AC_CHECK_HEADERS([stdint.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_CHECK_FUNCS([memmove memset])

# Flags

CFLAGS="$CFLAGS -Wall -Wextra -Wno-unused-parameter"
CFLAGS="$CFLAGS -include \"\$(top_builddir)/include/swrendconf.h\""

CFLAGS="$CFLAGS $BFLIBRARY_CFLAGS"
LIBS="$LIBS $BFLIBRARY_LIBS"

# Final commands

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
