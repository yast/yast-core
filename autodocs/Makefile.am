#
# Makefile.am for core/autodocs
#
htmldir = $(docdir)/autodocs

html_DATA = index.html $(wildcard *.html *.png) doxygen.css @RPMNAME@.tag

CLEANFILES = $(html_DATA) doxygen.log doxygen.conf installdox

DOC=/usr/share/doc/packages

doxygen.css @RPMNAME@.tag: index.html

index.html: $(wildcard $(top_srcdir)/*/*.cc $(top_srcdir)/*/*/*.cc)
	@ybindir@/ydoxygen PROJECT_NAME=@RPMNAME@ \
		INPUT=.. \
		"TAGFILES=\
		\"$(DOC)/liby2util/autodocs/liby2util.tag = \
		  $(DOC)/liby2util/autodocs\" \
		\"$(DOC)/yast2-packagemanager/autodocs/yast2-packagemanager.tag = \
		$(DOC)/yast2-packagemanager/autodocs\" \
		"
