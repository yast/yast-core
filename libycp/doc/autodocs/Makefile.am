#
# Makefile.am for core/libycp/doc/autodocs
#

htmldir = $(docdir)/libycp/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

header_dir = $(srcdir)/../../src/include
index.html: $(header_dir)/*.h $(header_dir)/ycp/*.h
	@ybindir@/ydoc libycp --private --url $(htmldir) $^
