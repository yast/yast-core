#
# Makefile.am for core/libycp/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libycp\" -DYAST2DIR=\"${yast2dir}\"

lib_LTLIBRARIES = libycp.la

libycp_la_SOURCES =			\
					\
	YCPBasicInterpreter.cc		\
	YCPBlock.cc			\
	YCPBoolean.cc			\
	YCPBreakStatement.cc		\
	YCPBuiltin.cc			\
	YCPBuiltinFloat.cc		\
	YCPBuiltinInteger.cc		\
	YCPBuiltinList.cc		\
	YCPBuiltinMap.cc		\
	YCPBuiltinMisc.cc		\
	YCPBuiltinPath.cc		\
	YCPBuiltinStatement.cc		\
	YCPBuiltinString.cc		\
	YCPBuiltinTerm.cc		\
	YCPByteblock.cc			\
	YCPContinueStatement.cc		\
	YCPDebugger.cc			\
	YCPDeclAny.cc			\
	YCPDeclList.cc			\
	YCPDeclStruct.cc		\
	YCPDeclTerm.cc			\
	YCPDeclType.cc			\
	YCPDeclaration.cc		\
	YCPDoWhileBlock.cc		\
	YCPElement.cc			\
	YCPError.cc			\
	YCPEvaluationStatement.cc	\
	YCPFloat.cc			\
	YCPIdentifier.cc		\
	YCPIfThenElseStatement.cc	\
	YCPInteger.cc			\
	YCPInterpreter.cc		\
	YCPList.cc			\
	YCPLocale.cc			\
	YCPMap.cc			\
	YCPNestedStatement.cc		\
	YCPParser.cc			\
	YCPPath.cc			\
	YCPReturnStatement.cc		\
	YCPScanner.cc			\
	YCPScope.cc			\
	YCPScopeInstance.cc		\
	YCPStatement.cc			\
	YCPString.cc			\
	YCPSymbol.cc			\
	YCPTerm.cc			\
	YCPValue.cc			\
	YCPVoid.cc			\
	YCPWhileBlock.cc		\
	parser.yy			\
	scanner.ll			\
	y2crypt.cc			\
	y2crypt.h			\
	y2string.cc			\
	y2string.h

$(srcdir)/scanner.cc: scanner.ll $(srcdir)/parser.h
	flex++ -o$@ $<

# we need a pattern rule - otherwise bison may be run twice in parallel
$(srcdir)/%.h $(srcdir)/%.cc: %.yy
	bison -y -d -v $<
	mv -f y.tab.c $(srcdir)/$*.cc
	mv -f y.tab.h $(srcdir)/$*.h

$(srcdir)/hashtable.h: hashtable.gperf
	gperf -k 3,5,7 -L ANSI-C -C -E -I -t <$< >$@.new
	mv -f $@.new $@

BUILT_SOURCES = parser.cc scanner.cc parser.h hashtable.h

INCLUDES = -I.. -I$(srcdir)/include -I$(srcdir)/include/ycp -I$(includedir)

libycp_la_LDFLAGS = -version-info 2:0:0

libycp_la_LIBADD = -lxcrypt -ly2util

EXTRA_DIST = scanner.ll parser.yy hashtable.gperf

CLEANFILES = y.output $(BUILT_SOURCES)
