#
# Makefile.am for liby2/doc
#

htmldir = $(docdir)/autodocs

html_DATA =			\
	ExternalDataSource.html	\
	ExternalProgram.html	\
	RemoteProtocol.html	\
	RloginProtocol.html	\
	RshProtocol.html	\
	SuProtocol.html		\
	SudoProtocol.html	\
	TelnetProtocol.html	\
	Y2CCProgram.html	\
	Y2CCRemote.html		\
	Y2CCScript.html		\
	Y2CCSerial.html		\
	Y2CCStdio.html		\
	Y2Component.html	\
	Y2ComponentBroker.html	\
	Y2ComponentCreator.html	\
	Y2ProgramComponent.html	\
	Y2RemoteComponent.html	\
	Y2ScriptComponent.html	\
	Y2SerialComponent.html	\
	Y2StdioComponent.html	\
	Y2PathSearch.html	\
	heir.html		\
	index.html		\
	intro.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/y2/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc -p @RPMNAME@ *.h y2/*.h
	mv -f $(srcdir)/../../src/include/*.html .
