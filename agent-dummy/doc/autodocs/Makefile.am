#
# Makefile.am for core/agent-dummy/doc/autodocs
#

htmldir = $(docdir)/agent-dummy/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-dummy *.h
	mv -f $(srcdir)/../../src/*.html .
