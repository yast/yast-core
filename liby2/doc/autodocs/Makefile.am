#
# Makefile.am for core/liby2/doc/autodocs
#

htmldir = $(docdir)/liby2/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

header_dir = $(srcdir)/../../src/include
index.html: $(header_dir)/*.h $(header_dir)/y2/*.h
	@ybindir@/ydoc liby2 --xref libycp --private --url $(htmldir) $^
