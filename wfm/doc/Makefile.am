# Makefile.am for core/wfm/doc

SUBDIRS =

htmldir = $(docdir)/wfm

html_DATA = YCP-builtins-wfm.html YCP-builtins-pkg.html

CLEANFILES = YCP-builtins-wfm.html YCP-builtins-pkg.html

builtin_wfm = $(srcdir)/../src/Y2WFMComponent.cc \
        $(srcdir)/../src/Y2CCWFM.cc

builtin_pkg = $(srcdir)/../src/PkgModule*.cc

YCP-builtins-wfm.html: $(builtin_wfm)
	@ybindir@/makebuiltindocs -t "WFM built-in functions" $^ > $@.new
	mv $@.new $@

YCP-builtins-pkg.html: $(builtin_pkg)
	@ybindir@/makebuiltindocs -t "Pkg:: built-in functions" $^ > $@.new
	mv $@.new $@

EXTRA_DIST = $(html_DATA)
