#
# Makefile.am for core/agent-losetup/doc/autodocs
#

htmldir = $(docdir)/agent-losetup/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-losetup *.h
	mv -f $(srcdir)/../../src/*.html .
