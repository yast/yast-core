#
# Makefile.am for y2scr/ag_resolver/doc
#
SUBDIRS = autodocs

htmldir = $(docdir)/ag_resolver
html_DATA = ag_resolver-builtins.html resolveragent.txt

builtin = $(srcdir)/../src/ResolverAgent.cc

EXTRA_DIST = $(html_DATA)

ag_resolver-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs -t "SCR Resolver agent built-in functions" $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = ag_resolver-builtins.html
