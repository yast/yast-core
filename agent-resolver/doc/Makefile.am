#
# Makefile.am for core/agent-resolver/doc
#

SUBDIRS =

htmldir = $(docdir)/agent-resolver

html_DATA = ag_resolver-builtins.html resolveragent.txt

builtin = $(srcdir)/../src/ResolverAgent.cc

EXTRA_DIST = $(html_DATA)

ag_resolver-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs -t "SCR Resolver agent built-in functions" $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = ag_resolver-builtins.html
