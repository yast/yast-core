#
# Makefile.am for libycp
#

SUBDIRS = include

# for pathsearch.cc
# FIXME use the standard defines
AM_CXXFLAGS = -DY2LOG=\"libycp\"			\
	-DYAST2DIR=\"$(prefix)/share/YaST2\"		\
	-DPLUGINDIR=\"${plugindir}\"		\
	-DEXECCOMPDIR=\"${execcompdir}\"	\
	-DLOCALEDIR=\"${localedir}\"

lib_LTLIBRARIES = libycp.la

#
# Sources, in REVERSE constructor order
#   -> initialize y2log FIRST
#

libycp_la_SOURCES = 					\
	pathsearch.cc					\
	ExecutionEnvironment.cc				\
	Bytecode.cc					\
	StaticDeclaration.cc YCode.cc YCPCode.cc	\
	YExpression.cc YStatement.cc YBlock.cc		\
	SymbolTable.cc					\
	Scanner.cc Parser.cc 				\
	YCPBoolean.cc					\
	YCPElement.cc YCPByteblock.cc YCPFloat.cc	\
	YCPInteger.cc YCPList.cc 			\
	YCPMap.cc YCPPath.cc				\
	YCPString.cc YCPSymbol.cc YCPTerm.cc		\
	YCPValue.cc YCPVoid.cc parser.yy scanner.ll	\
	YBuiltin.cc YCPBuiltinInteger.cc		\
	YCPBuiltinByteblock.cc				\
	YCPBuiltinPath.cc YCPBuiltinFloat.cc		\
	YCPBuiltinList.cc YCPBuiltinBoolean.cc		\
	YCPBuiltinString.cc YCPBuiltinTerm.cc		\
	YCPBuiltinVoid.cc YCPBuiltinMap.cc		\
	YCPBuiltinMisc.cc				\
	SymbolEntry.cc					\
	Type.cc TypeStatics.cc				\
	y2string.cc y2string.h				\
	y2crypt.cc y2crypt.h

$(srcdir)/scanner.cc: scanner.ll parser.h
	flex++ -o$@ $<

$(srcdir)/parser.h $(srcdir)/parser.cc: parser.yy
	bison -y -d -v $<
	mv -f y.tab.c $(srcdir)/parser.cc
	mv -f y.tab.h $(srcdir)/parser.h

BUILT_SOURCES = parser.cc scanner.cc parser.h

INCLUDES = -I.. -I$(srcdir)/include -I$(srcdir)/include/ycp -I$(srcdir)/../../liby2/src/include -I$(includedir) -I/usr/include/YaST2

libycp_la_LDFLAGS = -L$(libdir) -version-info 2:0

libycp_la_LIBADD = -lxcrypt -ly2util

EXTRA_DIST = scanner.ll $(BUILT_SOURCES)

CLEANFILES = y.output $(BUILT_SOURCES)
