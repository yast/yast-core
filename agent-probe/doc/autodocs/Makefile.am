#
# Makefile.am for core/agent-probe/doc/autodocs
#

htmldir = $(docdir)/agent-probe/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-probe *.h
	mv -f $(srcdir)/../../src/*.html .
