#
# Makefile.am for core/agent-ypserv/doc/autodocs
#

htmldir = $(docdir)/agent-ypserv/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-ypserv -l libycp -l liby2 -l libscr --url $(htmldir) $^
