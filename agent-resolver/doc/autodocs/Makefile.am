#
# Makefile.am for core/agent-resolver/doc/autodocs
#

htmldir = $(docdir)/agent-resolver/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc -p agent-resolver *.h *.cc
	mv -f $(srcdir)/../../src/*.html .
