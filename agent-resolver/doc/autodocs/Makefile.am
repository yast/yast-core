#
# Makefile.am for core/agent-resolver/doc/autodocs
#

htmldir = $(docdir)/agent-resolver/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-resolver -l libycp -l liby2 -l libscr --url $(htmldir) $^
