
htmldir = $(docdir)/autodocs

html_DATA =					\
	YAlignment.html				\
	YAnyOpt.html				\
	YBarGraph.html				\
	YBoolOpt.html				\
	YCheckBox.html				\
	YComboBox.html				\
	YContainerWidget.html			\
	YDialog.html				\
	YDownloadProgress.html			\
	YEmpty.html				\
	YFrame.html				\
	YImage.html				\
	YIntField.html				\
	YLabel.html				\
	YLogView.html				\
	YLongOpt.html				\
	YMacroPlayer.html			\
	YMacroRecorder.html			\
	YMenu.html				\
	YMenuButton.html			\
	YMenuItem.html				\
	YMultiLineEdit.html			\
	YMultiSelectionBox.html			\
	YPartitionSplitter.html			\
	YProgressBar.html			\
	YPushButton.html			\
	YRadioButton.html			\
	YRadioButtonGroup.html			\
	YReplacePoint.html			\
	YRichText.html				\
	YSelectionBox.html			\
	YSlider.html				\
	YSpacing.html				\
	YSplit.html				\
	YSquash.html				\
	YTable.html				\
	YTableRow.html				\
	YTextEntry.html				\
	YTree.html				\
	YTreeItem.html				\
	YUIInterpreter.html			\
	YWidget.html				\
	heir.html				\
	index.html

EXTRA_DIST = $(html_DATA)

CLEANFILES = $(html_DATA)

$(filter-out index.html, $(html_DATA)): index.html
index.html: $(srcdir)/../../src/include/*.h
	cd $(srcdir)/../../src/include && @ybindir@/ydoc libyui *.h
	mv -f $(srcdir)/../../src/include/*.html .

