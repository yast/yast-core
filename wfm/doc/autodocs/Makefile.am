#
# Makefile.am for core/wfm/doc/autodocs
#

htmldir = $(docdir)/wfm/autodocs

html_DATA =					\
	WFMInterpreter.html			\
	Y2CCWFM.html				\
	Y2WFMComponent.html			\
	heir.html				\
	index.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc -p wfm *.h
	mv -f $(srcdir)/../../src/*.html .
