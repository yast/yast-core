#
# Makefile.am for core/agent-system/doc/autodocs
#

htmldir = $(docdir)/agent-system/autodocs

html_DATA = index.html $(wildcard *.html)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-system *.h
	mv -f $(srcdir)/../../src/*.html .

CLEANFILES = $(html_DATA)
