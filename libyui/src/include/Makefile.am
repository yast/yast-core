#
# Makefile.am for core/libyui/src/include
#

pkgincludedir = $(includedir)/yui

pkginclude_HEADERS =				\
	YUIInterpreter.h			\
	YUISymbols.h				\
	hashtable.gperf				\
	hashtable.h 				\
	YMacroRecorder.h			\
	YMacroPlayer.h				\
	YShortcutManager.h			\
	YShortcut.h				\
	YAlignment.h				\
	YBarGraph.h				\
	YCheckBox.h				\
	YColoredLabel.h				\
	YComboBox.h				\
	YContainerWidget.h			\
	YDialog.h				\
	YDownloadProgress.h			\
	YEmpty.h				\
	YFrame.h				\
	YImage.h				\
	YIntField.h				\
	YLabel.h				\
	YLogView.h				\
	YMenuButton.h				\
	YMultiLineEdit.h			\
	YMultiSelectionBox.h			\
	YPackageSelector.h			\
	YPartitionSplitter.h			\
	YProgressBar.h				\
	YPushButton.h				\
	YRadioButton.h				\
	YRadioButtonGroup.h			\
	YReplacePoint.h				\
	YRichText.h				\
	YSelectionBox.h				\
	YSlider.h				\
	YSpacing.h				\
	YSplit.h				\
	YSquash.h				\
	YTable.h				\
	YTextEntry.h				\
	YTransText.h				\
	YTree.h					\
	YWidget.h				\
	YWidgetOpt.h				\
	TreeItem.h


hashtable.h: hashtable.gperf
	gperf -k 1,7,9 -H YUIInterpreter_hash -N YUIInterpreter_in_word_set -L ANSI-C -C -E -I <$< > hashtable.out && mv hashtable.out $@ || (rm -f hashtable.out ; false)

CLEANFILES = libyui.kdoc

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
