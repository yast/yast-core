#
# Makefile.am for y2wfm/doc
#

SUBDIRS = autodocs

htmldir = $(docdir)

html_DATA = YCP-builtins-wfm.html

builtin = $(srcdir)/../src/WFMInterpreter.cc \
        $(srcdir)/../src/Y2WFMComponent.cc \
        $(srcdir)/../src/Y2CCWFM.cc

YCP-builtins-wfm.html: $(builtin)
	@ybindir@/makebuiltindocs -t "WFM built-in functions" $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = YCP-builtins-wfm.html
EXTRA_DIST = $(html_DATA)
