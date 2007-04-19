#
# Makefile.am for core/libyui/src
#

AM_CXXFLAGS = -DY2LOG=\"libycp\"		\
	-DTHEMEDIR=\"${themedir}/current\"	\
	-DPLUGINDIR=\"${plugindir}\"		\
	-fmessage-length=0

lib_LTLIBRARIES = libyui.la

libyui_la_SOURCES =				\
	YUIComponent.cc				\
	YUI_core.cc				\
	YUI_builtins.cc				\
	YUI_widgets.cc				\
	YUI_special_widgets.cc			\
	YUI_bindings.cc				\
	YUI_util.cc				\
	YUIPlugin.cc				\
						\
	YAlignment.cc				\
	YBarGraph.cc				\
	YCheckBox.cc				\
	YCheckBoxFrame.cc			\
	YColoredLabel.cc			\
	YDate.cc				\
	YDumbTab.cc				\
	YComboBox.cc				\
	YContainerWidget.cc			\
	YDialog.cc				\
	YDownloadProgress.cc			\
	YEmpty.cc				\
	YEvent.cc				\
	YFrame.cc				\
	YImage.cc				\
	YIntField.cc				\
	YLabel.cc				\
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
	YShortcut.cc				\
	YShortcutManager.cc			\
	YSimpleEventHandler.cc			\
	YSlider.cc				\
	YSpacing.cc				\
	YSplit.cc				\
	YSquash.cc				\
	YTable.cc				\
	YTextEntry.cc				\
	YTime.cc				\
	YTree.cc				\
	YWidget.cc				\
	YWizard.cc				\
	YSelectionWidget.cc


pkgincludedir = $(includedir)/yui

pkginclude_HEADERS =				\
	YUIComponent.h				\
	YUI.h					\
	YUI_util.h				\
	UI.h					\
	YUISymbols.h				\
	YUIPlugin.h				\
	YPackageSelectorPlugin.h		\
						\
	YAlignment.h				\
	YBarGraph.h				\
	YCheckBox.h				\
	YCheckBoxFrame.h			\
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
	YSelectionWidget.h			\
	TreeItem.h



INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	${Y2UTIL_CFLAGS}

libyui_la_LDFLAGS = -version-info 2:0
libyui_la_LIBADD = -lpthread

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
