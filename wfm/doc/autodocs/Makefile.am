#
# Makefile.am for core/wfm/doc/autodocs
#

htmldir = $(docdir)/wfm/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc -p wfm *.h
	mv -f $(srcdir)/../../src/*.html .
