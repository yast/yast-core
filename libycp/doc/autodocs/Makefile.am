#
# Makefile.am for core/libycp/doc/autodocs
#

htmldir = $(docdir)/libycp/autodocs

html_DATA =								\
	YCPBasicInterpreter.html YCPBlock.html YCPBlockRep.html		\
	YCPBoolean.html YCPBooleanRep.html YCPBreakStatement.html	\
	YCPBreakStatementRep.html YCPBuiltin.html YCPBuiltinRep.html	\
	YCPBuiltinStatement.html YCPBuiltinStatementRep.html		\
	YCPByteblock.html YCPByteblockRep.html				\
	YCPContinueStatement.html YCPContinueStatementRep.html		\
	YCPDebugger.html YCPDeclAny.html YCPDeclAnyRep.html		\
	YCPDeclList.html YCPDeclListRep.html YCPDeclStruct.html		\
	YCPDeclStructRep.html YCPDeclTerm.html YCPDeclTermRep.html	\
	YCPDeclType.html YCPDeclTypeRep.html YCPDeclaration.html	\
	YCPDeclarationRep.html YCPDoWhileBlock.html			\
	YCPDoWhileBlockRep.html YCPElement.html YCPElementRep.html	\
	YCPError.html YCPErrorRep.html YCPEvaluationStatement.html	\
	YCPEvaluationStatementRep.html YCPFloat.html YCPFloatRep.html	\
	YCPIdentifier.html YCPIdentifierRep.html			\
	YCPIfThenElseStatement.html YCPIfThenElseStatementRep.html	\
	YCPInteger.html YCPIntegerRep.html YCPInterpreter.html		\
	YCPList.html YCPListRep.html YCPLocale.html YCPLocaleRep.html	\
	YCPMap.html YCPMapIterator.html YCPMapRep.html			\
	YCPNestedStatement.html YCPNestedStatementRep.html		\
	YCPParser.html YCPPath.html YCPPathRep.html			\
	YCPReturnStatement.html YCPReturnStatementRep.html		\
	YCPScanner.html YCPScope.html YCPScopeElement.html		\
	YCPScopeInstance.html YCPStatement.html YCPStatementRep.html	\
	YCPString.html YCPStringRep.html YCPSymbol.html			\
	YCPSymbolRep.html YCPTerm.html YCPTermRep.html YCPValue.html	\
	YCPValueRep.html YCPVoid.html YCPVoidRep.html			\
	YCPWhileBlock.html YCPWhileBlockRep.html			\
	heir.html index.html intro.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/include/*.h $(srcdir)/../../src/include/ycp/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc -p libycp *.h ycp/*.h
	mv -f $(srcdir)/../../src/include/*.html .
