#
# Makefile.am for core/agent-resolver/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-resolver\"

plugin_LTLIBRARIES = libpy2ag_resolver.la

libpy2ag_resolver_la_SOURCES = \
	ResolverAgent.cc ResolverAgent.h	\
	Y2CCResolverAgent.h

libpy2ag_resolver_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
