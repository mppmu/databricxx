CLEANFILES = *.d *.so *-rdict.cxx *-rdict.h
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libtemplate-project.la

libtemplate_project_la_SOURCES = \
	HelloWorld.cxx

libtemplate_project_la_headers = \
	HelloWorld.h

pkginclude_HEADERS = $(libtemplate_project_la_headers)

bin_PROGRAMS = template-project

noinst_PROGRAMS =

template_project_SOURCES = template-project.cxx
template_project_LDADD = libtemplate-project.la
# template_project_LDFLAGS = -static


libtemplate_project_la_SOURCES += libtemplate-project-rdict.cxx

libtemplate-project-rdict.cxx: $(libtemplate_project_la_headers) template-project-LinkDef.h
	$(ROOTCINT) -f $@ -c -p $(CPPFLAGS) $(CXXFLAGS) $+

rootmapdir = $(libdir)
rootmap_DATA = .libs/libtemplate-project.rootmap

.libs/libtemplate-project.rootmap: libtemplate-project.la | $(libtemplate_project_la_headers) template-project-LinkDef.h
	$(RLIBMAP) -o $@ -l .libs/libtemplate-project@SHLIBEXT@ -c $|
