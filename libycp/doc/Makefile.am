#
# Makefile.am for core/libycp/doc
#
SUBDIRS = ycp
htmldir = $(docdir)/libycp

html_DATA =			\
	YCP-builtins.html	\
	changes.html		\
	traps.html		\
	logging.html

builtin = $(wildcard $(srcdir)/../src/YCPBuiltin*.cc)

YCP-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs $^ > $@.new && mv $@.new $@

EXTRA_DIST = $(html_DATA)

CLEANFILES = CATALOG.local *.out *.aux *.dvi *.tex *.ps *.pdf *.log *.dsl \
	YCP-builtins.html

clean-local:
	rm -rf ycp.junk
	rm -rf ycp
