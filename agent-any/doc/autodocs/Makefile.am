#
# Makefile.am for core/agent-any/doc/autodocs
#

htmldir = $(docdir)/agent-any/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-any *.h
	mv -f $(srcdir)/../../src/*.html .
