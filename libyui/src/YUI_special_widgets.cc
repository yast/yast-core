/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

File:		YUI_special_widgets.cc

Special (optional) widgets

		
Authors:	Mathias Kettner <kettner@suse.de>
Stefan Hundhammer <sh@suse.de>
		
Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/



#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPMap.h>

#include "YUI.h"
#include "YUISymbols.h"
#include "YWidget.h"
#include "YMacroRecorder.h"
#include "YMacroPlayer.h"

#include "YBarGraph.h"

#include <assert.h>


/**
 * @builtin HasSpecialWidget( `symbol widget ) -> boolean
 *
 * Checks for support of a special widget type. Use this prior to creating a
 * widget of this kind. Do not use this to check for ordinary widgets like
 * PushButton etc. - just the widgets where the widget documentation explicitly
 * states it is an optional widget not supported by all UIs.
 * <p>
 * Returns true if the UI supports the special widget and false if not.
 */

YCPValue YUI::evaluateHasSpecialWidget( const YCPSymbol & widget )
{
    bool hasWidget = false;

    string symbol = widget->symbol();

    if	    ( symbol == YUISpecialWidget_DummySpecialWidget	)	hasWidget = hasDummySpecialWidget();
    else if ( symbol == YUISpecialWidget_BarGraph		)	hasWidget = hasBarGraph();
    else if ( symbol == YUISpecialWidget_ColoredLabel		)	hasWidget = hasColoredLabel();
    else if ( symbol == YUISpecialWidget_DownloadProgress	)	hasWidget = hasDownloadProgress();
    else if ( symbol == YUISpecialWidget_Slider			)	hasWidget = hasSlider();
    else if ( symbol == YUISpecialWidget_PartitionSplitter	)	hasWidget = hasPartitionSplitter();
    else
    {
	y2error( "HasSpecialWidget(): Unknown special widget: %s", symbol.c_str() );
	return YCPNull();
    }

    return YCPBoolean( hasWidget );
}



//
// =============================================================================
// High level ( abstract libyui layer ) widget creation functions.
// Most call a corresponding low level ( specific UI ) widget creation function.
//
// Special widgets that may or may not be supported by a specific UI.
// Remember to overwrite the has...() method as well as the create...() method!
// =============================================================================
//


