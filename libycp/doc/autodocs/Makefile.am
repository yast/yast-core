#
# Makefile.am for libycp/doc/autodocs
#

docdir = $(YAST2DOC)
htmldir = $(docdir)/autodocs

html_DATA = index.html $(wildcard *.html)

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/ycp/*.h
	cd $(srcdir)/../../src/include && @YAST2DIR@/bin/ydoc -p @RPMNAME@ *.h ycp/*.h
	mv -f $(srcdir)/../../src/include/*.html .
