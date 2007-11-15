#
# Makefile.am for core/libyui/src
#

AM_CXXFLAGS = -DY2LOG=\"libycp\"		\
	-DTHEMEDIR=\"${themedir}/current\"	\
	-DPLUGINDIR=\"${plugindir}\"		\
	-fmessage-length=0

plugin_LTLIBRARIES = libpy2UI.la

libpy2UI_la_SOURCES =				\
	Y2UINamespace.cc 			\
	Y2CCUI.cc				\
	YApplication.cc				\
	YCPDialogParser.cc			\
	YCPItemParser.cc			\
	YCPItemWriter.cc			\
	YCPMenuItemParser.cc			\
	YCPMenuItemWriter.cc			\
	YCPTableItemParser.cc			\
	YCPTableItemWriter.cc			\
	YCPTreeItemParser.cc			\
	YCPTreeItemWriter.cc			\
	YCPPropertyHandler.cc			\
	YCPValueWidgetID.cc			\
	YItem.cc				\
	YTreeItem.cc				\
	YTableItem.cc				\
	YTableHeader.cc				\
	YProperty.cc				\
	YUIComponent.cc				\
	YUIException.cc				\
	YUIPlugin.cc				\
	YUI_builtins.cc				\
	YUI_core.cc				\
	YUI_util.cc				\
	YWidget.cc				\
	YWidgetFactory.cc			\
	YOptionalWidgetFactory.cc		\
	YWidgetID.cc				\
						\
	YAlignment.cc				\
	YBarGraph.cc				\
	YCheckBox.cc				\
	YCheckBoxFrame.cc			\
	YComboBox.cc				\
	YContainerWidget.cc			\
	YDateField.cc				\
	YDialog.cc				\
	YDownloadProgress.cc			\
	YDumbTab.cc				\
	YEmpty.cc				\
	YEvent.cc				\
	YFrame.cc				\
	YImage.cc				\
	YInputField.cc				\
	YIntField.cc				\
	YLabel.cc				\
	YLayoutBox.cc				\
	YLogView.cc				\
	YMacroPlayer.cc				\
	YMacroRecorder.cc			\
	YMenuButton.cc				\
	YMultiLineEdit.cc			\
	YMultiProgressMeter.cc			\
	YMultiSelectionBox.cc			\
	YPackageSelector.cc			\
	YPartitionSplitter.cc			\
	YProgressBar.cc				\
	YPushButton.cc				\
	YRadioButton.cc				\
	YRadioButtonGroup.cc			\
	YReplacePoint.cc			\
	YRichText.cc				\
	YSelectionBox.cc			\
	YSelectionWidget.cc			\
	YShortcut.cc				\
	YShortcutManager.cc			\
	YSimpleEventHandler.cc			\
	YSimpleInputField.cc			\
	YSingleChildContainerWidget.cc		\
	YSlider.cc				\
	YSpacing.cc				\
	YSquash.cc				\
	YTable.cc				\
	YTimeField.cc				\
	YTree.cc				\
	YWizard.cc


pkgincludedir = $(includedir)/yui

pkginclude_HEADERS =				\
	ImplPtr.h				\
	Y2UINamespace.h				\
	Y2CCUI.h				\
	YApplication.h				\
	YBothDim.h				\
	YColor.h				\
	YCPDialogParser.h			\
	YCPItem.h				\
	YCPItemParser.h				\
	YCPItemWriter.h				\
	YCPMenuItem.h				\
	YCPMenuItemParser.h			\
	YCPMenuItemWriter.h			\
	YCPTableItemParser.h			\
	YCPTableItemWriter.h			\
	YCPTableItem.h				\
	YCPTreeItem.h				\
	YCPTreeItemParser.h			\
	YCPTreeItemWriter.h			\
	YCPPropertyHandler.h			\
	YCPValueWidgetID.h			\
	YCP_UI_Exception.h			\
	YChildrenManager.h			\
	YItem.h					\
	YMenuItem.h				\
	YTreeItem.h				\
	YTableItem.h				\
	YTableHeader.h				\
	YOptionalWidgetFactory.h		\
	YPackageSelectorPlugin.h		\
	YProperty.h				\
	YTypes.h				\
	YUI.h					\
	YUIComponent.h				\
	YUIException.h				\
	YUIPlugin.h				\
	YUISymbols.h				\
	YUI_util.h				\
	YWidget.h				\
	YWidgetFactory.h			\
	YWidgetID.h				\
	YWidgetOpt.h				\
	YWidget_OptimizeChanges.h		\
						\
	YAlignment.h				\
	YBarGraph.h				\
	YCheckBox.h				\
	YCheckBoxFrame.h			\
	YComboBox.h				\
	YContainerWidget.h			\
	YDateField.h				\
	YDialog.h				\
	YDownloadProgress.h			\
	YDumbTab.h				\
	YEmpty.h				\
	YEvent.h				\
	YFrame.h				\
	YImage.h				\
	YInputField.h				\
	YIntField.h				\
	YLabel.h				\
	YLayoutBox.h				\
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
	YSelectionWidget.h			\
	YShortcut.h				\
	YShortcutManager.h			\
	YSimpleEventHandler.h			\
	YSingleChildContainerWidget.h		\
	YSimpleInputField.h			\
	YSlider.h				\
	YSpacing.h				\
	YSquash.h				\
	YTable.h				\
	YTimeField.h				\
	YTransText.h				\
	YTree.h					\
	YWizard.h				\
	TreeItem.h



INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	${Y2UTIL_CFLAGS}

libpy2UI_la_LDFLAGS = -version-info 2:0
libpy2UI_la_LIBADD = -lpthread

# "foo bar: baz" is wrong
# see "info '(automake)Multiple Outputs'"
%BuiltinCalls.h %BuiltinTable.h : Y2%Namespace.h
	${top_srcdir}/base/tools/devtools/generateYCPWrappers $< $*BuiltinCalls.h $*BuiltinTable.h

BUILT_SOURCES = UIBuiltinTable.h UIBuiltinCalls.h

CLEANFILES = libyui.kdoc $(BUILT_SOURCES)


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
