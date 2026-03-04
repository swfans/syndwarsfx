ACLOCAL_AMFLAGS = --install -I m4

SUBDIRS = bflibrary bfsoundlib swrendersoft conf lang src util

# Do not install static libraries.
install installdirs: SUBDIRS = conf lang src util

doc_DATA = AUTHORS COPYING GPL3
docdir = $(prefix)/$(PACKAGE)

res_DATA =
resdir = $(prefix)/$(PACKAGE)

if TARGET_WINDOWS
res_DATA += res/syndwarsfx_icon.ico
else
res_DATA += res/syndwarsfx.icns res/syndwarsfx_icon.png
endif

instscript_DATA = util/install
instscriptdir = $(prefix)/$(PACKAGE)

EXTRA_DIST = doc res instscript
