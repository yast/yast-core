#
# Makefile.am for core/libyui/doc/autodocs
#

htmldir = $(docdir)/libyui/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h
	@ybindir@/ydoc libyui --xref libycp --xref liby2 --url $(htmldir) $^
