#
# Makefile.am for core/agent-resolver/doc/autodocs
#

htmldir = $(docdir)/agent-resolver/autodocs

html_DATA =				\
	heir.html index.html		\
	ResolverAgent.html		\
	Y2ResolverComponent.html	\
	Y2CCResolver.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/ResolverAgent.cc $(srcdir)/../../src/ResolverAgent.h
	cd $(srcdir)/../../src && @ybindir@/ydoc -p agent-resolver *.h *.cc
	mv -f $(srcdir)/../../src/*.html .
