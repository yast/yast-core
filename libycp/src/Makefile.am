#
# Makefile.am for libycp
#

SUBDIRS = include

# for pathsearch.cc
# FIXME use the standard defines
AM_CXXFLAGS = -DYAST2DIR=\"/usr/share/YaST2\"		\
	-DPLUGINDIR=\"/usr/lib/YaST2/plugin\"		\
	-DEXECCOMPDIR=\"/usr/lib/YaST2/plugin\"

lib_LTLIBRARIES = libycp.la

#
# Sources, in REVERSE constructor order
#   -> initialize y2log FIRST
#

libycp_la_SOURCES = 					\
	pathsearch.cc					\
	Bytecode.cc					\
	StaticDeclaration.cc YCode.cc YCPCode.cc	\
	YExpression.cc YStatement.cc YBlock.cc		\
	SymbolTable.cc					\
	Scanner.cc Parser.cc 				\
	YCPBoolean.cc					\
	YCPElement.cc YCPByteblock.cc YCPFloat.cc	\
	YCPInteger.cc YCPList.cc			\
	YCPMap.cc YCPPath.cc				\
	YCPString.cc YCPSymbol.cc YCPTerm.cc		\
	YCPValue.cc YCPVoid.cc parser.yy scanner.ll	\
	YCPError.cc					\
	YBuiltin.cc YCPBuiltinInteger.cc		\
	YCPBuiltinPath.cc YCPBuiltinFloat.cc		\
	YCPBuiltinList.cc YCPBuiltinBoolean.cc		\
	YCPBuiltinString.cc YCPBuiltinTerm.cc		\
	YCPBuiltinVoid.cc YCPBuiltinMap.cc		\
	YCPBuiltinMisc.cc				\
	SymbolEntry.cc					\
	TypeCode.cc					\
	y2log.cc

$(srcdir)/scanner.cc: scanner.ll parser.h
	flex++ -o$@ $<

BUILT_SOURCES = parser.cc scanner.cc parser.h

INCLUDES = -I.. -I$(srcdir)/include -I$(srcdir)/include/ycp

libycp_la_LDFLAGS = -version-info 3:0

libycp_la_LIBADD = -lcrypt

EXTRA_DIST = scanner.ll $(BUILT_SOURCES)

CLEANFILES = y.output $(BUILT_SOURCES)
