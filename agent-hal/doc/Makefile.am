#
# Makefile.am for core/agent-hal/doc
#

SUBDIRS =

builtin = $(srcdir)/../src/HalAgent.cc
hbuiltin = ag_hal.html
$(hbuiltin): $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/makebuiltindocs -t "SCR Hal agent functions" -a $(builtin) > $@.new
	mv $@.new $@

compdir = $(yastdocdir)/components
comp_DATA = $(hbuiltin)

CLEANFILES = $(hbuiltin)
