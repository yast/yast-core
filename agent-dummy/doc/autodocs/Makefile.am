#
# Makefile.am for y2scr/ag_dummy/doc/autodocs
#

htmldir = $(docdir)/ag_dummy/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc @RPMNAME@ *.h
	mv -f $(srcdir)/../../src/*.html .
