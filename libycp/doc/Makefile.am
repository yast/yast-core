#
# Makefile.am for core/libycp/doc
#
SUBDIRS = ycp

htmldir = $(docdir)/libycp
builtin = $(wildcard $(srcdir)/../src/YCPBuiltin*.cc)
builtinxml := $(builtin:%.cc=%.xml)
xmlfiles := $(foreach xml,$(builtinxml),$(notdir $(xml)))
htmlfiles = $(wildcard html/.html)


html_DATA =			\
	html/yast2docs.css	\
	html/index.html		\
	$(htmlfiles)

html/index.html: $(xmlfiles) builtins.xml
	XML_CATALOG_FILES=@XML_CATALOG@ $(XSLTPROC) $(XSLTPROC_FLAGS) --xinclude @STYLESHEET_HTML@ builtins.xml


$(xmlfiles) : $(builtin)
	`pkg-config --variable=ybindir yast2-devtools`/ydoc2 -o $@.tmp ../src/$(subst xml,cc,$@)
	$(XSLTPROC) $(XSLTPROC_FLAGS) @STYLESHEET_YDOC@ $@.tmp > $@
	`pkg-config --variable=ybindir yast2-devtools`/replace_entities $@
	rm $@.tmp

html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a `pkg-config --variable=yast2dir yast2-devtools`/data/docbook/images html

EXTRA_DIST =  builtins.xml

CLEANFILES = $(xmlfiles)

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)
