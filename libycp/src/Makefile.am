#
# Makefile.am for core/libycp/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libycp\" -DYAST2DIR=\"${yast2dir}\"

lib_LTLIBRARIES = libycp.la

libycp_la_SOURCES = 								\
										\
	YCPBasicInterpreter.cc YCPBlock.cc YCPBoolean.cc YCPBreakStatement.cc	\
	YCPByteblock.cc YCPContinueStatement.cc YCPDeclAny.cc			\
	YCPDeclList.cc YCPDeclStruct.cc YCPDeclTerm.cc YCPDeclType.cc		\
	YCPDeclaration.cc YCPDoWhileBlock.cc YCPElement.cc			\
	YCPEvaluationStatement.cc YCPFloat.cc YCPIfThenElseStatement.cc		\
	YCPInteger.cc YCPInterpreter.cc YCPList.cc YCPLocale.cc YCPMap.cc	\
	YCPParser.cc YCPPath.cc YCPReturnStatement.cc YCPScanner.cc		\
	YCPNestedStatement.cc YCPBuiltinStatement.cc				\
	YCPStatement.cc YCPString.cc YCPSymbol.cc YCPTerm.cc			\
	YCPValue.cc YCPVoid.cc YCPWhileBlock.cc parser.yy scanner.ll y2log.cc	\
	YCPScope.cc YCPIdentifier.cc YCPDebugger.cc YCPScopeInstance.cc		\
	YCPError.cc								\
	YCPBuiltin.cc YCPBuiltinInteger.cc YCPBuiltinPath.cc			\
	YCPBuiltinFloat.cc YCPBuiltinList.cc					\
	YCPBuiltinString.cc YCPBuiltinTerm.cc					\
	YCPBuiltinMap.cc YCPBuiltinMisc.cc					\
	y2crypt.cc y2crypt.h y2string.cc y2string.h

$(srcdir)/scanner.cc: scanner.ll $(srcdir)/parser.h
	flex++ -o$@ $<

$(srcdir)/parser.h $(srcdir)/parser.cc: parser.yy
	bison -y -d -v $<
	mv -f y.tab.c $(srcdir)/parser.cc
	mv -f y.tab.h $(srcdir)/parser.h

$(srcdir)/hashtable.h: hashtable.gperf
	gperf -k 3,5,7 -L ANSI-C -C -E -I -t <$< >$@.new
	mv -f $@.new $@

BUILT_SOURCES = parser.cc scanner.cc parser.h hashtable.h

INCLUDES = -I.. -I$(srcdir)/include -I$(srcdir)/include/ycp -I$(includedir)

libycp_la_LDFLAGS = -version-info 2:0:0

libycp_la_LIBADD = -lxcrypt -ly2util

EXTRA_DIST = scanner.ll parser.yy hashtable.gperf

CLEANFILES = y.output $(BUILT_SOURCES)
