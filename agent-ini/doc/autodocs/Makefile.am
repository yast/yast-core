#
# Makefile.am for core/agent-ini/doc/autodocs
#

htmldir = $(docdir)/agent-ini/autodocs

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-ini *.h
	mv -f $(srcdir)/../../src/*.html .