YWidget * YUI::createDummySpecialWidget( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
					 const YCPList & optList, int argnr )
{
    if ( term->size() - argnr > 0 )
    {
	y2error( "Invalid arguments for the DummySpecialWidget widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions( term,optList );

    if ( hasDummySpecialWidget() )
    {
	return createDummySpecialWidget( parent, opt );
    }
    else
    {
	y2error( "This UI does not support the DummySpecialWidget." );
	return 0;
    }
}


bool YUI::hasDummySpecialWidget()
{
    return true;
}


/**
 * Low level specific UI implementation of DummySpecialWidget,
 * just for demonstration purposes: Creates a heading with a fixed text.
 * Normally, the implementation within the libyui returns 0.
 */

YWidget * YUI::createDummySpecialWidget( YWidget *parent, YWidgetOpt & opt )
{
    opt.isHeading.setValue( true );
    opt.isOutputField.setValue( true );
    return createLabel( parent, opt, YCPString( "DummySpecialWidget" ) );
}


// ----------------------------------------------------------------------

/*
 * @widget	BarGraph
 * @short	Horizontal bar graph ( optional widget )
 * @class	YBarGraph
 * @arg		list values the initial values ( integer numbers )
 * @optarg	list labels the labels for each part; use "%1" to include the
 *		current numeric value. May include newlines.
 * @usage	if ( HasSpecialWidget( `BarGraph ) {...
 *		`BarGraph( [ 450, 100, 700 ],
 *		[ "Windows used\n%1 MB", "Windows free\n%1 MB", "Linux\n%1 MB" ] )
 *
 * @examples	BarGraph1.ycp BarGraph2.ycp BarGraph3.ycp
 *
 * @description
 *
 * A horizontal bar graph for graphical display of proportions of integer
 * values.  Labels can be passed for each portion; they can include a "%1"
 * placeholder where the current value will be inserted ( sformat() -style ) and
 * newlines. If no labels are specified, only the values will be
 * displayed. Specify empty labels to suppress this.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `BarGraph )</tt> before using it.
 *
 */

YWidget * YUI::createBarGraph( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			       const YCPList & optList, int argnr )
{
    int numArgs = term->size() - argnr;

    if ( numArgs < 1 || numArgs > 2
	 || ! term->value(argnr)->isList()
	 || ( numArgs > 1 && ! term->value(argnr+1)->isList() )
	 )
    {
	y2error( "Invalid arguments for the BarGraph widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions( term,optList );
    YBarGraph *barGraph;

    if ( hasBarGraph() )
    {
	barGraph = dynamic_cast <YBarGraph *> ( createBarGraph( parent, opt ) );
	assert( barGraph );

	barGraph->parseValuesList( term->value( argnr )->asList() );

	if ( numArgs > 1 )
	{
	    barGraph->parseLabelsList( term->value( argnr+1 )->asList() );
	}

	barGraph->doUpdate();
    }
    else
    {
	y2error( "This UI does not support the BarGraph widget." );
	return 0;
    }

    return barGraph;
}

// ----------------------------------------------------------------------


/**
 * @widgets	ColoredLabel
 * @short	Simple static text with specified background and foreground color
 * @class	YColoredLabel
 * @arg		string label
 * @arg		color foreground color
 * @arg		color background color
 * @arg		integer margin around the widget in pixels
 * @usage	`ColoredLabel( "Hello, World!", `rgb( 255, 0, 255 ), `rgb( 0, 128, 0 ), 20 )
 *
 * @examples	ColoredLabel1.ycp ColoredLabel2.ycp ColoredLabel3.ycp ColoredLabel4.ycp
 *
 * @description
 *
 * Very much the same as a `Label except you specify foreground and background colors and margins.
 * This widget is only available on graphical UIs with at least 15 bit color depth ( 32767 colors ).
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `ColoredLabel )</tt> before using it.
 */

YWidget * YUI::createColoredLabel( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				   const YCPList & optList, int argnr )
{
    if ( term->size() - argnr != 4
	 || ! term->value( argnr   )->isString()		// label
	 || ! term->value( argnr+1 )->isTerm()		// foreground color
	 || ! term->value( argnr+2 )->isTerm()		// background color
	 || ! term->value( argnr+3 )->isInteger() )	// margin
    {
	y2error( "Invalid arguments for the ColoredLabel widget: %s", term->toString().c_str() );
	return 0;
    }

    rejectAllOptions( term,optList );

    YColor fg;
    YColor bg;
    YCPString label = term->value( argnr )->asString();
    parseRgb( term->value( argnr+1 ), & fg, true );
    parseRgb( term->value( argnr+2 ), & bg, true );
    int margin  = term->value( argnr+3 )->asInteger()->value();

    if ( ! hasColoredLabel() )
    {
	y2error( "This UI does not support the ColoredLabel widget." );
	return 0;
    }

    return createColoredLabel( parent, opt, label, fg, bg, margin );
}


// ----------------------------------------------------------------------

/*
 * @widget	DownloadProgress
 * @short	Self-polling file growth progress indicator ( optional widget )
 * @class	YDownloadProgress
 * @arg		string label label above the indicator
 * @arg		string filename file name with full path of the file to poll
 * @arg		integer expectedSize expected final size of the file in bytes
 * @usage	if ( HasSpecialWidget( `DownloadProgress ) {...
 *		`DownloadProgress( "Base system ( 230k )", "/tmp/aaa_base.rpm", 230*1024 );
 *
 * @examples	DownloadProgress1.ycp
 *
 * @description
 *
 * This widget automatically displays the progress of a lengthy download
 * operation. The widget itself ( i.e. the UI ) polls the specified file and
 * automatically updates the display as required even if the download is taking
 * place in the foreground.
 * <p>
 * Please notice that this will work only if the UI runs on the same machine as
 * the file to download which may not taken for granted ( but which is so for
 * most users ).
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `DownloadProgress )</tt> before using it.
 *
 */

YWidget * YUI::createDownloadProgress( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				       const YCPList & optList, int argnr )
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 3
	 || ! term->value(argnr  )->isString()
	 || ! term->value(argnr+1)->isString()
	 || ! term->value(argnr+2)->isInteger()
	 )
    {
	y2error( "Invalid arguments for the DownloadProgress widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions( term,optList );
    YWidget *downloadProgress;

    if ( hasDownloadProgress() )
    {
	YCPString label	 = term->value( argnr )->asString();
	YCPString filename = term->value( argnr+1 )->asString();
	int expectedSize	 = term->value( argnr+2 )->asInteger()->value();

	downloadProgress = createDownloadProgress( parent, opt, label, filename, expectedSize );
    }
    else
    {
	y2error( "This UI does not support the DownloadProgress widget." );
	return 0;
    }

    return downloadProgress;
}



/*
 * @widget	Slider
 * @short	Numeric limited range input ( optional widget )
 * @class	YSlider
 * @arg		string	label		Explanatory label above the slider
 * @arg		integer minValue	minimum value
 * @arg		integer maxValue	maximum value
 * @arg		integer initialValue	initial value
 * @usage	if ( HasSpecialWidget( `Slider ) {...
 *		`Slider( "Percentage", 1, 100, 50 )
 *
 * @examples	Slider1.ycp Slider2.ycp ColoredLabel3.ycp
 *
 * @description
 *
 * A horizontal slider with ( numeric ) input field that allows input of an
 * integer value in a given range. The user can either drag the slider or
 * simply enter a value in the input field.
 * <p>
 * Remember you can use <tt>`opt( `notify )</tt> in order to get instant response
 * when the user changes the value - if this is desired.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `Slider )</tt> before using it.
 *
 */

YWidget * YUI::createSlider( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			     const YCPList & optList, int argnr )
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 4
	 || ! term->value(argnr)->isString()
	 || ! term->value(argnr+1)->isInteger()
	 || ! term->value(argnr+2)->isInteger()
	 || ! term->value(argnr+3)->isInteger()
	 )
    {
	y2error( "Invalid arguments for the Slider widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions( term,optList );
    YWidget *slider;

    if ( hasSlider() )
    {
	YCPString label	 = term->value( argnr   )->asString();
	int minValue	 = term->value( argnr+1 )->asInteger()->value();
	int maxValue	 = term->value( argnr+2 )->asInteger()->value();
	int initialValue = term->value( argnr+3 )->asInteger()->value();
	slider = createSlider( parent, opt, label, minValue, maxValue, initialValue );
    }
    else
    {
	y2error( "This UI does not support the Slider widget." );
	return 0;
    }

    return slider;
}


/*
 * @widget	PartitionSplitter
 * @short	Hard disk partition splitter tool ( optional widget )
 * @class	YPartitionSplitter
 *
 * @arg integer	usedSize		size of the used part of the partition
 * @arg integer	totalFreeSize 		total size of the free part of the partition
 *					( before the split )
 * @arg integer newPartSize		suggested size of the new partition
 * @arg integer minNewPartSize		minimum size of the new partition
 * @arg integer minFreeSize		minimum free size of the old partition
 * @arg string	usedLabel 		BarGraph label for the used part of the old partition
 * @arg string	freeLabel 		BarGraph label for the free part of the old partition
 * @arg string	newPartLabel  		BarGraph label for the new partition
 * @arg string	freeFieldLabel		label for the remaining free space field
 * @arg string	newPartFieldLabel	label for the new size field
 * @usage	if ( HasSpecialWidget( `PartitionSplitter ) {...
 *		`PartitionSplitter( 600, 1200, 800, 300, 50,
 *                                 "Windows used\n%1 MB", "Windows used\n%1 MB", "Linux\n%1 MB", "Linux ( MB )" )
 *
 * @examples	PartitionSplitter1.ycp PartitionSplitter2.ycp
 *
 * @description
 *
 * A very specialized widget to allow a user to comfortably split an existing
 * hard disk partition in two parts. Shows a bar graph that displays the used
 * space of the partition, the remaining free space ( before the split ) of the
 * partition and the space of the new partition ( as suggested ).
 * Below the bar graph is a slider with an input fields to the left and right
 * where the user can either input the desired remaining free space or the
 * desired size of the new partition or drag the slider to do this.
 * <p>
 * The total size is <tt>usedSize+freeSize</tt>.
 * <p>
 * The user can resize the new partition between <tt>minNewPartSize</tt> and
 * <tt>totalFreeSize-minFreeSize</tt>.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `PartitionSplitter )</tt> before using it.
 * */

YWidget * YUI::createPartitionSplitter( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
					const YCPList & optList, int argnr )
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 10
	 || ! term->value(argnr  )->isInteger()	// usedSize
	 || ! term->value(argnr+1)->isInteger()	// freeSize
	 || ! term->value(argnr+2)->isInteger()	// newPartSize
	 || ! term->value(argnr+3)->isInteger()	// minNewPartSize
	 || ! term->value(argnr+4)->isInteger()	// minFreeSize
	 || ! term->value(argnr+5)->isString()	// usedLabel
	 || ! term->value(argnr+6)->isString()	// freeLabel
	 || ! term->value(argnr+7)->isString()	// newPartLabel
	 || ! term->value(argnr+8)->isString()	// freeFieldLabel
	 || ! term->value(argnr+9)->isString()	// newPartFieldLabel
	 )
    {
	y2error( "Invalid arguments for the PartitionSplitter widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    for ( int o=0; o < optList->size(); o++ )
    {
	if ( optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_countShowDelta ) opt.countShowDelta.setValue( true );
	else logUnknownOption( term, optList->value(o) );
    }

    YWidget *splitter;

    if ( hasPartitionSplitter() )
    {
	int 		usedSize		= term->value( argnr   )->asInteger()->value();
	int 		totalFreeSize		= term->value( argnr+1 )->asInteger()->value();
	int 		newPartSize		= term->value( argnr+2 )->asInteger()->value();
	int 		minNewPartSize		= term->value( argnr+3 )->asInteger()->value();
	int 		minFreeSize		= term->value( argnr+4 )->asInteger()->value();
	YCPString 	usedLabel		= term->value( argnr+5 )->asString();
	YCPString 	freeLabel		= term->value( argnr+6 )->asString();
	YCPString 	newPartLabel		= term->value( argnr+7 )->asString();
	YCPString 	freeFieldLabel		= term->value( argnr+8 )->asString();
	YCPString 	newPartFieldLabel	= term->value( argnr+9 )->asString();

	splitter = createPartitionSplitter( parent, opt,
					    usedSize, totalFreeSize,
					    newPartSize, minNewPartSize, minFreeSize,
					    usedLabel, freeLabel, newPartLabel,
					    freeFieldLabel, newPartFieldLabel );

    }
    else
    {
	y2error( "This UI does not support the PartitionSplitter widget." );
	return 0;
    }

    return splitter;
}


/**
 * Special widget availability check methods.
 *
 * Overwrite if the specific UI provides the corresponding widget.
 */

bool YUI::hasDownloadProgress()
{
    return false;
}

bool YUI::hasBarGraph()
{
    return false;
}

bool YUI::hasColoredLabel()
{
    return false;
}

bool YUI::hasSlider()
{
    return false;
}

bool YUI::hasPartitionSplitter()
{
    return false;
}


/**
 * Default low level specific UI implementations for optional widgets.
 *
 * UIs that overwrite any of those should overwrite the corresponding
 * has...() method as well!
 */

YWidget * YUI::createDownloadProgress( YWidget *parent, YWidgetOpt & opt,
				       const YCPString & label,
				       const YCPString & filename,
				       int expectedSize )
{
    y2error( "Default createDownloadProgress() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget * YUI::createBarGraph( YWidget *parent, YWidgetOpt & opt )
{
    y2error( "Default createBarGraph() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget * YUI::createColoredLabel( YWidget *parent, YWidgetOpt & opt,
				   YCPString label,
				   YColor fg, YColor bg, int margin )
{
    y2error( "Default createColoredLabel() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget * YUI::createSlider( YWidget *parent, YWidgetOpt & opt,
			     const YCPString & label,
			     int minValue, int maxValue, int initialValue )
{
    y2error( "Default createSlider() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}


YWidget * YUI::createPartitionSplitter( YWidget *		parent,
					YWidgetOpt &		opt,
					int 			usedSize,
					int 			totalFreeSize,
					int 			newPartSize,
					int 			minNewPartSize,
					int 			minFreeSize,
					const YCPString &	usedLabel,
					const YCPString &	freeLabel,
					const YCPString &	newPartLabel,
					const YCPString &	freeFieldLabel,
					const YCPString &	newPartFieldLabel )
{
    y2error( "Default createPartitionSplitter() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}



// EOF
