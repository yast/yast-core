#
# Makefile.am for y2scr/ag_losetup/doc/autodocs
#

htmldir = $(docdir)/ag_losetup/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc @RPMNAME@ *.h
	mv -f $(srcdir)/../../src/*.html .
