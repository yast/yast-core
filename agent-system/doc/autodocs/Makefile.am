#
# Makefile.am for y2a_system/doc/autodocs
#
# $Id$
#

htmldir = $(docdir)/autodocs

html_DATA = index.html $(wildcard *.html)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc @RPMNAME@ *.h
	mv -f $(srcdir)/../../src/*.html .

CLEANFILES = $(html_DATA)
