#
# Makefile.am for core/libycp/src/include/ycp
#

pkgincludedir = $(includedir)/ycp

# these go to $(pkgincludedir)

pkginclude_HEADERS =								\
										\
	YCPBasicInterpreter.h YCPScope.h YCPBlock.h YCPBoolean.h		\
	YCPBreakStatement.h YCPByteblock.h YCPContinueStatement.h		\
	YCPDeclAny.h YCPDeclList.h YCPDeclStruct.h				\
	YCPDeclTerm.h YCPDeclType.h YCPDeclaration.h YCPDoWhileBlock.h		\
	YCPElement.h YCPEvaluationStatement.h YCPFloat.h			\
	YCPIfThenElseStatement.h YCPInteger.h YCPInterpreter.h YCPList.h	\
	YCPNestedStatement.h YCPBuiltinStatement.h				\
	YCPLocale.h YCPMap.h YCPParser.h YCPPath.h YCPReturnStatement.h		\
	YCPScanner.h YCPStatement.h YCPString.h YCPSymbol.h YCPTerm.h		\
	YCPValue.h YCPVoid.h YCPWhileBlock.h parserret.h toString.h		\
	YCPBuiltin.h YCPError.h	YCPIdentifier.h	YCPDebugger.h			\
	YCPScopeInstance.h							\
	y2log.h ycpless.h

#<INSTALL-HEADER-TARGET>

# Automatically inserted by insert_install_target
# DO NOT REMOVE THE TAGS!

# Install header files only if different to version already installed -
# prevent unnecessary installations to keep make dependency changes
# and thus rebuilds for dependent packages as low as possible.

install-pkgincludeHEADERS: $(pkginclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgincludedir)
	@list='$(pkginclude_HEADERS)';							\
	for p in $$list; do								\
		if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; 			\
		if ! cmp -s $$d$$p $(DESTDIR)$(pkgincludedir)/$$p; then			\
			echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p";	\
			$(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p;		\
	  	fi									\
	done

#</INSTALL-HEADER-TARGET>
