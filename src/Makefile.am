bin_PROGRAMS = syndwarsfx
bindir = $(prefix)/$(PACKAGE)

MKW = $(PYTHON) $(top_srcdir)/util/mkwrappers

syndwarsfx_SOURCES = \
	../swrendersoft/src/app_gengh.c \
	../swrendersoft/include/app_gentab.h \
	../swrendersoft/src/app_spr_init.c \
	../swrendersoft/src/ap_gspr_map.c \
	../swrendersoft/src/ap_spr_smap.c \
	../swrendersoft/include/app_sprite.h \
	../swrendersoft/src/app_text_cw.c \
	../swrendersoft/include/app_text_cw.h \
	../swrendersoft/src/app_text_sf.c \
	../swrendersoft/include/app_text_sf.h \
	../swrendersoft/src/drawshape.c \
	../swrendersoft/include/drawshape.h \
	../swrendersoft/src/drawtext.c \
	../swrendersoft/include/drawtext.h \
	../swrendersoft/src/enginbckt.c \
	../swrendersoft/include/enginbckt.h \
	../swrendersoft/src/engincolour.c \
	../swrendersoft/include/engincolour.h \
	../swrendersoft/src/enginfloor.c \
	../swrendersoft/include/enginfloor.h \
	../swrendersoft/src/engindrwlstx_fac.c \
	../swrendersoft/src/engindrwlstx_spr.c \
	../swrendersoft/include/engindrwlstx.h \
	../swrendersoft/src/enginlights.c \
	../swrendersoft/include/enginlights.h \
	../swrendersoft/src/enginpeff.c \
	../swrendersoft/include/enginpeff.h \
	../swrendersoft/src/enginpritxtr.c \
	../swrendersoft/include/enginpritxtr.h \
	../swrendersoft/src/engintxtrmap.c \
	../swrendersoft/include/engintxtrmap.h \
	../swrendersoft/src/enginprops.c \
	../swrendersoft/include/enginprops.h \
	../swrendersoft/src/enginsngtxtr.c \
	../swrendersoft/include/enginsngtxtr.h \
	../swrendersoft/src/enginzoom.c \
	../swrendersoft/include/enginzoom.h \
	../swrendersoft/src/frame_sprani.c \
	../swrendersoft/include/frame_sprani.h \
	../swrendersoft/include/render_gpoly.h \
	../swrendersoft/src/render_gpoly.c \
	../swrendersoft/include/privrdlog.h \
	bflib_snd_mss_s.sx \
	bflib_vidraw.c \
	bflib_vidraw.h \
	bflib_vidraw_s.sx \
	../bfsoundlib/src/ailssa.sx \
	../bfariadne/include/trfringe.h \
	../bfariadne/src/trfringe.c \
	../bfariadne/include/triangls.h \
	../bfariadne/src/triangls.c \
	../bfariadne/include/tringops.h \
	../bfariadne/src/tringops.c \
	../bfariadne/include/trpath.h \
	../bfariadne/include/trpoints.h \
	../bfariadne/src/trpoints.c \
	../bfariadne/src/trfind8.c \
	../bfariadne/include/trfind8.h \
	../bfariadne/src/trstate.c \
	../bfariadne/include/trstate.h \
	../bfariadne/src/delaunay.c \
	../bfariadne/include/delaunay.h \
	../bfsmacklib/include/bfsmack.h \
	../bfsmacklib/src/bfsmack.c \
	../bfsmacklib/include/smack.h \
	../bfsmacklib/include/smack2ail.h \
	../bfsmacklib/src/smack2ail.c \
	../bfsmacklib/src/smailfile.c \
	../bfsmacklib/src/bfsmacklib_s.sx \
	linksmk.h \
	linksmk.c \
	config.h \
	bmbang.c \
	bmbang.h \
	bat_s.sx \
	bat.c \
	bat.h \
	bigmap.c \
	bigmap.h \
	building.c \
	building.h \
	campaign.c \
	campaign.h \
	command.c \
	command.h \
	cybmod.c \
	cybmod.h \
	display.c \
	display.h \
	dos.c \
	dos.h \
	drawtext_wrp.c \
	drawtext_wrp.h \
	engindrwlstm.c \
	engindrwlstm.h \
	engindrwlstx.c \
	engindrwlstx_tng.h \
	enginfexpl.c \
	enginfexpl.h \
	enginpriobjs.c \
	enginpriobjs.h \
	enginshrapn.c \
	enginshrapn.h \
	enginsngobjs.c \
	enginsngobjs.h \
	enginshadws.c \
	enginshadws.h \
	engintext.c \
	engintext.h \
	engintrns.c \
	engintrns.h \
	feappbar.c \
	feappbar.h \
	febrief.c \
	febrief.h \
	fedebrief.c \
	fedebrief.h \
	fecntrls.c \
	fecntrls.h \
	fecryo.c \
	fecryo.h \
	feequip.c \
	feequip.h \
	feoptions.c \
	feoptions.h \
	fepanet.c \
	fepanet.h \
	feshared.c \
	feshared.h \
	felogin.c \
	felogin.h \
	femail.c \
	femail.h \
	femain.c \
	femain.h \
	fenet.c \
	fenet.h \
	fepause.c \
	fepause.h \
	feresearch.c \
	feresearch.h \
	festorage.c \
	festorage.h \
	feworld.c \
	feworld.h \
	guiboxes.c \
	guiboxes.h \
	guigraph.c \
	guigraph.h \
	guitext.c \
	guitext.h \
	game.c \
	game_bstype.h \
	game_data.c \
	game_data.h \
	game_options.c \
	game_options.h \
	game_save.c \
	game_save.h \
	game_speed.c \
	game_speed.h \
	game_sprts.c \
	game_sprts.h \
	game.h \
	globals.h \
	hud_panecfg.c \
	hud_panecfg.h \
	hud_panel.c \
	hud_panel.h \
	hud_target.c \
	hud_target.h \
	keyboard.c \
	keyboard.h \
	lvdraw3d.c \
	lvdraw3d.h \
	lvobjctv.c \
	lvobjctv.h \
	lvfiles.c \
	lvfiles.h \
	lvwalk.c \
	lvwalk.h \
	main.c \
	matrix.c \
	matrix.h \
	misstat.c \
	misstat.h \
	mouse.c \
	mouse.h \
	mydraw.c \
	mydraw.h \
	netipx1.sx \
	netser1.sx \
	network.c \
	network.h \
	pathtrig.c \
	pathtrig_debug.c \
	pathtrig.h \
	packet.c \
	packet.h \
	packetfe.c \
	packetfe.h \
	people.c \
	people.h \
	pepgroup.c \
	pepgroup.h \
	player.c \
	player.h \
	plyr_packet.c \
	plyr_packet.h \
	plyr_usrinp.c \
	plyr_usrinp.h \
	purpldrw.c \
	purpldrw.h \
	purpldrwlst.c \
	purpldrwlst.h \
	radica1.sx \
	radica2.sx \
	radica3.sx \
	research.c \
	research.h \
	rules.c \
	rules.h \
	scanner.c \
	scanner.h \
	scandraw.c \
	scandraw.h \
	sound.c \
	sound.h \
	specblit.c \
	specblit.h \
	swars.sx \
	swlog.h \
	thing.c \
	thing.h \
	thing_search.c \
	thing_search.h \
	thing_fire.c \
	thing_fire.h \
	thing_debug.c \
	thing_debug_s.sx \
	thing_onface.h \
	thing_onface.c \
	tngcolisn.c \
	tngcolisn.h \
	tngobjdrw.c \
	tngobjdrw.h \
	osunix.c \
	osunix.h \
	util.c \
	util.h \
	vehicle.c \
	vehicle.h \
	vehtraffic.c \
	vehtraffic.h \
	wadfile.c \
	wadfile.h \
	weapon.c \
	weapon.h \
	oswindws.c \
	oswindws.h \
	wrcities.c \
	wrcities.h

