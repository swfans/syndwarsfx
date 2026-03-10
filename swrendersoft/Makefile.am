ACLOCAL_AMFLAGS = --install -I m4

lib_LIBRARIES = libswrender.a

libswrender_a_headers_src = \
  include/app_gentab.h \
  include/app_sprite.h \
  include/app_text_cw.h \
  include/app_text_sf.h \
  include/drawshape.h \
  include/drawtext.h \
  include/enginbckt.h \
  include/engincam.h \
  include/engincolour.h \
  include/engindrwlstm.h \
  include/engindrwlstx.h \
  include/enginfexpl.h \
  include/enginfloor.h \
  include/enginlights.h \
  include/enginpeff.h \
  include/enginpriobjs.h \
  include/enginpritxtr.h \
  include/enginprops.h \
  include/enginshadws.h \
  include/enginshrapn.h \
  include/enginsngobjs.h \
  include/enginsngtxtr.h \
  include/engintrns.h \
  include/engintxtrmap.h \
  include/enginzoom.h \
  include/frame_sprani.h \
  include/privrdlog.h \
  include/render_gpoly.h

libswrender_a_headers_bld = \
  include/swrendconf.h

libswrender_a_SOURCES = \
  $(libswrender_a_headers_src) \
  src/ap_gspr_map.c \
  src/ap_spr_smap.c \
  src/app_gengh.c \
  src/app_spr_init.c \
  src/app_text_cw.c \
  src/app_text_sf.c \
  src/drawshape.c \
  src/drawtext.c \
  src/enginbckt.c \
  src/engincam.c \
  src/engincolour.c \
  src/engindrwlstm.c \
  src/engindrwlstx.c \
  src/engindrwlstx_fac.c \
  src/engindrwlstx_spr.c \
  src/enginfexpl.c \
  src/enginfloor.c \
  src/enginlights.c \
  src/enginpeff.c \
  src/enginpriobjs.c \
  src/enginpritxtr.c \
  src/enginprops.c \
  src/enginshadws.c \
  src/enginshrapn.c \
  src/enginsngobjs.c \
  src/enginsngtxtr.c \
  src/engintrns.c \
  src/engintxtrmap.c \
  src/enginzoom.c \
  src/frame_sprani.c \
  src/render_gpoly.c

libswrender_a_CPPFLAGS = \
  -I"$(top_srcdir)/include" -I"$(builddir)/include"

libswrenderheadersdir = $(includedir)/swrender
libswrenderheaders_HEADERS = \
  $(libswrender_a_headers_src:%=$(srcdir)/%) \
  $(libswrender_a_headers_bld:%=$(builddir)/%)

# Libraries included with the project
libswrender_a_PROJECT_LIBS = ../lib/libbullfrog.a

$(libswrender_a_OBJECTS) $(lib_LIBRARIES): $(libswrender_a_PROJECT_LIBS)

../lib/libbullfrog.a: ../bflibrary/libbullfrog.a
	$(MAKE) -C $(<D) DESTDIR="../" prefix="" install
