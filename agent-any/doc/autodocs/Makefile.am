#
# Makefile.am for core/agent-any/doc/autodocs
#

htmldir = $(docdir)/agent-any/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-any -l libycp -l liby2 -l libscr --url $(htmldir) $^
