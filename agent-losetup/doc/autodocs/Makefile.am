#
# Makefile.am for core/agent-losetup/doc/autodocs
#

htmldir = $(docdir)/agent-losetup/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-losetup -l libycp -l liby2 -l libscr --url $(htmldir) $^
