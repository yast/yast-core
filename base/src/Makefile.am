#
# Makefile.am for core/base/src
#

ybin_PROGRAMS = y2base

y2base_SOURCES = y2base.cc

YAST2CORE =						\
	$(srcdir)/../../liby2/src/libpy2plugin.la	\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../../liby2/src/libpy2program.la	\
	$(srcdir)/../../wfm/src/libpy2wfm.la		\
	$(srcdir)/../../scr/src/libpy2scr.la		\
	$(srcdir)/../../libycp/src/libycp.la		\
	$(srcdir)/../../liby2/src/liby2.la		\
	$(srcdir)/../../libyui/src/libyui.la		\
	$(srcdir)/../../libscr/src/libscr.la		


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
