#
# Makefile.am for core/agent-system/doc
#

SUBDIRS =

htmldir = $(docdir)/agent-system
html_DATA = systemagent.txt passwdagent.txt
EXTRA_DIST = $(html_DATA)

builtin = $(srcdir)/../src/SystemAgent.cc
hbuiltin = ag_system.html
$(hbuiltin): $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/makebuiltindocs -t "SCR System agent functions" -a $(builtin) > $@.new
	mv $@.new $@

compdir = $(yastdocdir)/components
comp_DATA = $(hbuiltin)

CLEANFILES = $(hbuiltin)
