#
# Makefile.am for y2a_ini/doc/autodocs
#
# $Id$
#

htmldir = $(docdir)/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc @RPMNAME@ *.h
	mv -f $(srcdir)/../../src/*.html .