syndwarsfx_CPPFLAGS = \
  -I"$(top_srcdir)/bfsmacklib/include" \
  -I"$(top_srcdir)/swrendersoft/include" \
  -I"$(top_srcdir)/bfariadne/include"

syndwarsfx_WRAPPERS = wrappers_dos.o wrappers_game.o wrappers_libc.o wrappers_util.o

syndwarsfx_RCFLAGS = \
  -I"$(top_srcdir)/src" -I"$(builddir)" \
  $(RCFLAGS)

if HAS_WINDRES
syndwarsfx_RESRCS = syndwarsfx_stdres.res
else
syndwarsfx_RESRCS =
endif

# Pretending to contain c++ source so that Automake select c++ linker
nodist_EXTRA_syndwarsfx_SOURCES = dummy.cxx

# Libraries included with the project
syndwarsfx_PROJECT_LIBS = ../lib/libbullfrog.a ../lib/libbfsound.a

$(syndwarsfx_OBJECTS) $(bin_PROGRAMS): $(syndwarsfx_PROJECT_LIBS)

syndwarsfx_LDADD = \
  $(syndwarsfx_WRAPPERS) $(syndwarsfx_RESRCS)

../lib/libbullfrog.a: ../bflibrary/libbullfrog.a
	$(MAKE) -C $(<D) DESTDIR="../" prefix="" install

../lib/libbfsound.a: ../bfsoundlib/libbfsound.a
	$(MAKE) -C $(<D) DESTDIR="../" prefix="" install

$(syndwarsfx_WRAPPERS:.o=.sx): %.sx: $(top_srcdir)/conf/%.conf
	$(AM_V_GEN)$(MKW) $(MKWFLAGS) -o $@ $<

.sx.o:
	$(AM_V_CC)$(CPP) $(CPPFLAGS) $< $(ASFILTER) \
		| $(CCAS) -x assembler -c $(CCASFLAGS) -o $@ -

if HAS_WINDRES
%.res: $(top_srcdir)/res/%.rc
	$(WINDRES) $(syndwarsfx_RCFLAGS) -i $< -J rc -o $@ -O coff
endif

# Include dynamic libraries in the package
if TARGET_WINDOWS
install-exec-hook:
	mkdir -p "$(DESTDIR)${prefix}/$(PACKAGE)"
	## Find main executable dependencies
	$(eval lib_SHARED_INSTALL := $(shell objdump -p syndwarsfx$(EXEEXT) | \
		sed -n 's/\s*\(DLL Name:\|NEEDED\)\s*\(.*\)$$/\2/p' | \
		xargs -I {} find $(shell dirname $(shell which ${CXX})) -name '{}'))
	## Find sub-dependencies for the previous dependencies
	$(eval lib_SHARED_INSTALL += $(shell echo -n '$(lib_SHARED_INSTALL)' | \
        xargs -d ' ' -I {} objdump -p '{}' | \
		sed -n 's/\s*\(DLL Name:\|NEEDED\)\s*\(.*\)$$/\2/p' | \
		xargs -I {} find $(shell dirname $(shell which ${CXX})) -name '{}'))
	## Remove duplicates
	$(eval lib_SHARED_INSTALL := $(sort $(lib_SHARED_INSTALL)))
	cp $(lib_SHARED_INSTALL) $(DESTDIR)${prefix}/$(PACKAGE)
endif

CLEANFILES = wrappers_*.sx
DISTCLEANFILES = *~
