#
# Makefile.am for y2a_system/doc
#
# $Id$
#

SUBDIRS = autodocs

htmldir = $(docdir)

html_DATA = systemagent.txt passwdagent.txt ag_system-builtins.html
EXTRA_DIST = $(html_DATA)

builtin = $(srcdir)/../src/SystemAgent.cc

ag_system-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs -t "SCR System agent functions" -a $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = ag_system-builtins.html
