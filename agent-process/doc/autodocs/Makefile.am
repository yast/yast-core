#
# Makefile.am for core/agent-process/doc
#

SUBDIRS =

htmldir = $(docdir)/agent-process

html_DATA = ag_process-builtins.html
EXTRA_DIST = $(html_DATA)

builtin = $(srcdir)/../../src/ProcessAgent.cc

ag_process-builtins.html: $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/makebuiltindocs -t "SCR Process agent functions" -a $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = ag_process-builtins.html
