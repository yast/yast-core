#
# Makefile.am for core/agent-dummy/doc/autodocs
#

htmldir = $(docdir)/agent-dummy/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-dummy -l libycp -l liby2 -l libscr --url $(htmldir) $^
