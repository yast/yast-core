#
# Makefile.am for core/liby2/doc/autodocs
#

htmldir = $(docdir)/liby2/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/y2/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc -p liby2 *.h y2/*.h
	mv -f $(srcdir)/../../src/include/*.html .
