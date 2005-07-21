# Makefile.am for core/agent-resolver/src

AM_CXXFLAGS = -DY2LOG=\"agent-resolver\"

plugin_LTLIBRARIES = libpy2ag_resolver.la

libpy2ag_resolver_la_SOURCES =	\
	ResolverAgent.cc	\
	ResolverAgent.h		\
	Y2CCResolverAgent.cc

libpy2ag_resolver_la_LDFLAGS = -version-info 2:0
libpy2ag_resolver_la_LIBADD = @AGENT_LIBADD@

INCLUDES = ${AGENT_INCLUDES}
