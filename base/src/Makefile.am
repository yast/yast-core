#
# Makefile.am for core/base/src
#

ybin_PROGRAMS = y2base

y2base_SOURCES = y2base.cc

YAST2CORE =					\
	../../wfm/src/libpy2wfm.la		\
        ../../scr/src/libpy2scr.la              \
	../../libycp/src/libycp.la		\
        ../../libycp/src/libycpvalues.la        \
	../../liby2/src/liby2.la		\
	../../debugger/liby2debug.la	\
	../../libscr/src/libscr.la		


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

scr_LDADD = ${AGENT_LIBADD}

scr_LDFLAGS =                                   \
        -Xlinker --whole-archive                \
        $(top_builddir)/scr/src/libpy2scr.la            \
	$(top_builddir)/wfm/src/libpy2wfm.la		\
        -Xlinker --no-whole-archive
