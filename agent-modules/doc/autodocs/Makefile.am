#
# Makefile.am for core/agent-modules/doc/autodocs
#

htmldir = $(docdir)/agent-modules/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-modules *.h
	mv -f $(srcdir)/../../src/*.html .
