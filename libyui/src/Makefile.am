#
# Makefile.am for core/libyui/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libycp\"

lib_LTLIBRARIES = libyui.la

libyui_la_SOURCES =				\
	YUIComponent.cc				\
	YUI_core.cc				\
	YUI_builtins.cc				\
	YUI_widgets.cc				\
	YUI_special_widgets.cc			\
	YUI_bindings.cc				\
						\
	YAlignment.cc				\
	YBarGraph.cc				\
	YCheckBox.cc				\
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
	YWizard.cc


INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	-I$(srcdir)/include			\
	${Y2UTIL_CFLAGS}

libyui_la_LDFLAGS = -version-info 2:0
libyui_la_LIBADD = -lpthread

CLEANFILES = libyui.kdoc
