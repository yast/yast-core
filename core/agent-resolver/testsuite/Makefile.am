# Makefile.am for core/agent-resolver/testsuite

SUBDIRS = config lib ag_resolver.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-resolver\"

PACKAGE = ag_resolver
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runresolver

runresolver_SOURCES = runresolver.cc
runresolver_LDADD = ${AGENT_LIBADD}
runresolver_LDFLAGS = 					\
	-Xlinker --whole-archive			\
	$(top_builddir)/liby2/src/libpy2stdio.la	\
	../src/libpy2ag_resolver.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* site.exp *.log *.sum *.bak

