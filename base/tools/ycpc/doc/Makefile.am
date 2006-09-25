#
# Makefile.am for core/y2base/tools/ycpc
#
htmldir = $(docdir)/ycpc
xmlfiles = ycpc.xml ycpc-data.xml
htmlfiles = $(wildcard html/.html)


html_DATA =			\
	html/yast2docs.css	\
	html/index.html		\
	$(htmlfiles)

html/index.html: $(xmlfiles)
	XML_CATALOG_FILES=@XML_CATALOG@ $(XSLTPROC) $(XSLTPROC_FLAGS) --xinclude @STYLESHEET_HTML@ ycpc.xml


html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a `pkg-config --variable=yast2dir yast2-devtools`/data/docbook/images html

EXTRA_DIST = $(xmlfiles)

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)
