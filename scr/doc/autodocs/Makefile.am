#
# Makefile.am for core/scr/doc/autodocs
#

htmldir = $(docdir)/scr/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

header_dir = $(srcdir)/../../src
index.html: $(header_dir)/*.h
	@ybindir@/ydoc scr -l libycp -l liby2 -l libscr --url $(htmldir) $^
