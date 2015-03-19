MAINTAINERCLEANFILES = Makefile.in html/* latex/*

EXTRA_DIST = \
	html/index.html html latex/refman.tex latex \
	\
	examples/examples.md \
	examples/rndgen-ttree-io-hist.json \
	examples/param_group_example.C

html/index.html latex/refman.tex: $(top_srcdir)/src/*.cxx $(top_srcdir)/src/*.h
	doxygen
