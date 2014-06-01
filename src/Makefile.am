CLEANFILES = *.d *.so *_rdict.* *_rdict_* *.rootmap
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libdatabricxx.la

libdatabricxx_la_SOURCES = \
	logging.cxx \
	Bric.cxx \
	DbrxTools.cxx \
	Name.cxx NameTable.cxx \
	Props.cxx \
	RootIO.cxx \
	TypeReflection.cxx \
	Value.cxx HasValue.cxx\
	brics.cxx

libdatabricxx_la_headers = \
	logging.h \
	Bric.h \
	DbrxTools.h \
	Name.h NameTable.h \
	Props.h \
	RootIO.h \
	TypeReflection.h \
	Value.h HasValue.h \
	brics.h

pkginclude_HEADERS = $(libdatabricxx_la_headers)

bin_PROGRAMS =

noinst_PROGRAMS =

# databricxx_SOURCES = databricxx.cxx
# databricxx_LDADD = libdatabricxx.la
# databricxx_LDFLAGS = -static


rootmapdir = $(libdir)
rootmap_DATA =


libdatabricxx_la_SOURCES += libdatabricxx_rdict.cxx
rootmap_DATA += .libs/libdatabricxx.rootmap

rootmap_DATA += .libs/libdatabricxx_rdict.pcm

libdatabricxx_rdict.cxx: $(libdatabricxx_la_headers) databricxx_LinkDef.h
	$(ROOTCLING) -f $@ -s libdatabricxx@SHLIBEXT@ -rml libdatabricxx@SHLIBEXT@ -rmf libdatabricxx.rootmap -c $(CPPFLAGS) $(CXXFLAGS) -I$(pkgincludedir) $+

.libs/libdatabricxx.rootmap: libdatabricxx.la
	cp libdatabricxx.rootmap libdatabricxx_rdict.pcm .libs/

.libs/libdatabricxx_rdict.pcm: .libs/libdatabricxx.rootmap
