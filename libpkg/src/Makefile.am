#
# Makefile.am for core/libpkg/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libpkg\"

lib_LTLIBRARIES = libpkg.la

libpkg_la_SOURCES =  \
	TagParser.h TagParser.cc	\
	TagSets.h	\
	Pathname.h Pathname.cc		\
	PathInfo.h PathInfo.cc		\
	Version.h  Version.cc		\
	RawPackage.h RawPackage.cc	\
	ConfigFile.cc RawPackageInfo.cc Solver.cc PackageKey.cc Package.cc

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/include

libpkg_la_LDFLAGS = -version-info 2:0
