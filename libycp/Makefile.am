#
# Makefile.am for libycp
#

SUBDIRS = src doc testsuite

VERSION = $(shell cat $(srcdir)/VERSION)

RPMNAME = $(shell cat $(srcdir)/RPMNAME)

EXTRA_DIST = COPYRIGHT.english COPYRIGHT.french COPYRIGHT.german \
	VERSION README RPMNAME MAINTAINER $(RPMNAME).spec.in

AUTOMAKE_OPTIONS = foreign dist-bzip2

$(RPMNAME)-$(VERSION).tar.bz2: dist-bzip2

CLEANFILES = $(RPMNAME).kdoc
MAINTAINERCLEANFILES = package/$(RPMNAME)-$(VERSION).tar.bz2 package/$(RPMNAME).spec

package: $(RPMNAME)-$(VERSION).tar.bz2
	rm -f package/$(RPMNAME)-*.tar.bz2
	mv $< package
