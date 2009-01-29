#
# Makefile.am for core/agent-dbus/doc
#

SUBDIRS =

builtin = $(srcdir)/../src/DbusAgent.cc
hbuiltin = ag_dbus.html
$(hbuiltin): $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/makebuiltindocs -t "SCR Dbus agent functions" -a $(builtin) > $@.new
	mv $@.new $@

compdir = $(yastdocdir)/components
comp_DATA = $(hbuiltin)

CLEANFILES = $(hbuiltin)
