#
# Makefile.am for core/libscr/doc/autodocs
#

htmldir = $(docdir)/libscr/autodocs

html_DATA = index.html $(wildcard *.html)

CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/scr/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc libscr SCR.h	\
		scr/SCRAgent.h scr/SCRInterpreter.h
	mv -f $(srcdir)/../../src/include/*.html .
