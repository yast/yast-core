#
# Makefile.am for core/libyui/src
#

SUBDIRS = include

lib_LTLIBRARIES = libyui.la

libyui_la_SOURCES =				\
	YUIInterpreter_core.cc			\
	YUIInterpreter_builtins.cc		\
	YUIInterpreter_widgets.cc		\
	YUIInterpreter_special_widgets.cc	\
						\
	YAlignment.cc				\
	YBarGraph.cc				\
	YCheckBox.cc				\
	YColoredLabel.cc			\
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
	YTree.cc				\
	YWidget.cc

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/include			\
	-I$(includedir)

libyui_la_LDFLAGS = -version-info 2:0
libyui_la_LIBADD = -lpthread

CLEANFILES = libyui.kdoc
