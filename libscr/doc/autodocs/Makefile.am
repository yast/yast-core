
htmldir = $(docdir)/autodocs

html_DATA =			\
	SCRAgent.html		\
	SCRInterpreter.html	\
	SCRView.html		\
	heir.html		\
	index.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/scr/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc @RPMNAME@ *.h scr/*.h
	mv -f $(srcdir)/../../src/include/*.html .
