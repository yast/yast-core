#
# Makefile.am for core/libyui/doc/autodocs
#

htmldir = $(docdir)/libyui/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc libyui *.h
	mv -f $(srcdir)/../../src/include/*.html .
