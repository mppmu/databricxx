CLEANFILES = *.d *.so *-rdict.cxx *-rdict.h
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libdatabricxx.la

libdatabricxx_la_SOURCES = \
	HelloWorld.cxx

libdatabricxx_la_headers = \
	HelloWorld.h

pkginclude_HEADERS = $(libdatabricxx_la_headers)

bin_PROGRAMS = databricxx

noinst_PROGRAMS =

databricxx_SOURCES = databricxx.cxx
databricxx_LDADD = libdatabricxx.la
# databricxx_LDFLAGS = -static


libdatabricxx_la_SOURCES += libdatabricxx-rdict.cxx

libdatabricxx-rdict.cxx: $(libdatabricxx_la_headers) databricxx-LinkDef.h
	$(ROOTCINT) -f $@ -c -p $(CPPFLAGS) $(CXXFLAGS) $+

rootmapdir = $(libdir)
rootmap_DATA = .libs/libdatabricxx.rootmap

.libs/libdatabricxx.rootmap: libdatabricxx.la | $(libdatabricxx_la_headers) databricxx-LinkDef.h
	$(RLIBMAP) -o $@ -l .libs/libdatabricxx@SHLIBEXT@ -c $|
