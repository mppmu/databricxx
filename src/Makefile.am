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
	histbrics.cxx \
	logging.cxx \
	textbrics.cxx \
	Bric.cxx \
	DbrxTools.cxx \
	MRBric.cxx \
	Name.cxx NameTable.cxx \
	Props.cxx \
	RootCollection.cxx \
	RootIO.cxx \
	TypeReflection.cxx \
	Value.cxx HasValue.cxx

libdatabricxx_la_headers = \
	basicbrics.h \
	funcbrics.h \
	collbrics.h \
	format.h \
	funcprog.h \
	histbrics.h \
	logging.h \
	textbrics.h \
	Bric.h \
	DbrxTools.h \
	MRBric.h \
	Name.h NameTable.h \
	Props.h \
	RootCollection.h \
	RootIO.h \
	TypeReflection.h \
	Value.h HasValue.h

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
	$(ROOTCLING) -f $@.tmp -s libdatabricxx@SHLIBEXT@ -rml libdatabricxx@SHLIBEXT@ -rmf libdatabricxx.rootmap -c $(CPPFLAGS) $(CXXFLAGS) -I$(pkgincludedir) $+
	$(GREP) -F -v '"'"`pwd`"'/",' $@.tmp > $@ && $(RM) $@.tmp

.libs/libdatabricxx.rootmap: libdatabricxx.la
	cp libdatabricxx.rootmap libdatabricxx_rdict.pcm .libs/

.libs/libdatabricxx_rdict.pcm: .libs/libdatabricxx.rootmap
