#
# Makefile.am for core/agent-ini/doc/autodocs
#

htmldir = $(docdir)/agent-ini/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	@ybindir@/ydoc agent-ini -l libycp -l liby2 -l libscr --url $(htmldir) $^
