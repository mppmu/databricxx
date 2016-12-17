CLEANFILES = *.d *.so *_rdict.* *_rdict_* *.rootmap
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libdatabricxx.la

libdatabricxx_la_SOURCES = \
	DbrxTools.cxx \
	ManagedStream.cxx \
	RootCollection.cxx \
	TypeReflection.cxx \
	WrappedTObj.cxx \
	funcprog.cxx \
	format.cxx \
	Name.cxx \
	NameTable.cxx \
	Printable.cxx \
	Props.cxx \
	logging.cxx \
	ApplicationConfig.cxx \
	Value.cxx \
	HasValue.cxx \
	Bric.cxx \
	ApplicationBric.cxx \
	basicbrics.cxx \
	collbrics.cxx \
	funcbrics.cxx \
	MRBric.cxx \
	propsbrics.cxx \
	RootHistBuilder.cxx \
	rootiobrics.cxx \
	RootIO.cxx \
	RootRndGen.cxx \
	textbrics.cxx \
	WrappedTObjConv.cxx

libdatabricxx_la_headers = \
	DbrxTools.h \
	ManagedStream.h \
	RootCollection.h \
	TypeReflection.h \
	WrappedTObj.h \
	funcprog.h \
	format.h \
	Name.h \
	NameTable.h \
	Printable.h \
	Props.h \
	logging.h \
	ApplicationConfig.h \
	Value.h \
	HasValue.h \
	Bric.h \
	ApplicationBric.h \
	basicbrics.h \
	collbrics.h \
	funcbrics.h \
	MRBric.h \
	propsbrics.h \
	RootHistBuilder.h \
	rootiobrics.h \
	RootIO.h \
	RootRndGen.h \
	textbrics.h \
	WrappedTObjConv.h

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
	$(GREP) -F -v '"'"`pwd`"'/",' $@.tmp | $(SED) 's|"\([^/"]*/\)*\([^/"]*[.]h\)",|"'$(PACKAGE)/'\2",| ; s|\\"\([^/"]*/\)*\([^/"]*[.]h\)\\"\\n"|\\"'$(PACKAGE)/'\2\\"\\n"|' > $@.tmp2
	$(SED) 's|\$$clingAutoload\$$\([^/"]*/\)*|$$clingAutoload$$'$(PACKAGE)'/|; /.*DICTPAYLOAD(.*/,/.*)DICTPAYLOAD.*/ s|#include "\([^/"]*/\)*\(.*\)"|#include <'$(PACKAGE)'/\2>|' $@.tmp2 > $@ && $(RM) $@.tmp $@.tmp2
	$(SED) 's|\(header  *\)\([^ ].*/\)*\([^ ].*[.]h\)|\1'$(PACKAGE)/'\3|' libdatabricxx.rootmap.tmp > libdatabricxx.rootmap && $(RM) libdatabricxx.rootmap.tmp

.libs/libdatabricxx.rootmap: libdatabricxx.la
	cp libdatabricxx.rootmap libdatabricxx_rdict.pcm .libs/

.libs/libdatabricxx_rdict.pcm: .libs/libdatabricxx.rootmap
