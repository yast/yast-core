#
# Makefile.am for core/libyui/doc
#

SUBDIRS = events widgets examples FAQs

htmldir = $(docdir)/libyui

html_DATA =			\
	ui-arch.jpg		\
	ui-arch.html		\
	UI-additional.html	\
	YCP-UI-layout.html	\
	YCP-builtins-ui.html	\
	YCP-UI-richtext.html	\
	README.newapi

EXTRA_DIST = $(html_DATA)

builtin	= $(srcdir)/../src/YUI_builtins.cc

YCP-builtins-ui.html: $(builtin)
	@ybindir@/makebuiltindocs -t "UI built-in functions" $(builtin) > $@.new
	mv $@.new $@

CLEANFILES = YCP-builtins-ui.html
