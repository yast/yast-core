#
# Makefile.am for core/base/src
#

AM_LDFLAGS = -L$(libdir)

ybin_PROGRAMS = y2base

y2base_SOURCES = y2base.cc

YAST2CORE =					\
	../../liby2/src/libpy2plugin.la		\
	../../liby2/src/libpy2stdio.la		\
	../../liby2/src/libpy2program.la	\
	../../wfm/src/libpy2wfm.la		\
	../../scr/src/libpy2scr.la		\
	../../libycp/src/libycp.la		\
	../../liby2/src/liby2.la		\
	../../libyui/src/libyui.la		\
	../../libscr/src/libscr.la		


y2base_DEPENDENCIES = $(YAST2CORE)

#
# do not add any libraries needed by a plugin.
# link the plugin against any special library.
#

y2base_LDADD = -L$(libdir) -ly2pm -ly2util

y2base_LDFLAGS =			\
	-Xlinker --whole-archive	\
	$(PERL_LDFLAGS)			\
	$(YAST2CORE)			\
	-Xlinker --no-whole-archive
