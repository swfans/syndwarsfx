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

AC_ARG_WITH([sdl2],
    AS_HELP_STRING([--without-sdl2], [Build without use of SDL2 library]))

# Checks for programs.

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.

AS_IF([test "x$with_sdl2" != "xno"],
  [AM_PATH_SDL2([2.0.0], [
    have_sdl2=yes], [
    have_sdl2=no])], [
  have_sdl2=no
])
AC_MSG_CHECKING([whether to use SDL2 library])
AS_IF([test "x$have_sdl2" == "xyes"], [
  AC_DEFINE(ENABLE_SDL2, [1], [Define for SDL2 support])
], [
  AS_IF([test "x$with_sdl2" = "xyes"], [
    AC_MSG_ERROR([sdl2 requested but not found])
  ])
])
AC_MSG_RESULT([$have_sdl2])

AS_IF([test "x$have_sdl2" != "xyes"], [
  AM_PATH_SDL([1.2.0], [], [
    AC_MSG_ERROR([could not find SDL2 nor SDL1 library])
  ])
  AC_DEFINE(ENABLE_SDL1, [1], [Define for SDL1 support])
])

# Do not allow SDL to control machine emulation
AS_IF([test "$have_windows" = "yes"], [
  SDL_LIBS=`echo $SDL_LIBS | sed -e 's/\(^\|\s\)-mwindows\>//g'`
])

PKG_CHECK_MODULES([PNG], [libpng], [], [
  AC_MSG_ERROR([libpng not found])
])

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

CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

CFLAGS="$CFLAGS $PNG_CFLAGS"
CXXFLAGS="$CXXFLAGS $PNG_CFLAGS"
LIBS="$LIBS $PNG_LIBS"

AM_CONDITIONAL([USE_SDL2], [test "x$have_sdl2" == "xyes"])

CFLAGS="$CFLAGS $BFLIBRARY_CFLAGS"
LIBS="$LIBS $BFLIBRARY_LIBS"

# Final commands

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
