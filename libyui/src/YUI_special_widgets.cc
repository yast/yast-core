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

Summary: Special (optional) widgets


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
#include "YDumbTab.h"

#include <assert.h>


/**
 * @builtin HasSpecialWidget
 * @short Checks for support of a special widget type.
 * @description
 * Checks for support of a special widget type. Use this prior to creating a
 * widget of this kind. Do not use this to check for ordinary widgets like
 * PushButton etc. - just the widgets where the widget documentation explicitly
 * states it is an optional widget not supported by all UIs.
 * 
 * Returns true if the UI supports the special widget and false if not.
 */

YCPValue YUI::evaluateHasSpecialWidget( const YCPSymbol & widget )
{
    bool hasWidget = false;

    string symbol = widget->symbol();

    if	    ( symbol == YUISpecialWidget_DummySpecialWidget	)	hasWidget = hasDummySpecialWidget();
    else if ( symbol == YUISpecialWidget_BarGraph		)	hasWidget = hasBarGraph();
    else if ( symbol == YUISpecialWidget_ColoredLabel		)	hasWidget = hasColoredLabel();
    else if ( symbol == YUISpecialWidget_DumbTab		)	hasWidget = hasDumbTab();
    else if ( symbol == YUISpecialWidget_DownloadProgress	)	hasWidget = hasDownloadProgress();
    else if ( symbol == YUISpecialWidget_HMultiProgressMeter	)	hasWidget = hasMultiProgressMeter();
    else if ( symbol == YUISpecialWidget_VMultiProgressMeter	)	hasWidget = hasMultiProgressMeter();
    else if ( symbol == YUISpecialWidget_Slider			)	hasWidget = hasSlider();
    else if ( symbol == YUISpecialWidget_PartitionSplitter	)	hasWidget = hasPartitionSplitter();
    else if ( symbol == YUISpecialWidget_Wizard			)	hasWidget = hasWizard();
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
 * @short	Horizontal bar graph (optional widget)
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
 * @note        This is a "special" widget, i.e. not all UIs necessarily support it.  Check
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
 * @note  This is a "special" widget, i.e. not all UIs necessarily support it.  Check
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
 * @short	Self-polling file growth progress indicator (optional widget)
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
 * 
 * Please notice that this will work only if the UI runs on the same machine as
 * the file to download which may not taken for granted ( but which is so for
 * most users ).
 * 
 * @note This is a "special" widget, i.e. not all UIs necessarily support it.  Check
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



// ----------------------------------------------------------------------

/*
 * @widget	DumbTab
 * @short	Simplistic tab widget that behaves like push buttons
 * @class	YDumbTab
 * @arg		list tabs page headers
 * @arg		term contents page contents - usually a ReplacePoint
 * @usage	if ( HasSpecialWidget( `DumbTab) {...
 *		`DumbTab( [ `item(`id(`page1), "Page &1" ), `item(`id(`page2), "Page &2" ) ], contents; }
 *
 * @examples	DumbTab1.ycp DumbTab2.ycp
 *
 * @description
 *
 * This is a very simplistic approach to tabbed dialogs: The application
 * specifies a number of tab headers and the page contents and takes care of
 * most other things all by itself, in particular page switching. Each tab
 * header behaves very much like a PushButton - as the user activates a tab
 * header, the DumbTab widget simply returns the ID of that tab (or its text if
 * it has no ID). The application should then take care of changing the page
 * contents accordingly - call UI::ReplaceWidget() on the ReplacePoint
 * specified as tab contents or similar actions (it might even just replace
 * data in a Table or RichText widget if this is the tab contents). Hence the
 * name <i>Dumb</i>Tab.
 * 
 * The items in the item list can either be simple strings or `item() terms
 * with an optional ID for each individual item (which will be returned upon
 * UI::UserInput() and related when the user selects this tab), a (mandatory)
 * user-visible label and an (optional) flag that indicates that this tab is
 * initially selected. If you specify only a string, UI::UserInput() will
 * return this string.
 *
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `DumbTab )</tt> before
 * using it.
 * 
 * @note Please notice that using this kind of widget more often than not is the
 * result of <b>poor dialog or workflow design</b>.
 * 
 * Using tabs only hides complexity, but the complexity remains there. They do
 * little to make problems simpler. This however should be the approach of
 * choice for good user interfaces.
 * 
 * It is very common for tabs to be overlooked by users if there are just two
 * tabs to select from, so in this case better use an "Expert..." or
 * "Details..." button - this gives much more clue to the user that there is
 * more information  available while at the same time clearly indicating that
 * those other options are much less commonly used.
 * 
 * If there are very many different views on data or if there are lots and lots
 * of settings, you might consider using a tree for much better navigation. The
 * Qt UI's wizard even has a built-in tree that can be used instead of the help
 * panel.
 * 
 * If you use a tree for navigation, unter all circumstances avoid using tabs
 * at the same time - there is no way for the user to tell which tree nodes
 * have tabs and which have not, making navigation even more difficult.
 * KDE's control center or Mozilla's settings are very good examples
 * how <b>not</b> to do that - you become bogged down for sure in all those
 * tree nodes and tabs hidden within so many of them.
 * 
 */

YWidget * YUI::createDumbTab( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			      const YCPList & optList, int argnr, YRadioButtonGroup * rbg )
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 2
	 || ! term->value( argnr   )->isList()
	 || ! term->value( argnr+1 )->isTerm()
	 )
    {
	y2error( "Invalid arguments for the DumbTab widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    YCPList itemList = term->value( argnr   )->asList();
    YCPTerm contents = term->value( argnr+1 )->asTerm();

    rejectAllOptions( term,optList );

    if ( ! hasDumbTab() )
    {
	y2error( "This UI does not support the DumbTab widget." );
	return 0;
    }


    //
    // Create the DumbTab itself
    //

    YDumbTab * dumbTab = dynamic_cast<YDumbTab *> ( createDumbTab( parent, opt ) );

    if ( dumbTab )
    {
	dumbTab->setParent( parent );

	//
	// Parse item (tab header) list
	//

	for ( int i=0; i < itemList->size(); i++ )
	{
	    bool error = false;
	    YCPValue item = itemList->value(i);

	    if ( item->isString() )
	    {
		// Simple case: The list element is just a string. Use it as the tab label.

		dumbTab->addTab( YCPNull(), item->asString(), false );
	    }
	    else if ( item->isTerm () && item->asTerm()->name() == YUISymbol_item )
	    {
		// Complex case: `item( `id( item_id ), label, selected )
		// "item_id" and the "selected" flag are optional, "label" is mandatory.

		YCPTerm		itemTerm( item->asTerm() );
		YCPValue 	id = YCPNull();
		YCPString	label( "" );
		bool	  	selected = false;
		int 		n 	 = 0;

		// get (optional) tab ID

		if ( checkId( itemTerm->value(n) ) )	// check for `id(...)
		    id = getId( itemTerm->value( n++ ) );

		// get (mandatory) tab label

		if ( itemTerm->size() > n && itemTerm->value(n)->isString() )
		    label = itemTerm->value( n++ )->asString();
		else
		    error = true;

		// get (optional) tab selected state

		if ( itemTerm->size() > n )
		{
		    if ( itemTerm->value(n)->isBoolean() )
			selected = itemTerm->value( n++ )->asBoolean()->value();
		    else
			error = true;
		}


		// check for leftover arguments

		if ( itemTerm->size() != n )
		    error = true;

		if ( ! error )
		    dumbTab->addTab( id, label, selected );
	    }
	    else
		error = true;


	    if ( error )
	    {
		y2error( "Invalid DumbTab item: %s", item->toString().c_str() );

		delete dumbTab;
		return 0;
	    }
	}

	//
	// Create child (page contents)
	//

	YWidget * child = createWidgetTree( dumbTab, rbg, contents );

	if ( child )
	{
	    dumbTab->addChild( child );
	}
	else
	{
	    y2error( "Couldn't create DumbTab children" );

	    delete dumbTab;
	    return 0;
	}

    }

    return dumbTab;
}



/*
 * @widget	VMultiProgressMeter HMultiProgressMeter
 * @short	Progress bar with multiple segments (optional widget)
 * @class	YMultiProgressMeter
 * @arg		List<integer> maxValues		maximum values
 * @usage	if ( HasSpecialWidget( `MultiProgressMeter ) {...
 *		`MultiProgressMeter( "Percentage", 1, 100, 50 )
 *
 * @examples	MultiProgressMeter1.ycp 
 *
 * @description
 *
 * A vertical (VMultiProgressMeter) or horizontal (HMultiProgressMeter)
 * progress display with multiple segments. The numbers passed on widget
 * creation are the maximum numbers of each individual segment. Segments sizes
 * will be displayed proportionally to these numbers.
 * 
 * This widget is intended for applications like showing the progress of
 * installing from multiple CDs while giving the user a hint how much will be
 * installed from each individual CD.
 * 
 * Set actual values later with
 * <code>
 * UI::ChangeWidget(`id(...), `Values, [ 1, 2, ...] );
 * </code>
 * 
 * The widget may choose to reserve a minimum amount of space for each segment
 * even if that means that some segments will be shown slightly out of
 * proportion.
 * 
 * @note  This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `MultiProgressMeter )</tt> before using it.
 */

YWidget * YUI::createMultiProgressMeter( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
					 const YCPList & optList, int argnr, bool horizontal )
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 1 || ! term->value(argnr)->isList() )
    {
	y2error( "Invalid arguments for the MultiProgressMeter widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    YCPList maxValues = term->value( argnr )->asList();

    if ( maxValues->size() < 1 )
    {
	y2error( "Empty MultiProgressMeter maxValues list" );
	return 0;
    }

    for ( int i=0; i < maxValues->size(); i++ )
    {
	if ( ! maxValues->value( i )->isInteger() )
	{
	    y2error( "MultiProgressMeter maxValues list should contain integer values, not %s",
		     maxValues->value( i )->toString().c_str() );
	    return 0;
	}
    }


    rejectAllOptions( term,optList );
    YWidget *multiProgressMeter;

    if ( hasMultiProgressMeter() )
    {
	multiProgressMeter = createMultiProgressMeter( parent, opt, horizontal, maxValues );
    }
    else
    {
	y2error( "This UI does not support the MultiProgressMeter widget." );
	return 0;
    }

    return multiProgressMeter;
}



/*
 * @widget	Slider
 * @short	Numeric limited range input (optional widget)
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
 * A horizontal slider with ( numeric ) input field that allows input of an
 * integer value in a given range. The user can either drag the slider or
 * simply enter a value in the input field.
 * 
 * Remember you can use <tt>`opt( `notify )</tt> in order to get instant response
 * when the user changes the value - if this is desired.
 * 
 * @note  This is a "special" widget, i.e. not all UIs necessarily support it.  Check
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
 * @short	Hard disk partition splitter tool (optional widget)
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
 * 
 * The total size is <tt>usedSize+freeSize</tt>.
 * 
 * The user can resize the new partition between <tt>minNewPartSize</tt> and
 * <tt>totalFreeSize-minFreeSize</tt>.
 * 
 * @note This is a "special" widget, i.e. not all UIs necessarily support it.  Check
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



/*
 * @widget	Wizard
 * @short	Wizard frame - not for general use, use the Wizard:: module instead!
 * @class	YWizard
 *
 * @option	stepsEnabled	Enable showing wizard steps (use UI::WizardCommand() to set them).
 * @option	treeEnabled	Enable showing a selection tree in the left panel. Disables stepsEnabled.
 *
 * @arg		any	backButtonId		ID to return when the user presses the "Back" button
 * @arg		string	backButtonLabel		Label of the "Back" button
 *
 * @arg		any	abortButtonId		ID to return when the user presses the "Abort" button
 * @arg		string	abortButtonLabel	Label of the "Abort" button
 *
 * @arg		any	nextButtonId		ID to return when the user presses the "Next" button
 * @arg		string	nextButtonLabel		Label of the "Next" button
 *
 * @usage	`Wizard(`id(`back), "&Back", `id(`abort), "Ab&ort", `id(`next), "&Next" )
 * @usage	`Wizard(`back, "&Back", `abort, "Ab&ort", `next, "&Next" )
 *
 * @description
 *
 * This is the UI-specific technical implementation of a wizard dialog's main widget.
 * This is not intended for general use - use the Wizard:: module instead which will use this
 * widget properly.
 * 
 * A wizard widget always has ID `wizard.<p>
 * The ID of the single replace point within the wizard is always `contents.
 * 
 * @note This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget( `PartitionSplitter )</tt> before using it.
 */

YWidget * YUI::createWizard( YWidget * parent, YWidgetOpt & opt, const YCPTerm & term,
			     const YCPList & optList, int argnr )
{
    if ( term->size() - argnr != 6
	 || ! isSymbolOrId( term->value( argnr   ) ) || ! term->value( argnr+1 )->isString()
	 || ! isSymbolOrId( term->value( argnr+2 ) ) || ! term->value( argnr+3 )->isString()
	 || ! isSymbolOrId( term->value( argnr+4 ) ) || ! term->value( argnr+5 )->isString() )
    {
	y2error( "Invalid arguments for the Wizard widget: %s",
		 term->toString().c_str() );
	return 0;
    }


    // Parse options

    for ( int o=0; o < optList->size(); o++ )
    {
	if      ( optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_stepsEnabled ) opt.stepsEnabled.setValue( true );
	else if ( optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_treeEnabled  ) opt.treeEnabled.setValue( true );
	else logUnknownOption( term, optList->value(o) );
    }

    YCPValue	backButtonId		= getId( term->value( argnr ) );
    YCPString 	backButtonLabel 	= term->value( argnr+1 )->asString();

    YCPValue	abortButtonId		= getId( term->value( argnr+2 ) );
    YCPString 	abortButtonLabel	= term->value( argnr+3 )->asString();

    YCPValue	nextButtonId		= getId( term->value( argnr+4 ) );
    YCPString 	nextButtonLabel		= term->value( argnr+5 )->asString();

    return createWizard( parent, opt,
			 backButtonId,  backButtonLabel,
			 abortButtonId,	abortButtonLabel,
			 nextButtonId,	nextButtonLabel  );
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


YWidget * YUI::createDumbTab( YWidget *parent, YWidgetOpt & opt )
{
    y2error( "Default createDumbTab() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}


YWidget * YUI::createMultiProgressMeter( YWidget *parent, YWidgetOpt & opt,
					 bool horizontal, const YCPList & maxValues )
{
    y2error( "Default createMultiProgressMeter() method called - "
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


YWidget * YUI::createWizard( YWidget *parent, YWidgetOpt & opt,
			     const YCPValue & backButtonId,	const YCPString & backButtonLabel,
			     const YCPValue & abortButtonId,	const YCPString & abortButtonLabel,
			     const YCPValue & nextButtonId,	const YCPString & nextButtonLabel  )
{
    y2error( "Default createWizard() method called - "
	     "forgot to call HasSpecialWidget()?" );

    return 0;
}


// EOF
