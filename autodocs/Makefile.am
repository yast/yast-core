#
# Makefile.am for core/autodocs
#
htmldir = $(docdir)/autodocs

html_DATA = index.html $(wildcard *.html *.png *.gif *.css) @RPMNAME@.tag

CLEANFILES = $(html_DATA) doxygen.log doxygen.conf installdox

DOC=/usr/share/doc/packages

doxygen.css @RPMNAME@.tag: index.html

index.html: $(wildcard $(top_srcdir)/*/*.cc $(top_srcdir)/*/*/*.cc)
	${YDOXYGEN} PROJECT_NAME=@RPMNAME@ \
		INPUT=.. 
# TODO: create libzypp.tag and use it (elsewhere)
