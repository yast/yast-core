#
# Makefile.am for y2scr/ag_losetup/doc
#
SUBDIRS = autodocs

htmldir = $(docdir)/ag_losetup
html_DATA = ag_losetup-builtins.html

builtin = $(srcdir)/../src/LosetupAgent.cc

EXTRA_DIST = $(html_DATA)

ag_losetup-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs -t "SCR losetup agent built-in functions" $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = $(html_DATA)
