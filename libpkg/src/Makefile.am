#
# Makefile.am for libpkg/src
#

SUBDIRS = include

lib_LTLIBRARIES = libpkg.la

libpkg_la_SOURCES =  \
	TagParser.h TagParser.cc	\
	TagSets.h	\
	Pathname.h Pathname.cc		\
	PathInfo.h PathInfo.cc		\
	Version.h  Version.cc		\
	RawPackage.h RawPackage.cc	\
	ConfigFile.cc RawPackageInfo.cc Solver.cc PackageKey.cc Package.cc

INCLUDES = -I$(srcdir)/include -I$(includedir)

libpkg_la_LDFLAGS = -version-info 2:0
