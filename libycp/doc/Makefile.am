#
# Makefile.am for core/libycp/doc
#

htmldir = $(docdir)/libycp

html_DATA =					\
	ycp/book1.html				\
	changes.html syntax.html		\
	bracket.html traps.html			\
	logging.html debugger.html		\
	modules.html				\
	YCP-builtins.html			\
	YCP-datatypes.html			\
	YCP-quoting.html YCP-statements.html	\
	YCP-operators.html

builtin = $(wildcard $(srcdir)/../src/YCPBuiltin*.cc)

YCP-builtins.html: $(builtin)
	@ybindir@/makebuiltindocs $(builtin) > $@.new && mv $@.new $@

ycp/book1.html: ycp.xml $(wildcard sections/*.xml)
	db2html ycp.xml -o ycp

EXTRA_DIST = *.xml $(html_DATA)

CLEANFILES = CATALOG.local *.out *.aux *.dvi *.tex *.ps *.pdf *.log *.dsl \
	YCP-builtins.html

clean-local:
	rm -rf ycp.junk
	rm -rf ycp
