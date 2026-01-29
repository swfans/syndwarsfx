ACLOCAL_AMFLAGS = --install -I m4

lib_LIBRARIES = libbullfrog.a

libbullfrog_a_headers_src = \
  include/bfanywnd.h \
  include/bfbox.h \
  include/bfbuffer.h \
  include/bfcircle.h \
  include/bfdata.h \
  include/bfdir.h \
  include/bfdos.h \
  include/bfdrive.h \
  include/bfendian.h \
  include/bfexe_key.h \
  include/bffile.h \
  include/bfflic.h \
  include/bffnuniq.h \
  include/bffont.h \
  include/bfgentab.h \
  include/bfiff.h \
  include/bfini.h \
  include/bfkeybd.h \
  include/bfline.h \
  include/bflog.h \
  include/bfmath.h \
  include/bfmemory.h \
  include/bfmemut.h \
  include/bfmouse.h \
  include/bfpalcrss.h \
  include/bfpalette.h \
  include/bfpixel.h \
  include/bfplanar.h \
  include/bfpng.h \
  include/bfscrcopy.h \
  include/bfscreen.h \
  include/bfscrsurf.h \
  include/bfsemphr.hpp \
  include/bfsprite.h \
  include/bfstrut.h \
  include/bftext.h \
  include/bftime.h \
  include/bftringl.h \
  include/bftypes.h \
  include/bfutility.h \
  include/bfwindows.h \
  include/insprite.h \
  include/insspr.h \
  include/mshandler.hpp \
  include/mspointer.hpp \
  include/poly.h \
  include/poly_gp.h \
  include/poly_trigp.h \
  include/poly_trigr.h \
  include/privbflog.h \
  include/rnc_1fm.h \
  include/rom.h \
  include/svesa.h

libbullfrog_a_headers_bld = \
  include/bfconfig.h

libbullfrog_a_SOURCES = \
  $(libbullfrog_a_headers_src) \
  src/general/arctan.c \
  src/general/box_clip.c \
  src/general/ganywnd.c \
  src/general/gbox.c \
  src/general/gbuffer.cpp \
  src/general/gcircle.c \
  src/general/gdata.c \
  src/general/gendian.c \
  src/general/gerrorlg.c \
  src/general/gexe_key.c \
  src/general/gfile.c \
  src/general/gflicply.c \
  src/general/gflicrec.c \
  src/general/gfnuniq.c \
  src/general/gfont.c \
  src/general/ggenf.c \
  src/general/ggengh.c \
  src/general/giff.c \
  src/general/gini.c \
  src/general/gkeybd.c \
  src/general/gline.c \
  src/general/gmemut.c \
  src/general/gmouse.c \
  src/general/gpalcrss.c \
  src/general/gpalette.c \
  src/general/gpalvga.c \
  src/general/gplanar.c \
  src/general/gpng.c \
  src/general/gscreen.c \
  src/general/gsprite.c \
  src/general/gspr_col.c \
  src/general/gspr_enc.c \
  src/general/gspr_map.c \
  src/general/gspr_std.c \
  src/general/gstrut.c \
  src/general/gtext.c \
  src/general/gtrig.c \
  src/general/gtringl.c \
  src/general/gutility.c \
  src/general/hvline.c \
  src/general/mshandler.cpp \
  src/general/mspointer.cpp \
  src/general/pixel.c \
  src/general/poly.c \
  src/general/poly_gp.c \
  src/general/poly_trigp.c \
  src/general/poly_trigr.c \
  src/general/rnc_1fm.c \
  src/general/rom.c \
  src/general/scr_copy.c \
  src/general/spr_mcur.c \
  src/general/spr_scl.c \
  src/general/spr_scol.c \
  src/general/spr_smap.c \
  src/general/spr_sstd.c \
  src/general/spr_ssts.c \
  src/general/spr_ssta.c \
  src/general/sqroot.c

if USE_SDL2
libbullfrog_a_SOURCES += \
  src/x86-win-sdl2/sdir.c \
  src/x86-win-sdl2/sdrive.c \
  src/x86-win-sdl2/sffind.c \
  src/x86-win-sdl2/sfile.c \
  src/x86-win-sdl2/sjoyst.c \
  src/x86-win-sdl2/skeybd.cpp \
  src/x86-win-sdl2/slog.c \
  src/x86-win-sdl2/smemory.c \
  src/x86-win-sdl2/smouse.cpp \
  src/x86-win-sdl2/spalette.c \
  src/x86-win-sdl2/sscreen.c \
  src/x86-win-sdl2/sscrsurf.c \
  src/x86-win-sdl2/ssemphr.cpp \
  src/x86-win-sdl2/stime.c \
  src/x86-win-sdl2/svesa.c \
  src/x86-win-sdl2/swindows.c
