#
# Makefile.am for packager/doc/autodocs
#

htmldir = $(docdir)/packager/autodocs

html_DATA = \
	Rpm_Installer.html		\
	Y2CCPackager.html		\
	Y2PackagerComponent.html	\
	heir.html index.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src; @ybindir@/ydoc -p @RPMNAME@ *.h
	mv -f $(srcdir)/../../src/*.html .

