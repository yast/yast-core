#
# Makefile.am for core/wfm/doc/autodocs
#

htmldir = $(docdir)/wfm/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h $(srcdir)/../../src/PkgModule.cc
	@ybindir@/ydoc wfm -l libycp -l liby2 -l libscr --url $(htmldir) $^
