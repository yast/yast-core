#
# Makefile.am for core/agent-system/doc/autodocs
#

htmldir = $(docdir)/agent-system/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-system -l libycp -l liby2 -l libscr --url $(htmldir) $^
