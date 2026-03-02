#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])

m4_define([bflibrary_version], [0.1.3.0])

AC_INIT([bullfrog-library], [bflibrary_version], [mefistotelis@gmail.com], [bflibrary])

AC_CONFIG_SRCDIR([src/general/gfile.c])
AC_CONFIG_HEADERS([include/bfprivconfig.h])
AX_PREFIX_CONFIG_H([include/bfconfig.h], [LB])
#AC_CONFIG_LINKS([include:include])
# custom command for AC_CONFIG_LINKS([referenc/:tests/referenc/*.png])
AC_CONFIG_COMMANDS([tests_referenc], [
	mkdir -p "$ac_builddir/referenc"
	cp -f "$ac_srcdir/tests/referenc/"*.png "$ac_builddir/referenc"
])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])

# Define arguments.

AC_ARG_WITH([sdl2],
    AS_HELP_STRING([--without-sdl2], [Build without use of SDL2 library]))

AC_ARG_ENABLE([lb-wscreen-control],
    AS_HELP_STRING([--disable-lb-wscreen-control],
        [disallow the bflibrary to change value of lbDisplay.WScreen]),
    [lb_wscreen_control=$enableval], [lb_wscreen_control=yes])

AS_IF([test "$lb_wscreen_control" = "yes"], [
    AC_DEFINE([WSCREEN_CONTROL], 1,
        [Define to 1 if the bflibrary should change value of lbDisplay.WScreen])
])

AC_ARG_ENABLE([lb-filename-transform],
    AS_HELP_STRING([--disable-lb-filename-transform],
        [do not transform filenames received by bflibrary file handling routines]),
    [lb_filename_transform=$enableval], [lb_filename_transform=yes])

AS_IF([test "$lb_filename_transform" = "yes"], [
    AC_DEFINE([FILENAME_TRANSFORM], 1,
        [Define to 1 if the bflibrary file handling should transform filenames])
])

AC_ARG_ENABLE([lb-shadow-colour],
    AS_HELP_STRING([--disable-lb-shadow-colour],
        [make the bflibrary to not define lbDisplay.ShadowColour property]),
    [lb_shadow_colour=$enableval], [lb_shadow_colour=yes])

AS_IF([test "$lb_shadow_colour" = "yes"], [
    AC_DEFINE([ENABLE_SHADOW_COLOUR], 1,
        [Define to 1 if the bflibrary should define and use lbDisplay.ShadowColour property])
])

AC_ARG_ENABLE([lb-memory-arenas],
    AS_HELP_STRING([--disable-lb-memory-arenas],
        [make the bflibrary to not maintain list of memory blocks and arenas, rely on standard C malloc instead]),
    [lb_memory_arenas=$enableval], [lb_memory_arenas=yes])

AS_IF([test "$lb_memory_arenas" = "yes"], [
    AC_DEFINE([MEMORY_ARENAS], 1,
        [Define to 1 if the bflibrary should maintain list of memory blocks and arenas within the library, rather than relying on standard C memory allocation interface])
])

AC_ARG_WITH([lb-memory-available],
    [AS_HELP_STRING([--with-lb-memory-available=N],
        [set amount of memory available for allocations in above 1MB arena to N KB (default: 16384)])],
    [lb_memory_available="$withval"], [lb_memory_available=16384])

AC_DEFINE_UNQUOTED([MEMORY_AVAILABLE], [$lb_memory_available],
    [Set to amount of memory available for allocations in above 1MB arena, in kilobytes; only used in memory arenas mode])

AC_ARG_WITH([lb-memory-low-available],
    [AS_HELP_STRING([--with-lb-memory-low-available=N],
        [set amount of memory available for allocations in below 1MB arena to N KB (default: 512)])],
    [lb_memory_low_available="$withval"], [lb_memory_low_available=512])

AC_DEFINE_UNQUOTED([MEMORY_LOW_AVAILABLE], [$lb_memory_low_available],
    [Set to amount of memory available for allocations in below 1MB arena, in kilobytes; only used in memory arenas mode])

AC_ARG_ENABLE([lb-mouse-wheel],
    AS_HELP_STRING([--disable-lb-mouse-wheel],
        [make the bflibrary to not define mouse wheel properties within lbDisplay]),
    [lb_mouse_wheel=$enableval], [lb_mouse_wheel=yes])

AS_IF([test "$lb_mouse_wheel" = "yes"], [
    AC_DEFINE([ENABLE_MOUSE_WHEEL], 1,
        [Define to 1 if the bflibrary should maintain mouse wheel properties within lbDisplay])
])

AC_ARG_ENABLE([lb-mouse-move-ratio],
    AS_HELP_STRING([--disable-lb-mouse-move-ratio],
        [make the bflibrary to not define MouseMoveRatio properties within lbDisplay]),
    [lb_mouse_move_ratio=$enableval], [lb_mouse_move_ratio=yes])

AS_IF([test "$lb_mouse_move_ratio" = "yes"], [
    AC_DEFINE([ENABLE_MOUSE_MOVE_RATIO], 1,
        [Define to 1 if the bflibrary should support MouseMoveRatio properties within lbDisplay])
])

AC_ARG_ENABLE([lb-poly-render-time-check],
    AS_HELP_STRING([--enable-lb-poly-render-time-check],
        [make the bflibrary log a warning on exceptionally long polygon render]),
    [lb_poly_render_time_check=$enableval], [lb_poly_render_time_check=no])

AS_IF([test "$lb_poly_render_time_check" = "yes"], [
    AC_DEFINE([POLY_RENDER_TIME_CHECK], 1,
        [Define to 1 if the bflibrary should log a warning on exceptionally long polygon render])
])

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

PKG_CHECK_MODULES([PNG], [libpng], [], [
  AC_MSG_ERROR([libpng not found])
])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd memmove memset mkdir rmdir])

# Flags

CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

CFLAGS="$CFLAGS $PNG_CFLAGS"
CXXFLAGS="$CXXFLAGS $PNG_CFLAGS"
LIBS="$LIBS $PNG_LIBS"

AM_CONDITIONAL([USE_SDL2], [test "x$have_sdl2" == "xyes"])

# Final commands

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
