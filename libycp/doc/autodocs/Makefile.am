#
# Makefile.am for core/libycp/doc/autodocs
#

htmldir = $(docdir)/libycp/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/ycp/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc -p libycp *.h ycp/*.h
	mv -f $(srcdir)/../../src/include/*.html .
