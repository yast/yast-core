#
# Makefile.am for core/base/src
#

ybin_PROGRAMS = y2base

y2base_SOURCES = y2base.cc

YAST2CORE =				\
	$(plugindir)/libpy2plugin.la	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	$(plugindir)/libpy2program.la	\
	$(plugindir)/libpy2wfm.la	\
	$(plugindir)/libpy2scr.la	\
	$(srcdir)/../../libycp/src/libycp.la		\
	$(srcdir)/../../liby2/src/liby2.la		\
	$(srcdir)/../../libyui/src/libyui.la		\
	$(srcdir)/../../libscr/src/libscr.la

y2base_DEPENDENCIES = $(YAST2CORE)

#
# do not add any libraries needed by a plugin.
# link the plugin against any special library.
#

y2base_LDADD =				\
	-lpthread -lz			\
	-lutil

y2base_LDFLAGS =			\
	-Xlinker --whole-archive	\
	$(YAST2CORE)			\
	-Xlinker --no-whole-archive
