#
# Makefile.am for core/libyui/doc/widgets
#

widgetdir = $(docdir)/libyui/widgets

widget_DATA =					\
	Box-widget.html				\
	YAlignment-widget.html			\
	YBarGraph-widget.html			\
	YCheckBox-widget.html			\
	YColoredLabel-widget.html		\
	YComboBox-widget.html			\
	YDownloadProgress-widget.html		\
	YEmpty-widget.html			\
	YFrame-widget.html			\
	YImage-widget.html			\
	YIntField-widget.html			\
	YLabel-widget.html			\
	YLogView-widget.html			\
	YMenuButton-widget.html			\
	YMultiLineEdit-widget.html		\
	YMultiSelectionBox-widget.html		\
	YPackageSelector-widget.html		\
	YPartitionSplitter-widget.html		\
	YProgressBar-widget.html		\
	YPushButton-widget.html			\
	YRadioButton-widget.html		\
	YRadioButtonGroup-widget.html		\
	YReplacePoint-widget.html		\
	YRichText-widget.html			\
	YSelectionBox-widget.html		\
	YSlider-widget.html			\
	YSpacing-widget.html			\
	YSquash-widget.html			\
	YTable-widget.html			\
	YTextEntry-widget.html			\
	YTree-widget.html			\
	YWeight-widget.html			\
	YWidget-widget.html			\
	index.html

EXTRA_DIST = $(widget_DATA) make_widget_doc

# MAINTAINERCLEANFILES = $(widget_DATA)

CLEANFILES = $(widget_DATA)

$(filter-out index.html, $(widget_DATA)): index.html
index.html: $(srcdir)/../../src/*.cc
	$(srcdir)/make_widget_doc -s $(srcdir)/../../src

