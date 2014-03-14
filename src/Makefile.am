CLEANFILES = *.d *.so *_rdict.* *_rdict_* *.rootmap
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libdatabricxx.la

libdatabricxx_la_SOURCES =

libdatabricxx_la_headers =

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

if WITH_CLING

rootmap_DATA += .libs/libdatabricxx_rdict.pcm

libdatabricxx_rdict.cxx: $(libdatabricxx_la_headers) databricxx_LinkDef.h
	$(ROOTCLING) -f $@ -s libdatabricxx@SHLIBEXT@ -rml libdatabricxx@SHLIBEXT@ -rmf libdatabricxx.rootmap -c $(CPPFLAGS) $(CXXFLAGS) -I$(pkgincludedir) $+

.libs/libdatabricxx.rootmap: libdatabricxx.la
	cp libdatabricxx.rootmap libdatabricxx_rdict.pcm .libs/

.libs/libdatabricxx_rdict.pcm: .libs/libdatabricxx.rootmap

else # not WITH_CLING

libdatabricxx_rdict.cxx: $(libdatabricxx_la_headers) databricxx_LinkDef.h
	$(ROOTCINT) -f $@ -c -p $(CPPFLAGS) $(CXXFLAGS) $+

.libs/libdatabricxx.rootmap: libdatabricxx.la | $(libdatabricxx_la_headers) databricxx_LinkDef.h
	$(RLIBMAP) -o $@ -l .libs/libdatabricxx@SHLIBEXT@ -c $|

endif # WITH_CLING
