#
# Makefile.am for core/agent-modules/doc/autodocs
#

htmldir = $(docdir)/agent-modules/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-modules -l libycp -l liby2 -l libscr --url $(htmldir) $^
