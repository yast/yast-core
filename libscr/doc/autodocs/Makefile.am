#
# Makefile.am for core/libscr/doc/autodocs
#

htmldir = $(docdir)/libscr/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

header_dir = $(srcdir)/../../src/include
index.html: $(header_dir)/*.h $(header_dir)/scr/*.h
	@ybindir@/ydoc libscr --xref libycp --xref liby2 --url $(htmldir) $^
