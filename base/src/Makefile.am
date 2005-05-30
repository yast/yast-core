#
# Makefile.am for core/base/src
#

ybin_PROGRAMS = y2base

y2base_SOURCES = y2base.cc

YAST2CORE =					\
	../../liby2/src/libpy2plugin.la		\
	../../liby2/src/libpy2stdio.la		\
	../../liby2/src/libpy2program.la	\
	../../wfm/src/libpy2wfm.la		\
	../../libycp/src/libycp.la		\
	../../liby2/src/liby2.la		\
	../../libyui/src/libyui.la		\
	../../libscr/src/libscr.la		

#	../../scr/src/libpy2scr.la		


y2base_DEPENDENCIES = $(YAST2CORE)

#
# do not add any libraries needed by a plugin.
# link the plugin against any special library.
#

y2base_LDADD = ${Y2UTIL_LIBS}

y2base_LDFLAGS =			\
	-Xlinker --whole-archive	\
	$(PERL_LDFLAGS)			\
	$(YAST2CORE)			\
	-Xlinker --no-whole-archive

AM_CXXFLAGS = -DY2LOG=\"scr\" -DSUSEVERSION=\"${SUSEVERSION}\"

serversdir = $(execcompdir)/servers
servers_PROGRAMS = scr

INCLUDES = ${AGENT_INCLUDES}                 		\
	-I$(srcdir)/../../libscr/src/include		\
	-I$(srcdir)/../../scr/src

scr_SOURCES =                                           \
        scr.cc

scr_LDADD = ${AGENT_LIBADD}                             \
        $(top_builddir)/liby2/src/libpy2stdio.la        \
        $(top_builddir)/liby2/src/libpy2plugin.la

scr_LDFLAGS =                                   \
        -Xlinker --whole-archive                \
        $(top_builddir)/liby2/src/libpy2plugin.la       \
        $(top_builddir)/liby2/src/libpy2stdio.la        \
        $(top_builddir)/liby2/src/libpy2program.la      \
        $(top_builddir)/scr/src/libpy2scr.la            \
	$(top_builddir)/wfm/src/libpy2wfm.la		\
        -Xlinker --no-whole-archive