else
libbullfrog_a_SOURCES += \
  src/x86-win-sdl/sdir.c \
  src/x86-win-sdl/sdrive.c \
  src/x86-win-sdl/sffind.c \
  src/x86-win-sdl/sfile.c \
  src/x86-win-sdl/sjoyst.c \
  src/x86-win-sdl/skeybd.cpp \
  src/x86-win-sdl/slog.c \
  src/x86-win-sdl/smemory.c \
  src/x86-win-sdl/smouse.cpp \
  src/x86-win-sdl/spalette.c \
  src/x86-win-sdl/sscreen.c \
  src/x86-win-sdl/sscrsurf.c \
  src/x86-win-sdl/ssemphr.cpp \
  src/x86-win-sdl/stime.c \
  src/x86-win-sdl/svesa.c \
  src/x86-win-sdl/swindows.c
endif

libbullfrog_a_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

libbullfrogheadersdir = $(includedir)/bullfrog
libbullfrogheaders_HEADERS = \
  $(libbullfrog_a_headers_src:%=$(srcdir)/%) \
  $(libbullfrog_a_headers_bld:%=$(builddir)/%)

check_PROGRAMS = \
  bflib_test_math \
  bflib_test_memory \
  bflib_test_pogpol \
  bflib_test_poline \
  bflib_test_potrig \
  bflib_test_sprdrw \
  bflib_test_tringl

bflib_test_math_SOURCES = \
  tests/bflib_test_math.c

bflib_test_math_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_math_SOURCES = dummy.cxx

bflib_test_math_LDADD = \
  -L$(builddir) -lbullfrog

bflib_test_memory_SOURCES = \
  tests/mock_windows.c \
  tests/bflib_test_memory.c

bflib_test_memory_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_memory_SOURCES = dummy.cxx

bflib_test_memory_LDADD = \
  -L$(builddir) -lbullfrog


bflib_test_pogpol_SOURCES = \
  tests/mock_mouse.c \
  tests/mock_palette.c \
  tests/mock_screen.c \
  tests/mock_windows.c \
  tests/helpers_fr_swars01.c \
  tests/helpers_screen.c \
  tests/bflib_test_pogpol.c

bflib_test_pogpol_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_pogpol_SOURCES = dummy.cxx

bflib_test_pogpol_LDADD = \
  -L$(builddir) -lbullfrog


bflib_test_poline_SOURCES = \
  tests/mock_mouse.c \
  tests/mock_palette.c \
  tests/mock_screen.c \
  tests/mock_windows.c \
  tests/helpers_screen.c \
  tests/bflib_test_poline.c

bflib_test_poline_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_poline_SOURCES = dummy.cxx

bflib_test_poline_LDADD = \
  -L$(builddir) -lbullfrog


bflib_test_potrig_SOURCES = \
  tests/mock_mouse.c \
  tests/mock_palette.c \
  tests/mock_screen.c \
  tests/mock_windows.c \
  tests/helpers_screen.c \
  tests/bflib_test_potrig.c

bflib_test_potrig_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_potrig_SOURCES = dummy.cxx

bflib_test_potrig_LDADD = \
  -L$(builddir) -lbullfrog


bflib_test_sprdrw_SOURCES = \
  tests/mock_mouse.c \
  tests/mock_palette.c \
  tests/mock_screen.c \
  tests/mock_windows.c \
  tests/helpers_sprite.c \
  tests/bflib_test_sprdrw.c

bflib_test_sprdrw_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_sprdrw_SOURCES = dummy.cxx

bflib_test_sprdrw_LDADD = \
  -L$(builddir) -lbullfrog


bflib_test_tringl_SOURCES = \
  tests/mock_mouse.c \
  tests/mock_palette.c \
  tests/mock_screen.c \
  tests/mock_windows.c \
  tests/helpers_screen.c \
  tests/bflib_test_tringl.c

bflib_test_tringl_CPPFLAGS = \
  -I$(top_srcdir)/include -I$(builddir)/include

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_bflib_test_tringl_SOURCES = dummy.cxx

bflib_test_tringl_LDADD = \
  -L$(builddir) -lbullfrog

TESTS = $(check_PROGRAMS)
