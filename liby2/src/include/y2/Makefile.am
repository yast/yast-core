#
# Makefile.am for core/liby2/src/include/y2
#

pkgincludedir = $(includedir)/y2

# these go to $(pkgincludedir)

pkginclude_HEADERS =							\
	RemoteProtocol.h RloginProtocol.h RshProtocol.h SuProtocol.h	\
	SudoProtocol.h TelnetProtocol.h					\
	Y2CCProgram.h Y2CCRemote.h Y2CCScript.h Y2CCSerial.h Y2CCStdio.h\
	Y2Component.h Y2ComponentBroker.h Y2ComponentCreator.h		\
	Y2ProgramComponent.h Y2RemoteComponent.h Y2ScriptComponent.h	\
	Y2SerialComponent.h Y2StdioComponent.h				\
	Y2PluginComponent.h Y2CCPlugin.h				\
	pathsearch.h

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
