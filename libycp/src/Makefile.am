#
# Makefile.am for libycp
#

SUBDIRS = include

# for pathsearch.cc
# FIXME use the standard defines
AM_CXXFLAGS = -DY2LOG=\"libycp\"		\
	-DYAST2DIR=\"$(prefix)/share/YaST2\"	\
	-DPLUGINDIR=\"${plugindir}\"		\
	-DEXECCOMPDIR=\"${execcompdir}\"	\
	-DLOCALEDIR=\"${localedir}\"

lib_LTLIBRARIES = libycpvalues.la libycp.la

#
# Sources, in REVERSE constructor order
#   -> initialize y2log FIRST
#

libycpvalues_la_SOURCES = 				\
	Bytecode.cc Import.cc Point.cc			\
	YCPBoolean.cc					\
	YCPElement.cc YCPByteblock.cc YCPFloat.cc	\
	YCPInteger.cc YCPList.cc 			\
	YCPMap.cc YCPPath.cc				\
	YCPString.cc YCPSymbol.cc YCPTerm.cc		\
	YCPValue.cc YCPVoid.cc 				\
	Type.cc 

libycp_la_SOURCES = 					\
	pathsearch.cc					\
	ExecutionEnvironment.cc				\
	StaticDeclaration.cc YCode.cc YCPCode.cc	\
	YExpression.cc YStatement.cc YBlock.cc		\
	SymbolTable.cc					\
	Scanner.cc Parser.cc 				\
	parser.yy scanner.ll				\
	YBuiltin.cc YCPBuiltinInteger.cc		\
	YCPBuiltinByteblock.cc				\
	YCPBuiltinPath.cc YCPBuiltinFloat.cc		\
	YCPBuiltinList.cc YCPBuiltinBoolean.cc		\
	YCPBuiltinString.cc YCPBuiltinTerm.cc		\
	YCPBuiltinVoid.cc YCPBuiltinMap.cc		\
	YCPBuiltinMisc.cc				\
	YSymbolEntry.cc					\
	TypeStatics.cc					\
	y2string.cc y2string.h				\
	y2crypt.cc y2crypt.h

$(srcdir)/scanner.cc: scanner.ll parser.h
	flex++ -o$@ $<

BUILT_SOURCES = parser.h

AM_YFLAGS = -d -v

INCLUDES = -I$(srcdir)/include -I$(srcdir)/include/ycp -I$(top_srcdir)/liby2/src/include ${Y2UTIL_CFLAGS}

libycpvalues_la_LDFLAGS = -version-info 2:0

libycpvalues_la_LIBADD = ${Y2UTIL_LIBS} 

libycp_la_LDFLAGS = -version-info 2:0

libycp_la_LIBADD = ${Y2UTIL_LIBS} -lxcrypt libycpvalues.la

CLEANFILES = parser.output parser.cc scanner.cc $(BUILT_SOURCES)
