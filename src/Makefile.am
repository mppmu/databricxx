CLEANFILES = *.d *.so *_rdict.* *_rdict_* *.rootmap
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libdatabricxx.la

libdatabricxx_la_SOURCES = \
	basicbrics.cxx \
	funcbrics.cxx \
	collbrics.cxx \
	format.cxx \
	funcprog.cxx \
	logging.cxx \
	propsbrics.cxx \
	rootiobrics.cxx \
	textbrics.cxx \
	ApplicationBric.cxx \
	ApplicationConfig.cxx \
	Bric.cxx \
	DbrxTools.cxx \
	ManagedStream.cxx \
	MRBric.cxx \
	Name.cxx NameTable.cxx \
	Printable.cxx \
	Props.cxx \
	RootCollection.cxx \
	RootHistBuilder.cxx \
	RootIO.cxx \
	RootRndGen.cxx \
	TypeReflection.cxx \
	Value.cxx HasValue.cxx \
	WrappedTObj.cxx

libdatabricxx_la_headers = \
	basicbrics.h \
	funcbrics.h \
	collbrics.h \
	format.h \
	funcprog.h \
	logging.h \
	propsbrics.h \
	rootiobrics.h \
	textbrics.h \
	ApplicationBric.h \
	ApplicationConfig.h \
	Bric.h \
	DbrxTools.h \
	ManagedStream.h \
	MRBric.h \
	Name.h NameTable.h \
	Printable.h \
	Props.h \
	RootCollection.h \
	RootHistBuilder.h \
	RootIO.h \
	RootRndGen.h \
	TypeReflection.h \
	Value.h HasValue.h \
	WrappedTObj.h

pkginclude_HEADERS = $(libdatabricxx_la_headers)

bin_PROGRAMS = dbrx

noinst_PROGRAMS =

dbrx_SOURCES = dbrx.cxx
dbrx_LDADD = libdatabricxx.la
dbrx_LDFLAGS = -static


rootmapdir = $(libdir)
rootmap_DATA =


libdatabricxx_la_SOURCES += libdatabricxx_rdict.cxx
rootmap_DATA += .libs/libdatabricxx.rootmap

rootmap_DATA += .libs/libdatabricxx_rdict.pcm

libdatabricxx_rdict.cxx: $(libdatabricxx_la_headers) databricxx_LinkDef.h
	$(ROOTCLING) -f $@.tmp -s libdatabricxx@SHLIBEXT@ -rml libdatabricxx@SHLIBEXT@ -rmf libdatabricxx.rootmap.tmp -c $(CPPFLAGS) $(CXXFLAGS) -I$(includedir) $+
	@# Some magic to prefix header names with "$(PACKAGE)/", and only that, in dictionary and rootmap:
	$(GREP) -F -v '"'"`pwd`"'/",' $@.tmp | $(SED) 's|"\([^"]*/\)\?\([^/"]*[.]h\)",|"'$(PACKAGE)/'\2",| ; s|\\"\([^"]*/\)\?\([^/"]*[.]h\)\\"\\n"|\\"'$(PACKAGE)/'\2\\"\\n"|' > $@.tmp2
	$(SED) 's|\$$clingAutoload\$$\([^/""]\+/\)*|$$clingAutoload$$'$(PACKAGE)'/|; /.*DICTPAYLOAD(.*/,/.*)DICTPAYLOAD.*/ s|#include "\([^/"]\+/\)*\(.*\)"|#include <'$(PACKAGE)'/\2>|' $@.tmp2 > $@ && $(RM) $@.tmp $@.tmp2
	$(SED) 's|\(header \+\)\([^ ].*/\)\?\([^ ].*[.]h\)|\1'$(PACKAGE)/'\3|' libdatabricxx.rootmap.tmp > libdatabricxx.rootmap && $(RM) libdatabricxx.rootmap.tmp

.libs/libdatabricxx.rootmap: libdatabricxx.la
	cp libdatabricxx.rootmap libdatabricxx_rdict.pcm .libs/

.libs/libdatabricxx_rdict.pcm: .libs/libdatabricxx.rootmap
