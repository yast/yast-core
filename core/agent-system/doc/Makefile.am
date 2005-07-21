#
# Makefile.am for core/agent-system/doc
#

SUBDIRS =

htmldir = $(docdir)/agent-system

html_DATA = systemagent.txt passwdagent.txt ag_system-builtins.html
EXTRA_DIST = $(html_DATA)

builtin = $(srcdir)/../src/SystemAgent.cc

ag_system-builtins.html: $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/makebuiltindocs -t "SCR System agent functions" -a $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = ag_system-builtins.html
