#
# Makefile.am for core/libyui/src/include
#

pkgincludedir = $(includedir)/yui

pkginclude_HEADERS =				\
	YUIComponent.h				\
	YUI.h					\
	UI.h					\
	YUISymbols.h				\
						\
	YAlignment.h				\
	YBarGraph.h				\
	YCheckBox.h				\
	YColoredLabel.h				\
	YDumbTab.h				\
	YComboBox.h				\
	YContainerWidget.h			\
	YDate.h					\
	YDialog.h				\
	YDownloadProgress.h			\
	YEmpty.h				\
	YEvent.h				\
	YFrame.h				\
	YImage.h				\
	YIntField.h				\
	YLabel.h				\
	YLogView.h				\
	YMacroPlayer.h				\
	YMacroRecorder.h			\
	YMenuButton.h				\
	YMultiLineEdit.h			\
	YMultiProgressMeter.h			\
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
	YShortcut.h				\
	YShortcutManager.h			\
	YSimpleEventHandler.h			\
	YSlider.h				\
	YSpacing.h				\
	YSplit.h				\
	YSquash.h				\
	YTable.h				\
	YTextEntry.h				\
	YTime.h					\
	YTransText.h				\
	YTree.h					\
	YWidget.h				\
	YWidgetOpt.h				\
	YWizard.h				\
	TreeItem.h


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
