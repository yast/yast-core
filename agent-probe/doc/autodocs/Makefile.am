#
# Makefile.am for core/agent-probe/doc/autodocs
#

htmldir = $(docdir)/agent-probe/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-probe -l libycp -l liby2 -l libscr --url $(htmldir) $^
