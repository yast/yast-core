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

  File:		YUIInterpreter_widgets.cc

		Standard (mandatory) widgets


  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_REPLACE_WIDGET


#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUIInterpreter.h"
#include "YUISymbols.h"
#include "YWidget.h"
#include "YDialog.h"

#include "YAlignment.h"
#include "YComboBox.h"
#include "YMenuButton.h"
#include "YRadioButton.h"
#include "YRadioButtonGroup.h"
#include "YSelectionBox.h"
#include "YMultiSelectionBox.h"
#include "YPushButton.h"
#include "YTable.h"
#include "YTree.h"



/**
 * The true core of the UI interpreter: Recursively create a widget tree from
 * the contents of "term". Most create???() functions that create container
 * widgets will recurse to this function.
 *
 * Note: There is also an overloaded version without YWidgetOpt - see below.
 */

YWidget *YUIInterpreter::createWidgetTree( YWidget *		p,
					   YWidgetOpt &		opt,
					   YRadioButtonGroup *	rbg,
					   const YCPTerm &	term )
{
    // Extract optional widget ID, if present
    int n;
    YCPValue id = getWidgetId(term, &n);


    // Extract optional widget options `opt(`xyz)

    YCPList rawopt = getWidgetOptions(term, &n);

    // Handle generic options

    /**
     * @widget	AAA_All-Widgets
     * @usage	---
     * @examples	none
     * @short	Generic options for all widgets
     * @class	YWidget
     *
     * @option	notify		Make UserInput() return on any action in this widget.
     *				Normally UserInput() returns only when a button is clicked;
     *				with this option on you can make it return for other events, too,
     *				e.g. when the user selects an item in a SelectionBox
     *				(if `opt(`notify) is set for that SelectionBox).
     *				Only widgets with this option set are affected.
     *
     * @option	disabled	Set this widget insensitive, i.e. disable any user interaction.
     *				The widget will show this state by being greyed out
     *				(depending on the specific UI).
     *
     * @option	hstretch	Make this widget stretchable in the horizontal dimension.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option	vstretch	Make this widget stretchable in the vertical   dimension.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option	hvstretch	Make this widget stretchable in both dimensions.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option  autoShortcut	Automatically choose a keyboard shortcut for this widget and don't complain
     *				in the log file about the missing shortcut.
     *				<br>Don't use this regularly for all widgets - manually chosen keyboard shortcuts
     *				are almost always better than those automatically assigned. Refer to the style guide
     *				for details.
     *				<br>This option is intended used for automatically generated data, e.g., RadioButtons
     *				for software selections that come from file or from some other data base.
     *
     * @option	key_F1  	(NCurses only) activate this widget with the F1 key
     * @option	key_F2  	(NCurses only) activate this widget with the F2 key
     * @option  key_Fxx 	(NCurses only) activate this widget with the Fxx key
     * @option	key_F24 	(NCurses only) activate this widget with the F24 key
     * @option	key_none	(NCurses only) no function key for this widget
     *
     * @description
     *
     * This is not a widget for general usage, this is just a placeholder for
     * descriptions of options that all widgets have in common.
     * <p>
     * Use them for any widget whenever it makes sense.
     *
     * @example AutoShortcut1.ycp AutoShortcut2.ycp
     */

    YCPList ol;

    for (int o=0; o<rawopt->size(); o++)
    {
	if (rawopt->value(o)->isSymbol())
	{
	    string sym = rawopt->value(o)->asSymbol()->symbol();
	    if	    ( sym == YUIOpt_notify	) opt.notifyMode.setValue(true);
	    else if ( sym == YUIOpt_disabled	) opt.isDisabled.setValue(true);
	    else if ( sym == YUIOpt_hstretch	) opt.isHStretchable.setValue(true);
	    else if ( sym == YUIOpt_vstretch	) opt.isVStretchable.setValue(true);
	    else if ( sym == YUIOpt_hvstretch	) { opt.isHStretchable.setValue(true); opt.isVStretchable.setValue(true); }
	    else if ( sym == YUIOpt_autoShortcut) opt.autoShortcut.setValue(true);
	    else if ( sym == YUIOpt_easterEgg	) opt.easterEgg.setValue(true);
	    else if ( sym == YUIOpt_testMode	) opt.testMode.setValue(true);
	    else if ( sym == YUIOpt_key_F1	) opt.key_Fxx.setValue( 1 );
	    else if ( sym == YUIOpt_key_F2	) opt.key_Fxx.setValue( 2 );
	    else if ( sym == YUIOpt_key_F3	) opt.key_Fxx.setValue( 3 );
	    else if ( sym == YUIOpt_key_F4	) opt.key_Fxx.setValue( 4 );
	    else if ( sym == YUIOpt_key_F5	) opt.key_Fxx.setValue( 5 );
	    else if ( sym == YUIOpt_key_F6	) opt.key_Fxx.setValue( 6 );
	    else if ( sym == YUIOpt_key_F7	) opt.key_Fxx.setValue( 7 );
	    else if ( sym == YUIOpt_key_F8	) opt.key_Fxx.setValue( 8 );
	    else if ( sym == YUIOpt_key_F9	) opt.key_Fxx.setValue( 9 );
	    else if ( sym == YUIOpt_key_F10	) opt.key_Fxx.setValue( 10 );
	    else if ( sym == YUIOpt_key_F11	) opt.key_Fxx.setValue( 11 );
	    else if ( sym == YUIOpt_key_F12	) opt.key_Fxx.setValue( 12 );
	    else if ( sym == YUIOpt_key_F13	) opt.key_Fxx.setValue( 13 );
	    else if ( sym == YUIOpt_key_F14	) opt.key_Fxx.setValue( 14 );
	    else if ( sym == YUIOpt_key_F15	) opt.key_Fxx.setValue( 15 );
	    else if ( sym == YUIOpt_key_F16	) opt.key_Fxx.setValue( 16 );
	    else if ( sym == YUIOpt_key_F17	) opt.key_Fxx.setValue( 17 );
	    else if ( sym == YUIOpt_key_F18	) opt.key_Fxx.setValue( 18 );
	    else if ( sym == YUIOpt_key_F19	) opt.key_Fxx.setValue( 19 );
	    else if ( sym == YUIOpt_key_F20	) opt.key_Fxx.setValue( 20 );
	    else if ( sym == YUIOpt_key_F21	) opt.key_Fxx.setValue( 21 );
	    else if ( sym == YUIOpt_key_F22	) opt.key_Fxx.setValue( 22 );
	    else if ( sym == YUIOpt_key_F23	) opt.key_Fxx.setValue( 23 );
	    else if ( sym == YUIOpt_key_F24	) opt.key_Fxx.setValue( 24 );
	    else if ( sym == YUIOpt_key_none	) opt.key_Fxx.setValue( -1 );
	    else ol->add(rawopt->value(o));
	}
	else if (!rawopt->value(o)->isTerm())
	{
	    y2warning("Invalid widget option %s. Options must be symbols or terms",
		      rawopt->value(o)->toString().c_str());
	}
	else ol->add(rawopt->value(o));
    }


    //
    // Extract the widget class
    //

    YWidget *w	= 0;
    string   s	= term->symbol()->symbol();

    // Container widgets

    if      (s == YUIWidget_Bottom	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignEnd	    );
    else if (s == YUIWidget_Frame	)	w = createFrame			(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_HBox	)	w = createLBox			(p, opt, term, ol, n, rbg, YD_HORIZ);
    else if (s == YUIWidget_HCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignCenter,    YAlignUnchanged );
    else if (s == YUIWidget_HSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, true,  false);
    else if (s == YUIWidget_HVCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignCenter,    YAlignCenter    );
    else if (s == YUIWidget_HVSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, true,  true);
    else if (s == YUIWidget_HWeight	)	w = createWeight		(p, opt, term, ol, n, rbg, YD_HORIZ);
    else if (s == YUIWidget_Left	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignBegin,	    YAlignUnchanged );
    else if (s == YUIWidget_RadioButtonGroup)	w = createRadioButtonGroup	(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_Right	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignEnd,	    YAlignUnchanged );
    else if (s == YUIWidget_Top		)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignBegin	    );
    else if (s == YUIWidget_VBox	)	w = createLBox			(p, opt, term, ol, n, rbg, YD_VERT);
    else if (s == YUIWidget_VCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignCenter    );
    else if (s == YUIWidget_VSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, false, true);
    else if (s == YUIWidget_VWeight	)	w = createWeight		(p, opt, term, ol, n, rbg, YD_VERT);
    else if (s == YUIWidget_ReplacePoint)	w = createReplacePoint		(p, opt, term, ol, n, rbg);

    // Leaf widgets

    else if (s == YUIWidget_CheckBox	)	w = createCheckBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_ComboBox	)	w = createComboBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_Empty	)	w = createEmpty			(p, opt, term, ol, n, false, false);
    else if (s == YUIWidget_HSpacing	)	w = createSpacing		(p, opt, term, ol, n, true,  false);
    else if (s == YUIWidget_HStretch	)	w = createEmpty			(p, opt, term, ol, n, true,  false);
    else if (s == YUIWidget_HVStretch	)	w = createEmpty			(p, opt, term, ol, n, true,  true);
    else if (s == YUIWidget_Heading	)	w = createLabel			(p, opt, term, ol, n, true);
    else if (s == YUIWidget_Image	)	w = createImage			(p, opt, term, ol, n);
    else if (s == YUIWidget_IntField	)	w = createIntField		(p, opt, term, ol, n);
    else if (s == YUIWidget_Label	)	w = createLabel			(p, opt, term, ol, n, false);
    else if (s == YUIWidget_LogView	)	w = createLogView		(p, opt, term, ol, n);
    else if (s == YUIWidget_MultiLineEdit)	w = createMultiLineEdit		(p, opt, term, ol, n);
    else if (s == YUIWidget_MultiSelectionBox)	w = createMultiSelectionBox	(p, opt, term, ol, n);
    else if (s == YUIWidget_Password	)	w = createTextEntry		(p, opt, term, ol, n, true);
    else if (s == YUIWidget_ProgressBar	)	w = createProgressBar		(p, opt, term, ol, n);
    else if (s == YUIWidget_PackageSelector)	w = createPackageSelector	(p, opt, term, ol, n);
    else if (s == YUIWidget_PushButton	)	w = createPushButton		(p, opt, term, ol, n, false);
    else if (s == YUIWidget_IconButton	)	w = createPushButton		(p, opt, term, ol, n, true );
    else if (s == YUIWidget_MenuButton	)	w = createMenuButton		(p, opt, term, ol, n);
    else if (s == YUIWidget_RadioButton	)	w = createRadioButton		(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_RichText	)	w = createRichText		(p, opt, term, ol, n);
    else if (s == YUIWidget_SelectionBox)	w = createSelectionBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_Table	)	w = createTable			(p, opt, term, ol, n);
    else if (s == YUIWidget_TextEntry	)	w = createTextEntry		(p, opt, term, ol, n, false);
    else if (s == YUIWidget_Tree	)	w = createTree			(p, opt, term, ol, n);
    else if (s == YUIWidget_VSpacing	)	w = createSpacing		(p, opt, term, ol, n, false, true);
    else if (s == YUIWidget_VStretch	)	w = createEmpty			(p, opt, term, ol, n, false, true);
    else if (s == YUIWidget_PkgSpecial	)	w = createPkgSpecial		(p, opt, term, ol, n);

    // Special widgets - may or may not be supported by the specific UI.
    // The YCP application should ask for presence of such a widget with Has???Widget() prior to creating one.

    else if (s == YUISpecialWidget_DummySpecialWidget)	w = createDummySpecialWidget	(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_DownloadProgress  )	w = createDownloadProgress	(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_BarGraph	)	w = createBarGraph		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_ColoredLabel	)	w = createColoredLabel		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_Slider	)	w = createSlider		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_PartitionSplitter)	w = createPartitionSplitter	(p, opt, term, ol, n);
    else
    {
	y2error("Unknown widget type %s", s.c_str());
	return 0;
    }


    // Post-process the newly created widget

    if (w)
    {
	if ( ! id.isNull()  &&	// ID specified for this widget
	     ! id->isVoid() &&
	     ! w->hasId() )	// widget doesn't have an ID yet
	{
	    w->setId(id);

	    /*
	     * Note: Don't set the ID if it is already set!
	     * This is important for createXy() functions that don't really create
	     * anything immediately but recursively call createWidgetTree()
	     * internally - e.g. createWeight(). In this case, the widget might
	     * already have an ID, so leave it alone.
	     */
	}

	if ( opt.isDisabled.value() )
	{
	    w->setEnabling(false);
	}

	w->setParent(p);
    }

    return w;
}


/**
 * Overloaded version - just for convenience.
 * Most callers don't need to set up the widget options before calling, so this
 * version will pass through an empty set of widget options.
 */

YWidget *YUIInterpreter::createWidgetTree(YWidget *parent, YRadioButtonGroup *rbg, const YCPTerm &term)
{
    YWidgetOpt opt;

    return createWidgetTree(parent, opt, rbg, term);
}



//
// =============================================================================
// High level (abstract libyui layer) widget creation functions.
// Most call a corresponding low level (specific UI) widget creation function.
// =============================================================================
//


/**
 * @widget	ReplacePoint
 * @short	Pseudo widget to replace parts of a dialog
 * @class	YReplacePoint
 * @arg		term child the child widget
 * @usage	`ReplacePoint(`id(`rp), `Empty())
 * @example	ReplacePoint1.ycp
 *
 * @description
 *
 * A ReplacePoint can be used to dynamically change parts of a dialog.
 * It contains one widget. This widget can be replaced by another widget
 * by calling <tt>ReplaceWidget(`id(id), newchild)</tt>, where <tt>id</tt> is the
 * the id of the new child widget of the replace point. The ReplacePoint widget
 * itself has no further effect and no optical representation.
 */

YWidget *YUIInterpreter::createReplacePoint(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					    int argnr, YRadioButtonGroup *rbg)
{
    if ( term->size() != argnr+1 ||
	 term->value( argnr ).isNull() ||
	 term->value( argnr )->isVoid()  )
    {
	y2error("Invalid arguments for the ReplacePoint widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YContainerWidget *replacePoint = createReplacePoint(parent, opt);

    if (replacePoint)
    {
	replacePoint->setParent( parent );
	YWidget *child = createWidgetTree(replacePoint, rbg, term->value(argnr)->asTerm());
	if (child) replacePoint->addChild(child);
	else
	{
	    delete replacePoint;
	    return 0;
	}
    }
    return replacePoint;
}


/**
 * @widgets	Empty HStretch VStretch HVStretch
 * @short	Stretchable space for layout
 * @class	YEmpty
 * @usage	`HStretch()
 * @example	HStretch1.ycp Layout-Buttons-Equal-Even-Spaced1.ycp
 *
 * @description
 *
 * These four widgets denote an empty place in the dialog. They differ
 * in whether they are stretchable or not. <tt>Empty</tt> is not stretchable
 * in either direction. It can be used in a <tt>`ReplacePoint</tt>, when
 * currently no real widget should be displayed. <tt>HStretch</tt> and <tt>VStretch</tt>
 * are stretchable horizontally or vertically resp., <tt>HVStretch</tt> is
 * stretchable in both directions. You can use them to control
 * the layout.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createEmpty(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				     int argnr, bool hstretchable, bool vstretchable)
{
    if (term->size() != argnr)
    {
	y2error("Invalid arguments for the %s widget: %s",
		term->symbol()->symbol().c_str(),term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);

    if ( hstretchable ) opt.isHStretchable.setValue(true);
    if ( vstretchable ) opt.isVStretchable.setValue(true);

    return createEmpty(parent, opt);
}



/**
 * @widgets	HSpacing VSpacing
 * @short	Fixed size empty space for layout
 * @class	YSpacing
 * @optarg	integer|float size
 * @usage	`HSpacing(0.3)
 * @example	Spacing1.ycp Layout-Buttons-Equal-Even-Spaced2.ycp
 *		Table2.ycp Table3.ycp
 *
 * @description
 *
 * These widgets can be used to create empty space within a dialog to avoid
 * widgets being cramped together - purely for aesthetical reasons. There is no
 * functionality attached.
 * <p>
 * <em>Do not try to use spacings with excessive sizes to create layouts!</em>
 * This is very likely to work for just one UI.	 Use spacings only to separate
 * widgets from each other or from dialog borders. For other purposes, use
 * <tt>`HWeight</tt> and <tt>`VWeight</tt> and describe the dialog logically
 * rather than physically.
 * <p>
 * The <tt>size</tt> given is measured in units roughly equivalent to the size
 * of a character in the respective UI. Fractional numbers can be used here,
 * but text based UIs may choose to round the number as appropriate - even if
 * this means simply ignoring a spacing when its size becomes zero.
 *
 * <p>If <tt>size</tt> is omitted, it defaults to 1.
 * <p><tt>HSpacing</tt> will create a horizontal spacing with default width and zero height.
 * <p><tt>VSpacing</tt> will create a vertical spacing with default height and zero width.
 * <br><p>
 * With options <tt>hstretch</tt> or <tt>vstretch</tt>, the spacing
 * will at least take the amount of space specified with
 * <tt>size</tt>, but it will be stretchable in the respective
 * dimension. Thus,
 * <p><tt>`HSpacing(`opt(`hstretch)</tt>
 * <p>is equivalent to
 * <p>`HBox( `HSpacing(0.5), `HSpacing(0.5) )</tt>
 * <br>
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createSpacing ( YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					 int argnr, bool horizontal, bool vertical)
{
    // Process parameters

    float size	  = 1.0;
    bool	 param_ok = false;

    if ( term->size() == argnr )			// no parameter
    {
	param_ok = true;
    }
    else if ( term->size() == argnr + 1 )	// one parameter
    {
	if ( term->value(argnr)->isInteger() )
	{
	    size	= (float) term->value(argnr)->asInteger()->value();
	    param_ok	= true;
	}
	else if ( term->value(argnr)->isFloat() )
	{
	    size	= term->value(argnr)->asFloat()->value();
	    param_ok	= true;
	}
    }

    if ( ! param_ok )
    {
	y2error("Invalid arguments for the %s widget: %s",
		term->symbol()->symbol().c_str(),term->toString().c_str());
	return 0;
    }


    rejectAllOptions(term,optList);
    return createSpacing(parent, opt, size, horizontal, vertical);
}



/**
 * @widgets	Left Right Top Bottom HCenter VCenter HVCenter
 * @short	Layout alignment
 * @class	YAlignment
 * @arg		term child The contained child widget
 * @optarg	boolean enabled true if ...
 * @usage	`Left(`CheckBox("Crash every five minutes"))
 * @example	HCenter1.ycp HCenter2.ycp HCenter3.ycp Alignment1.ycp
 *
 * @description
 *
 * The Alignment widgets are used to control the layout of a dialog. They are
 * useful in situations, where to a widget is assigned more space than it can
 * use. For example if you have a VBox containing four CheckBoxes, the width of
 * the VBox is determined by the CheckBox with the longest label. The other
 * CheckBoxes are centered per default. With <tt>`Left(widget)</tt> you tell
 * widget that it should be layouted leftmost of the space that is available to
 * it. <tt>Right, Top</tt> and <tt>Bottom</tt> are working accordingly.	 The
 * other three widgets center their child widget horizontally, vertically or in
 * both directions. The important fact for all alignment widgets is, that they
 * make their child widget <b>stretchable</b> in the dimension it is aligned.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createAlignment(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					 int argnr, YRadioButtonGroup *rbg,
					 YAlignmentType halign, YAlignmentType valign)
{
    if (term->size() != argnr+1)
    {
	y2error("%s: The alignment widgets take one widget as argument",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YAlignment *alignment = dynamic_cast <YAlignment *> ( createAlignment(parent, opt, halign, valign) );
    assert(alignment);

    if (alignment)
    {
	alignment->setParent( parent );
	YWidget *child = createWidgetTree(alignment, rbg, term->value(argnr)->asTerm());
	if (child) alignment->addChild(child);
	else
	{
	    delete alignment;
	    return 0;
	}
    }
    return alignment;
}



/**
 * @widgets	Frame
 * @short	Frame with label
 * @class	YFrame
 * @arg		string label title to be displayed on the top left edge
 * @arg		term child the contained child widget
 * @usage	`Frame(`RadioButtonGroup(`id(rb), `VBox(...)));
 * @examples	Frame1.ycp Frame2.ycp TextEntry5.ycp
 *
 * @description
 *
 * This widget draws a frame around its child and displays a title label within
 * the top left edge of that frame. It is used to visually group widgets
 * together. It is very common to use a frame like this around radio button
 * groups.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createFrame ( YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				       const YCPList &optList, int argnr, YRadioButtonGroup *rbg )
{

    int s = term->size() - argnr;
    if (s != 2
	|| ! term->value( argnr )->isString()
	|| ! term->value( argnr+1 )->isTerm() )
    {
	y2error("Invalid arguments for the Frame widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YCPString label = term->value( argnr++ )->asString();
    YContainerWidget *frame = createFrame( parent, opt, label );

    if ( frame )
    {
	frame->setParent( parent );
	YWidget *child = createWidgetTree( frame, rbg, term->value(argnr)->asTerm() );

	if ( child )
	{
	    frame->addChild( child );
	}
	else
	{
	    delete frame;
	    return 0;
	}
    }

    return frame;
}



/**
 * @widgets	HSquash VSquash HVSquash
 * @short	Layout aid: Minimize widget to its nice size
 * @class	YSquash
 * @arg		term child the child widget
 * @usage	HSquash(`TextEntry("Name:"))
 * @example	HSquash1.ycp
 *
 * @description
 *
 * The Squash widgets are used to control the layout. A <tt>HSquash</tt> widget
 * makes its child widget <b>nonstretchable</b> in the horizontal dimension.
 * A <tt>VSquash</tt> operates vertically, a <tt>HVSquash</tt> in both
 * dimensions.	You can used this for example to reverse the effect of
 * <tt>`Left</tt> making a widget stretchable. If you want to make a VBox
 * containing for left aligned CheckBoxes, but want the VBox itself to be
 * nonstretchable and centered, than you enclose each CheckBox with a
 * <tt>`Left(..)</tt> and the whole VBox with a <tt>HSquash(...)</tt>.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createSquash(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				      int argnr, YRadioButtonGroup *rbg,
				      bool hsquash, bool vsquash)
{
    if (term->size() != argnr+1)
    {
	y2error("%s: The squash widgets take one widget as argument",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YContainerWidget *squash = createSquash(parent, opt, hsquash, vsquash);

    if (squash)
    {
	squash->setParent( parent );
	YWidget *child = createWidgetTree(squash, rbg, term->value(argnr)->asTerm());
	if (child) squash->addChild(child);
	else
	{
	    delete squash;
	    return 0;
	}
    }
    return squash;
}



/**
 * @widgets	HWeight VWeight
 * @short	Control relative size of layouts
 * @class	(YWeight)
 * @arg		integer weight the new weight of the child widget
 * @arg		term child the child widget
 * @usage	`HWeight(2, `SelectionBox("Language"))
 * @examples	Weight1.ycp
 *		Layout-Buttons-Equal-Even-Spaced1.ycp
 *		Layout-Buttons-Equal-Even-Spaced2.ycp
 *		Layout-Buttons-Equal-Growing.ycp
 *		Layout-Mixed.ycp
 *		Layout-Weights1.ycp
 *		Layout-Weights2.ycp
 *
 * @description
 *
 * This widget is used to control the layout. When a <tt>HBox</tt> or
 * <tt>VBox</tt> widget decides how to devide remaining space amount two
 * <b>stretchable</b> widgets, their weights are taken into account. This
 * widget is used to change the weight of the child widget.  Each widget has a
 * vertical and a horizontal weight. You can change on or both of them.	 If you
 * use <tt>HVWeight</tt>, the weight in both dimensions is set to the same
 * value.
 * <p>
 * <b>Note:</b> No real widget is created (any more), just the weight value is
 * passed to the child widget.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createWeight(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				      int argnr, YRadioButtonGroup *rbg, YUIDimension dim)
{
    if (term->size() != argnr + 2
	|| !term->value(argnr)->isInteger()
	|| !term->value(argnr+1)->isTerm())
    {
	y2error("Invalid arguments for the Weight widget: %s",
		term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);
    long weightValue = (long)(term->value(argnr)->asInteger()->value());

    /**
     * This is an exception from the general rule: No YWeight widget is created,
     * just the weight is passed as widget options to a newly created child widget.
     * The YWeight widget is plain superfluos - YWidget can handle everything itself.
     */

    YWidgetOpt childOpt;

    if (dim == YD_HORIZ)	childOpt.hWeight.setValue(weightValue);
    else			childOpt.vWeight.setValue(weightValue);

    return createWidgetTree(parent, childOpt,rbg, term->value(argnr+1)->asTerm());
}



/**
 * @widgets	HBox VBox
 * @short	Generic layout: Arrange widgets horizontally or vertically
 * @class	(Box)
 * @optarg	term child1 the first child widget
 * @optarg	term child2 the second child widget
 * @optarg	term child3 the third child widget
 * @optarg	term child4 the fourth child widget (and so on...)
 * @option	debugLayout verbose logging
 * @usage	HBox(`PushButton(`id(`ok), "OK"), `PushButton(`id(`cancel), "Cancel"))
 *
 * @examples	VBox1.ycp HBox1.ycp
 *		Layout-Buttons-Equal-Growing.ycp
 *		Layout-Fixed.ycp
 *		Layout-Mixed.ycp
 *
 * @description
 *
 * The layout boxes are used to split up the dialog and layout a number of
 * widgets horizontally (<tt>HBox</tt>) or vertically (<tt>VBox</tt>).
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createLBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				    int argnr, YRadioButtonGroup *rbg, YUIDimension dim)
{
    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if   (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_debugLayout)	opt.debugLayoutMode.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YContainerWidget *split = createSplit(parent, opt, dim);

    if (split)
    {
	split->setParent(parent);

	for (int w=argnr; w<term->size(); w++)
	{
	    // Create and add the next child widget.

	    if (!term->value(w)->isTerm())
	    {
		y2error("%s: Should be a widget specification",
			term->value(w)->toString().c_str());
		delete split;
		return 0;
	    }

	    YWidget *child = createWidgetTree(split, rbg, term->value(w)->asTerm());

	    if (!child )
	    {
		delete split;
		return 0;
	    }

	    split->addChild(child);
	}
    }
    return split;
}



/**
 * @widgets	Label Heading
 * @short	Simple static text
 * @class	YLabel
 * @arg		string label
 * @option	outputField make the label look like an input field in read-only mode
 * @usage	`Label("Here goes some text\nsecond line")
 *
 * @examples	Label1.ycp Label2.ycp Label3.ycp
 *		Heading1.ycp Heading2.ycp Heading3.ycp
 *
 * @description
 *
 * A <tt>Label</tt> is some text displayed in the dialog. A <tt>Heading</tt> is
 * a text with a font marking it as heading. The text can have more than one
 * line, in which case line feed must be entered.
 */

YWidget *YUIInterpreter::createLabel(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				     const YCPList &optList, int argnr, bool heading)
{
    if (term->size() - argnr != 1
	|| !term->value(argnr)->isString())
    {
	y2error("Invalid arguments for the Label widget: %s",
		term->toString().c_str());
	return 0;
    }


    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_outputField) opt.isOutputField.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if(heading) opt.isHeading.setValue(true);

    return createLabel(parent, opt, term->value(argnr)->asString());
}



/**
 * @widget	RichText
 * @short	Static text with HTML-like formatting
 * @class	YRichText
 * @arg		string text
 * @option	plainText don't interpret text as HTML
 * @option	autoScrollDown automatically scroll down for each text change
 * @option	shrinkable make the widget very small
 * @usage	`RichText("This is a <b>bold</b> text")
 * @example	RichText1.ycp
 *
 * @description
 *
 * A <tt>RichText</tt> is a text area with two major differences to a
 * <tt>Label</tt>: The amount of data it can contain is not restricted by the
 * layout and a number of control sequences are allowed, which control the
 * layout of the text.
 * <p>
 * Refer to the <a href="../YCP-UI-richtext.html">YaST2 RichText specification</a> for details.
 */

YWidget *YUIInterpreter::createRichText(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 1
	|| !term->value(argnr)->isString())
    {
	y2error("Invalid arguments for the Label RichText: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() )
	{
	    std::string sym = optList->value(o)->asSymbol()->symbol();

	    if      ( sym  == YUIOpt_plainText      ) opt.plainTextMode.setValue(true);
	    else if ( sym  == YUIOpt_autoScrollDown ) opt.autoScrollDown.setValue(true);
	    else if ( sym  == YUIOpt_shrinkable     ) opt.isShrinkable.setValue(true);
	    else    logUnknownOption(term, optList->value(o));
	}
	else logUnknownOption(term, optList->value(o));
    }

    return createRichText(parent, opt, term->value(argnr)->asString());
}



/**
 * @widget	LogView
 * @short	scrollable log lines like "tail -f"
 * @class	YLogView
 * @arg		string label (above the log lines)
 * @arg		integer visibleLines number of visible lines (without scrolling)
 * @arg		integer maxLines number of log lines to store (use 0 for "all")
 * @usage	`LogView("Log file", 4, 200);
 * @example	LogView1.ycp
 *
 * @description
 *
 * A scrolled output-only text window where ASCII output of any kind can be
 * redirected - very much like a shell window with "tail -f".
 * <p>
 * The LogView will keep up to "maxLines" of output, discarding the oldest
 * lines if there are more. If "maxLines" is set to 0, all lines will be kept.
 * <p>
 * "visibleLines" lines will be visible by default (without scrolling) unless
 * you stretch the widget in the layout.
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `LastLine, "bla blurb...\n")</tt> to append
 * one or several line(s) to the output. Notice the newline at the end of each line!
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `Value, "bla blurb...\n")</tt> to replace
 * the entire contents of the LogView.
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `Value, "")</tt> to clear the contents.
 */

YWidget *YUIInterpreter::createLogView(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 3
	|| ! term->value(argnr  )->isString()
	|| ! term->value(argnr+1)->isInteger()
	|| ! term->value(argnr+2)->isInteger())
    {
	y2error("Invalid arguments for the LogView widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);

    YCPString label	= term->value(argnr)->asString();
    int visibleLines	= term->value(argnr+1)->asInteger()->value();
    int maxLines	= term->value(argnr+2)->asInteger()->value();

    return createLogView(parent, opt, label, visibleLines, maxLines);
}



/**
 * @widget	PushButton IconButton
 * @short	Perform action on click
 * @class	YPushButton
 * @arg		string iconName (IconButton only)
 * @arg		string label
 * @option	default makes this button the dialogs default button
 * @usage	`PushButton(`id(`click), `opt(`default, `hstretch), "Click me")
 * @examples	PushButton1.ycp PushButton2.ycp IconButton1.ycp
 *
 * @description
 *
 * A <tt>PushButton</tt> is a button with a text label the user can
 * press in order to activate some action. If you call <tt>UserInput()</tt> and
 * the user presses the button, <tt>UserInput()</tt> returns with the id of the
 * pressed button.
 * <p>
 * You can (and should) provide keybord shortcuts along with the button
 * label. For example "&Apply" as a button label will allow the user to
 * activate the button with Alt-A, even if it currently doesn't have keyboard
 * focus. This is important for UIs that don't support using a mouse.
 * <p>
 * An <tt>IconButton</tt> is pretty much the same, but it has an icon in
 * addition to the text. If the UI cannot handle icons, it displays only the
 * text, and the icon is silently omitted.
 * <p>
 * Icons are (at the time of this writing) loaded from the <i>theme</i>
 * directory, <tt>/usr/share/YaST2/theme/current</tt>.
 */

YWidget *YUIInterpreter::createPushButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					  const YCPList &optList, int argnr,
					  bool isIconButton )
{
    YCPString label( "" );
    YCPString iconName( "" );

    if ( isIconButton )
    {
	if ( term->size() - argnr != 2
	     || ! term->value(argnr)->isString()
	     || ! term->value(argnr+1)->isString() )
	{
	    y2error("Invalid arguments for the IconButton widget: %s",
		    term->toString().c_str());
	    return 0;
	}

	iconName = term->value( argnr   )->asString();
	label    = term->value( argnr+1 )->asString();
    }
    else
    {
	if ( term->size() - argnr != 1
	     || !term->value(argnr)->isString() )
	{
	    y2error("Invalid arguments for the PushButton widget: %s",
		    term->toString().c_str());
	    return 0;
	}

	label = term->value( argnr )->asString();
    }

    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() )
	{
	    std::string sym = optList->value(o)->asSymbol()->symbol();

	    if   ( sym == YUIOpt_default ) opt.isDefaultButton.setValue( true );
	    else logUnknownOption(term, optList->value(o));
	}
	else logUnknownOption(term, optList->value(o));
    }


    // Look up default function keys unless explicitly set
    
    if ( opt.key_Fxx.value() == 0 )
	opt.key_Fxx.setValue( defaultFunctionKey( label ) );


    YPushButton * button = dynamic_cast<YPushButton *>
	( createPushButton(parent, opt, label ) );

    if ( isIconButton && button )
	button->setIcon( iconName );

    return button;
}


/**
 * @widget	MenuButton
 * @short	Button with popup menu
 * @class	YMenuButton
 * @arg		string		label
 * @arg		itemList	menu items
 * @usage	`MenuButton( "button label", [ `item(`id(`doit), "&Do it"), `item(`id(`something), "&Something") ] );
 * @examples	MenuButton1.ycp MenuButton2.ycp
 *
 * @description
 *
 * This is a widget that looks very much like a <tt>PushButton</tt>, but unlike
 * a <tt>PushButton</tt> it doesn't immediately start some action but opens a
 * popup menu from where the user can select an item that starts an action. Any
 * item may in turn open a submenu etc.
 * <p>
 * <tt>UserInput()</tt> returns the ID of a menu item if one was activated. It
 * will never return the ID of the <tt>MenuButton</tt> itself.
 * <p>
 * <b>Style guide hint:</b> Don't overuse this widget. Use it for dialogs that
 * provide lots of actions. Make the most frequently used actions accessible
 * via normal <tt>PushButtons</tt>. Move less frequently used actions
 * (e.g. "expert" actions) into one or more <tt>MenuButtons</tt>. Don't nest
 * the popup menus too deep - the deeper the nesting, the more awkward the user
 * interface will be.
 * <p>
 * You can (and should) provide keybord shortcuts along with the button
 * label as well as for any menu item.
 */

YWidget *YUIInterpreter::createMenuButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if ( term->size() - argnr != 2
	 || ! term->value(argnr  )->isString()
	 || ! term->value(argnr+1)->isList()  )
    {
	y2error("Invalid arguments for the MenuButton widget: "
		"expected \"label\", [ `item(), `item(), ...], not %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);

    YMenuButton *menu_button = dynamic_cast <YMenuButton *>
	( createMenuButton( parent, opt, term->value(argnr)->asString() ) );

    if ( ! menu_button ||
	 parseMenuItemList( term->value( argnr+1 )->asList(), menu_button ) == -1 )
    {
	return 0;
    }

    menu_button->createMenu();	// actually create the specific UI's menu hierarchy

    return menu_button;
}



int
YUIInterpreter::parseMenuItemList( const YCPList &	itemList,
				   YMenuButton *	menu_button,
				   YMenu *		parent_menu )	// 0 if top level
{
    for ( int i=0; i < itemList->size(); i++ )
    {
	YCPValue item = itemList->value(i);

	if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() != 2 ||
		 ! iterm->value( 0 )->isTerm()	||	// `id(...)
		 ! iterm->value( 1 )->isString()  )	// "menu item label"
	    {
		y2error("MenuButton: Invalid menu item - expected `item(`id(...), \"label\"), not %s",
			iterm->toString().c_str() );

		return -1;
	    }


	    // check for item `id() (mandatory)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( 0 ), true ) )
	    {
		item_id = getId ( iterm->value( 0 ) );
	    }
	    else	// no `id()
	    {
		y2error( "MenuButton: Invalid menu item - no `id() specified: %s",
			 item->toString().c_str() );

		return -1;
	    }


	    // extract item label (mandatory) and create the item

	    YCPString item_label = iterm->value( 1 )->asString();
	    menu_button->addMenuItem( item_label, item_id, parent_menu );
	    // y2debug( "Inserted menu entry '%s'", item_label->value().c_str() );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_menu )
	{
	    // found `menu()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() != 2 ||
		 ! iterm->value( 0 )->isString() &&	// "submenu label"
		 ! iterm->value( 1 )->isList()     )	// [ `item(...), `item(...) ] )
	    {
		y2error( "MenuButton: Invalid submenu specification: "
			 "expected `menu( \"submenu label\", [ `item(), `item(), ...] ), not %s",
			 item->toString().c_str() );

		return -1;
	    }

	    YCPString	sub_menu_label	= iterm->value( 0 )->asString();
	    YMenu *	sub_menu	= menu_button->addSubMenu( sub_menu_label, parent_menu );
	    // y2debug( "Inserted sub menu '%s'", sub_menu_label->value().c_str() );

	    if ( parseMenuItemList( iterm->value( 1 )->asList(), menu_button, sub_menu ) == -1 )
	    {
		return -1;
	    }
	}
	else
	{
	    y2error("MenuButton: Invalid menu item - use either `item() or `menu(), not %s",
		    item->toString().c_str() );
	    return -1;
	}
    }

    return 0;
}



/**
 * @widget	CheckBox
 * @short	Clickable on/off toggle button
 * @class	YCheckBox
 * @arg		string label the text describing the check box
 * @optarg	boolean|nil checked whether the check box should start checked -
 *		nil means tristate condition, i.e. neither on nor off
 * @usage	`CheckBox(`id(`cheese), "&Extra cheese")
 * @examples	CheckBox1.ycp CheckBox2.ycp CheckBox3.ycp
 *
 * @description
 *
 * A checkbox widget has two states: Checked and not checked. It returns no
 * user input but you can query and change its state via the <tt>Value</tt>
 * property.
 */

YWidget *YUIInterpreter::createCheckBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int s = term->size() - argnr;
    if (s < 1 || s > 2
	|| !term->value(argnr)->isString()
	|| (s == 2 && !term->value(argnr+1)->isBoolean()))
    {
	y2error("Invalid arguments for the CheckBox widget: %s",
		term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);
    YCPBoolean checked(false);
    if (s == 2) checked = term->value(argnr+1)->asBoolean();
    return createCheckBox(parent, opt, term->value(argnr)->asString(), checked->value());
}



/**
 * @widget	RadioButton
 * @short	Clickable on/off toggle button for radio boxes
 * @class	YRadioButton
 * @arg		string label
 * @optarg	boolean selected
 * @usage	`RadioButton(`id(`now), "Crash now", true)
 * @examples	RadioButton1.ycp RadioButton2.ycp Frame2.ycp
 *
 * @description
 *
 * A radio button is not usefull alone. Radio buttons are group such that the
 * user can select one radio button of a group. It is much like a selection
 * box, but radio buttons can be dispersed over the dialog.  Radio buttons must
 * be contained in a <tt>RadioButtonGroup</tt>.
 */

YWidget *YUIInterpreter::createRadioButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					   int argnr, YRadioButtonGroup *rbg)
{

    int s = term->size() - argnr;
    if (s < 1 || s > 2
	|| !term->value(argnr)->isString()
	|| (s == 2 && !term->value(argnr+1)->isBoolean()))
    {
	y2error("Invalid arguments for the RadioButton widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YCPBoolean checked(false);
    if (s == 2) checked = term->value(argnr+1)->asBoolean();

    if (!rbg)
    {
	y2error("%s: must be inside a RadioButtonGroup",
		term->toString().c_str());
	return 0;
    }

    YRadioButton *radioButton = dynamic_cast <YRadioButton *> ( createRadioButton( parent, opt, rbg, term->value(argnr)->asString(), checked->value() ) );
    assert(radioButton);

    // Add to radiobutton group. This can _not_ be done in the
    // constructor of YRadioButton, since the ui specific widget is not yet
    // constructed yet at this stage.

    rbg->addRadioButton(radioButton);

    return radioButton;
}



/**
 * @widget	RadioButtonGroup
 * @short	Radio box - select one of many radio buttons
 * @class	YRadioButtonGroup
 * @arg		term child the child widget
 * @usage	`RadioButtonGroup(`id(rb), `VBox(...))
 * @examples	RadioButton1.ycp Frame2.ycp
 *
 * @description
 *
 * A <tt>RadioButtonGroup</tt> is a container widget that has neither impact on
 * the layout nor has it a graphical representation. It is just used to
 * logically group RadioButtons together so the one-out-of-many selection
 * strategy can be ensured.
 * <p>
 * Radio button groups may be nested.  Looking bottom up we can say that a
 * radio button belongs to the radio button group that is nearest to it. If you
 * give the <tt>RadioButtonGroup</tt> widget an id, you can use it to query and
 * set which radio button is currently selected.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createRadioButtonGroup(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
						int argnr, YRadioButtonGroup *)
{
    if (term->size() != argnr+1
	|| !term->value(argnr)->isTerm())
    {
	y2error("Invalid arguments for the RadioButtonGroup widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YRadioButtonGroup *rbg = dynamic_cast <YRadioButtonGroup *> ( createRadioButtonGroup(parent, opt) );
    assert(rbg);

    rbg->setParent( parent );
    YWidget *child = createWidgetTree(rbg, rbg, term->value(argnr)->asTerm());

    if (child) rbg->addChild(child);
    else
    {
	delete rbg;
	return 0;
    }

    return rbg;
}



/**
 * @widgets	TextEntry Password
 * @short	Input field
 * @class	YTextEntry
 * @arg		string label the label describing the meaning of the entry
 * @optarg	string defaulttext The text contained in the text entry
 * @option	shrinkable make the input field very small
 * @usage	`TextEntry(`id(`name), "Enter your name:", "Kilroy")
 *
 * @examples	TextEntry1.ycp TextEntry2.ycp TextEntry3.ycp TextEntry4.ycp
 *		TextEntry5.ycp TextEntry6.ycp
 *		Password1.ycp Password2.ycp
 *
 * @description
 *
 * This widget is a one line text entry field with a label above it. An initial
 * text can be provided.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the corresponding text entry widget
 * will get the keyboard focus.
 */

YWidget *YUIInterpreter::createTextEntry(YWidget *parent, YWidgetOpt &opt,
					 const YCPTerm &term, const YCPList &optList, int argnr,
					 bool passwordMode)
{

    if (term->size() - argnr < 1 || term->size() - argnr > 2
	|| !term->value(argnr)->isString()
	|| (term->size() == argnr+2 && !term->value(argnr+1)->isString()))
    {
	y2error("Invalid arguments for the %s widget: %s",
		passwordMode ? "Password" : "TextEntry",
		term->toString().c_str());
	return 0;
    }
    YCPString initial_text("");
    if (term->size() >= argnr + 2) initial_text = term->value(argnr+1)->asString();

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if ( passwordMode ) opt.passwordMode.setValue(true);

    return createTextEntry(parent, opt, term->value(argnr)->asString(), initial_text);
}


/**
 * @widgets	MultiLineEdit
 * @short	multiple line text edit field
 * @class	YMultiLineEdit
 * @arg		string label label above the field
 * @optarg	string initialText the initial contents of the field
 * @usage	`MultiLineEdit(`id(`descr), "Enter problem &description:", "No problem here.")
 *
 * @examples	MultiLineEdit1.ycp MultiLineEdit2.ycp MultiLineEdit3.ycp
 *
 * @description
 *
 * This widget is a multiple line text entry field with a label above it.
 * An initial text can be provided.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the corresponding MultiLineEdit
 * widget will get the keyboard focus.
 */

YWidget *YUIInterpreter::createMultiLineEdit(YWidget *parent, YWidgetOpt &opt,
					     const YCPTerm &term, const YCPList &optList, int argnr)
{

    if (term->size() - argnr < 1 || term->size() - argnr > 2
	|| !term->value(argnr)->isString()
	|| (term->size() == argnr+2 && !term->value(argnr+1)->isString()))
    {
	y2error("Invalid arguments for the MultiLineEdit widget: %s",
		term->toString().c_str() );
	return 0;
    }

    YCPString initial_text("");
    if (term->size() >= argnr + 2) initial_text = term->value(argnr+1)->asString();

    rejectAllOptions(term,optList);
    return createMultiLineEdit(parent, opt, term->value(argnr)->asString(), initial_text);
}



/**
 * @widget	SelectionBox
 * @short	Scrollable list selection
 * @class	YSelectionBox
 * @arg		string label
 * @optarg	list items the items contained in the selection box
 * @option	shrinkable make the widget very small
 * @option	immediate (NCurses UI special) make `notify trigger immediately when the selected item changes
 * @usage	`SelectionBox(`id(`pizza), "select your Pizza:", [ "Margarita", `item(`id(`na), "Napoli") ])
 * @examples	SelectionBox1.ycp SelectionBox2.ycp SelectionBox3.ycp SelectionBox4.ycp
 *
 * @description
 *
 * A selection box offers the user to select an item out of a list. Each item
 * has a label and an optional id. When constructing the list of items, you
 * have two way of specifying an item. Either you give a plain string, in which
 * case the string is used both for the id and the label of the item. Or you
 * specify a term <tt>`item(term id, string label)</tt> or <tt>`item(term id,
 * string label, boolean selected)</tt>, where you give an id of the form
 * <tt>`id(any v)</tt> where you can store an aribtrary value as id. The third
 * argument controls whether the item is the selected item.
 */

YWidget *YUIInterpreter::createSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isString()
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the SelectionBox widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable ) opt.isShrinkable.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_immediate  )
	{
	    opt.notifyMode.setValue(true);
	    opt.immediateMode.setValue(true);
	}
	else logUnknownOption(term, optList->value(o));
    }

    YSelectionBox *selbox = dynamic_cast <YSelectionBox *> ( createSelectionBox(parent, opt, term->value(argnr)->asString()) );
    assert(selbox);
    if ( numargs == 2)
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	for (int i=0; i<itemlist->size(); i++)
	{
	    YCPValue item = itemlist->value(i);
	    if (item->isString())
	    {
		selbox->addItem(YCPNull(), item->asString(), false);
	    }
	    else if (item->isTerm()
		     && item->asTerm()->symbol()->symbol() == YUISymbol_item)
	    {
		YCPTerm iterm = item->asTerm();
		if (iterm->size() < 1 || iterm->size() > 3)
		{
		    y2error("SelectionBox: Invalid argument number in %s",
			    iterm->toString().c_str());
		}
		else
		{
		    int argnr = checkId(iterm->value(0)) ? 1 : 0;
		    if (iterm->size() <= argnr || !iterm->value(argnr)->isString())
			y2error("SelectionBox: Invalid item arguments in %s",
				iterm->toString().c_str());
		    else
		    {
			YCPValue item_id = YCPNull();
			if (argnr == 1) item_id = getId(iterm->value(0));
			YCPString item_label = iterm->value(argnr)->asString();
			bool item_selected = false;
			if (iterm->size() >= argnr + 2)
			{
			    if (iterm->value(argnr+1)->isBoolean())
				item_selected = iterm->value(argnr+1)->asBoolean()->value();
			    else
			    {
				y2error("SelectionBox: Invalid item arguments in %s",
					iterm->toString().c_str());
			    }
			}

			// UFF! It's made. All arguments checked and gathered :-)
			selbox->addItem(item_id, item_label, item_selected);
		    }
		}
	    }
	    else
	    {
		y2error("Invalid item %s: SelectionBox items must be strings or specified with `"
			YUISymbol_item "()", item->toString().c_str());
	    }
	    // `item
	} // loop over items
    }
    return selbox;
}


/**
 * @widget	MultiSelectionBox
 * @short	Selection box that allows selecton of multiple items
 * @class	YMultiSelectionBox
 * @arg		string	label
 * @arg		list	items	the items initially contained in the selection box
 * @option	shrinkable make the widget very small
 * @usage	`MultiSelectionBox(`id(`topping), "select pizza toppings:", [ "Salami", `item(`id(`cheese), "Cheese", true) ])
 * @examples	MultiSelectionBox1.ycp MultiSelectionBox2.ycp MultiSelectionBox3.ycp
 *
 * @description
 *
 * The MultiSelectionBox displays a (scrollable) list of items from which any
 * number (even nothing!) can be selected. Use the MultiSelectionBox's
 * <tt>SelectedItems</tt> property to find out which.
 * <p>
 * Each item can be specified either as a simple string or as
 * <tt>`item(...)</tt> which includes an (optional) ID and an (optional)
 * 'selected' flag that specifies the initial selected state ('not selected',
 * i.e. 'false', is default).
 */

YWidget *YUIInterpreter::createMultiSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int term_size = term->size() - argnr;

    if ( term_size != 2
	 || ! term->value( argnr   )->isString()
	 || ! term->value( argnr+1 )->isList() )
    {
	y2error("Invalid arguments for the MultiSelectionBox widget: %s",
		term->toString().c_str());

	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YMultiSelectionBox *multi_sel_box = dynamic_cast <YMultiSelectionBox *>
	( createMultiSelectionBox( parent, opt, term->value(argnr)->asString() ) );
    assert( multi_sel_box );


    if ( ! multi_sel_box ||
	 parseMultiSelectionBoxItemList( term->value ( argnr+1 )->asList(), multi_sel_box ) < 0 )
    {
	return 0;
    }

    return multi_sel_box;
}


int
YUIInterpreter::parseMultiSelectionBoxItemList( const YCPList &		item_list,
						YMultiSelectionBox *	multi_sel_box )
{
    multi_sel_box->deleteAllItems();

    for ( int i=0; i < item_list->size(); i++ )
    {
	YCPValue item = item_list->value(i);

	if ( item->isString() )
	{
	    // Simple case: string

	    multi_sel_box->addItem( item->asString() );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // Found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() < 1 || iterm->size() > 3 )	// `item( `id(...), "label", true )
	    {
		y2error( "MultiSelectionBox: Invalid argument number in %s",
			 iterm->toString().c_str() );

		return -1;
	    }

	    int argnr = 0;


	    // check for item `id() (optional)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( argnr ), false ) )
	    {
		item_id = getId ( iterm->value ( argnr++ ) );
	    }


	    // extract item label (mandatory)

	    if ( iterm->size() <= argnr || ! iterm->value(argnr)->isString() )
	    {
		y2error( "MultiSelectionBox: Expected item label string, not %s",
			 iterm->toString().c_str() );

		return -1;
	    }

	    YCPString item_label = iterm->value( argnr++ )->asString();


	    bool item_selected = false;

	    if ( argnr < iterm->size() )
	    {
		// check for 'selected' flag (true/false) (optional)

		if ( iterm->value( argnr )->isBoolean() )
		{
		    item_selected = iterm->value( argnr++ )->asBoolean()->value();
		}
	    }


	    // Anything left over must be an error.

	    if ( argnr != iterm->size() )
	    {
		y2error("MultiSelectinBox: Wrong number of arguments in %s",
			item->toString().c_str() );

		return -1;
	    }

	    multi_sel_box->addItem( item_label, item_id, item_selected );
	}
	else
	{
	    y2error("MultiSelectionBox: Invalid item - use either a "
		    "simple string or `item(`opt(...), \"label\", true/false), not %s",
		    item->toString().c_str() );
	    return -1;
	}
    }

    return 0;
}


/**
 * @widget	ComboBox
 * @short	drop-down list selection (optionally editable)
 * @class	YComboBox
 * @arg		string label
 * @optarg	list items the items contained in the combo box
 * @option	editable the user can enter any value.
 * @usage	`ComboBox(`id(`pizza), "select your Pizza:", [ "Margarita", `item(`id(`na), "Napoli") ])
 * @examples	ComboBox1.ycp ComboBox2.ycp ComboBox3.ycp ComboBox4.ycp
 *
 * @description
 *
 * A combo box is a combination of a selection box and an input field. It gives
 * the user a one-out-of-many choice from a list of items.  Each item has a
 * (mandatory) label and an (optional) id.  When the 'editable' option is set,
 * the user can also freely enter any value. By default, the user can only
 * select an item already present in the list.
 * <p>
 * The items are very much like SelectionBox items: They can have an (optional)
 * ID, they have a mandatory text to be displayed and an optional boolean
 * parameter indicating the selected state. Only one of the items may have this
 * parameter set to "true"; this will be the default selection on startup.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the combo box will get the keyboard
 * focus.
 */

YWidget *YUIInterpreter::createComboBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isString()
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the ComboBox widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_editable) opt.isEditable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YComboBox *cbox = dynamic_cast <YComboBox *> ( createComboBox(parent, opt, term->value(argnr)->asString()) );
    assert(cbox);
    if ( numargs == 2)
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	for (int i=0; i<itemlist->size(); i++)
	{
	    YCPValue item = itemlist->value(i);
	    if (item->isString())
	    {
		cbox->addItem(YCPNull(), item->asString(), false);
	    }
	    else if (item->isTerm()
		     && item->asTerm()->symbol()->symbol() == YUISymbol_item)
	    {
		YCPTerm iterm = item->asTerm();
		if (iterm->size() < 1 || iterm->size() > 3)
		{
		    y2error("ComboBox: Invalid argument number in %s",
			    iterm->toString().c_str());
		}
		else
		{
		    int argnr = checkId(iterm->value(0)) ? 1 : 0;
		    if (iterm->size() <= argnr || !iterm->value(argnr)->isString())
			y2error("ComboBox: Invalid item arguments in %s",
				iterm->toString().c_str());
		    else
		    {
			YCPValue item_id = YCPNull();
			if (argnr == 1) item_id = getId(iterm->value(0));
			YCPString item_label = iterm->value(argnr)->asString();
			bool item_selected = false;
			if (iterm->size() >= argnr + 2)
			{
			    if (iterm->value(argnr+1)->isBoolean())
				item_selected = iterm->value(argnr+1)->asBoolean()->value();
			    else
			    {
				y2error("ComboBox: Invalid item arguments in %s",
					iterm->toString().c_str());
			    }
			}

			// We're all set - all arguments checked and gathered.
			cbox->addItem(item_id, item_label, item_selected);
		    }
		}
	    }
	    else
	    {
		y2error("Invalid item %s: ComboBox items must be strings or specified with `"
			YUISymbol_item "()", item->toString().c_str());
	    }
	    // `item
	} // loop over items
    }
    return cbox;
}



/**
 * @widget	Tree
 * @short	Scrollable tree selection
 * @class	YTree
 * @arg		string		label
 * @optarg	itemList	items	the items contained in the tree <br>
 *		<br>
 *		itemList ::= <br>
 *		<blockquote>
 *			<tt><b>[</b></tt>		<br>
 *			<blockquote>
 *				item			<br>
 *				[ , item ]		<br>
 *				[ , item ]		<br>
 *				...			<br>
 *			</blockquote>
 *			<tt><b>]</b></tt> <br>
 *		</blockquote>
 *		<br>
 *		item ::= <br>
 *		<blockquote>
 *			string |		<br>
 *			<tt><b>`item(</b></tt>	<br>
 *			<blockquote>
 *				[ <tt><b>`id(</b></tt> string <tt><b>),</b></tt> ] <br>
 *				string			<br>
 *				[ , true | false ]	<br>
 *				[ , itemList ]		<br>
 *			</blockquote>
 *			<tt><b>)</b></tt>		<br>
 *		</blockquote>
 *		<br>
 *		The boolean parameter inside `item() indicates whether or not
 *		the respective tree item should be opened by default - if it
 *		has any subitems and if the respective UI is capable of closing
 *		and opening subtrees. If the UI cannot handle this, all
 *		subtrees will always be open.
 *
 * @usage	`Tree(`id(`treeID), "treeLabel", [ "top1", "top2", "top3" ] );
 * @examples	Tree1.ycp Tree2.ycp
 *
 * @description
 *
 * A tree widget provides a selection from a hierarchical tree structure. The
 * semantics are very much like those of a SelectionBox. Unlike the
 * SelectionBox, however, tree items may have subitems that in turn may have
 * subitems etc.
 * <p>
 * Each item has a label string, optionally preceded by an ID. If the item has
 * subitems, they are specified as a list of items after the string.
 * <p>
 * The tree widget will not perform any sorting on its own: The items are
 * always sorted by insertion order. The application needs to handle sorting
 * itself, if desired.
 */

YWidget *YUIInterpreter::createTree (YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int termSize = term->size() - argnr;

    if ( termSize < 1 || termSize > 2
	 || ! term->value( argnr )->isString() )
    {
	y2error("Invalid arguments for the Tree widget: %s",
		term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);
    YTree *tree = dynamic_cast <YTree *> ( createTree ( parent, opt, term->value ( argnr )->asString() ) );
    assert(tree);

    if ( tree && termSize > 1 )
    {
	if ( ! term->value( argnr+1 )->isList() )
	{
	    y2error("Expecting tree item list instead of: %s",
		    term->value( argnr+1 )->toString().c_str() );

	    return 0;
	}

	if ( parseTreeItemList ( term->value ( argnr+1 )->asList(), tree ) == -1 )
	    return 0;
    }

    if ( tree )
    {
	tree->rebuildTree();
    }

    return tree;
}


int
YUIInterpreter::parseTreeItemList ( const YCPList &	itemList,
				    YTree *		tree,
				    YTreeItem *		parentItem )
{
    for ( int i=0; i < itemList->size(); i++ )
    {
	YCPValue item = itemList->value(i);

	if ( item->isString() )
	{
	    // The simplest case: just a string, nothing else

	    (void) tree->addItem ( parentItem, YCPNull(), item->asString(), false );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() < 1 || iterm->size() > 4 )
	    {
		y2error("Tree: Invalid argument number in %s",
			iterm->toString().c_str() );

		return -1;
	    }

	    int argnr = 0;


	    // check for item `id() (optional)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( argnr ), false ) )
	    {
		item_id = getId ( iterm->value ( argnr++ ) );
	    }


	    // extract item label (mandatory)

	    if ( iterm->size() <= argnr || ! iterm->value(argnr)->isString() )
	    {
		y2error("Tree: Invalid item arguments in %s",
			iterm->toString().c_str() );

		return -1;
	    }

	    YCPString item_label = iterm->value( argnr++ )->asString();


	    bool item_open = false;

	    if ( argnr < iterm->size() )
	    {
		// check for 'open' flag (true/false) (optional)

		if ( iterm->value( argnr )->isBoolean() )
		{
		    item_open = iterm->value( argnr++ )->asBoolean()->value();
		}
	    }

	    YTreeItem * treeItem = tree->addItem ( parentItem, item_id, item_label, item_open );

	    if ( argnr < iterm->size() )
	    {
		// check for sub-item list (optional)

		if ( ! iterm->value( argnr )->isList() )
		{
		    y2error("Expecting tree item list instead of: %s",
			    iterm->value( argnr )->toString().c_str() );

		    return -1;
		}

		if ( parseTreeItemList ( iterm->value ( argnr++ )->asList(), tree, treeItem ) == -1 )
		{
		    return -1;
		}
	    }


	    // Anything left over must be an error.

	    if ( argnr != iterm->size() )
	    {
		y2error("Tree: Wrong number of arguments in %s", item->toString().c_str() );
	    }
	}
	else
	{
	    y2error("Invalid item %s: Tree items must be strings or specified with `"
		    YUISymbol_item "()", item->toString().c_str() );
	}
    }

    return 0;
}



/**
 * @widget	Table
 * @short	Multicolumn table widget
 * @class	YTable
 * @arg		term header the headers of the columns
 * @optarg	list items the items contained in the selection box
 * @option	immediate make `notify trigger immediately when the selected item changes
 * @option	keepSorting keep the insertion order - don't let the user sort manually by clicking
 * @usage	`Table(`header("Game", "Highscore"), [ `item(`id(1), "xkobo", "1708") ])
 * @examples	Table1.ycp Table2.ycp Table3.ycp Table4.ycp Table5.ycp
 *
 * @description
 *
 * A Table widget is a generalization of the SelectionBox. Information is
 * displayed in a number of columns. Each column has a header.	The number of
 * columns and their titles are described by the first argument, which is a
 * term with the symbol <tt>header</tt>. For each column you add a string
 * specifying its title. For example <tt>`header("Name", "Price")</tt> creates
 * the two columns "Name" and "Price".
 * <p>
 * The second argument is an optional list of items (rows) that are inserted in
 * the table. Each item has the form <tt>`item(`id(</tt> id <tt>), first
 * column, second column, ...)</tt>. For each column one argument has to be
 * specified, which must be of type void, string or integer. Strings are being
 * left justified, integer right and a nil denote an empty cell, just as the
 * empty string.
 */

YWidget *YUIInterpreter::createTable(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isTerm()
	|| term->value(argnr)->asTerm()->symbol()->symbol() != YUISymbol_header
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the Table widget: %s",
		term->toString().c_str());
	return 0;
    }

    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_immediate  ) opt.immediateMode.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_keepSorting) opt.keepSorting.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    // Build header. The header is a vector of strings, each defining
    // one column. The first string is one of the characters L, R or C
    // denoting left, right or center justification, resp.

    vector<string> header;
    YCPTerm headerterm = term->value(argnr)->asTerm();
    for (int i=0; i< headerterm->size(); i++)
    {
	YCPValue v = headerterm->value(i);
	string this_column;

	if (v->isString())
	    this_column = "L" + v->asString()->value(); // left justified per default

	else if (v->isTerm())
	{
	    YCPTerm t=v->asTerm();
	    if (t->size() != 1 ||
		!t->value(0)->isString())
	    {
		y2error("Invalid Table column specification %s",
			t->toString().c_str());
		return 0;
	    }

	    string s = t->symbol()->symbol();
	    string just;
	    if	 (s == "Left")	 just = "L";
	    else if (s == "Right")	 just = "R";
	    else if (s == "Center") just = "C";
	    else
	    {
		y2error("Invalid Table column specification %s",
			t->toString().c_str());
		return 0;
	    }
	    this_column = just + t->value(0)->asString()->value();
	}
	else
	{
	    y2error("Invalid header declaration in Table widget: %s",
		    headerterm->toString().c_str());
	    return 0;
	}
	header.push_back(this_column);
    }


    // Empty header not allowed!
    if (header.size() == 0)
    {
	y2error("empty header in Table widget not allowed");
	return 0;
    }


    YTable *table = dynamic_cast <YTable *> ( createTable(parent, opt, header) );
    assert(table);

    if (table && numargs == 2) // Fill table with items, if item list is specified
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	table->addItems(itemlist);
    }
    return table;
}



/**
 * @widget	ProgressBar
 * @short	Graphical progress indicator
 * @class	YProgressBar
 * @arg		string label the label describing the bar
 * @optarg	integer maxvalue the maximum value of the bar
 * @optarg	integer progress the current progress value of the bar
 * @usage	`ProgressBar(`id(`pb), "17 of 42 Packages installed", 42, 17)
 * @examples	ProgressBar1.ycp ProgressBar2.ycp
 *
 * @description
 *
 * A progress bar is a horizontal bar with a label that shows a progress
 * value. If you omit the optional parameter <tt>maxvalue</tt>, the maximum
 * value will be 100. If you omit the optional parameter <tt>progress</tt>, the
 * progress bar will set to 0 initially.
 */

YWidget *YUIInterpreter::createProgressBar(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int s = term->size() - argnr;
    if (s < 1
	|| s > 3
	|| (s >= 1 && !term->value(argnr)->isString())
	|| (s >= 2 && !term->value(argnr+1)->isInteger())
	|| (s >= 3 && !term->value(argnr+2)->isInteger()))
    {
	y2error("Invalid arguments for the ProgressBar widget: %s",
		term->toString().c_str());
	return 0;
    }

    YCPString  label	  = term->value(argnr)->asString();
    YCPInteger maxprogress(100);
    if (s >= 2) maxprogress = term->value(argnr+1)->asInteger();
    YCPInteger progress(0LL);
    if (s >= 3) progress = term->value(argnr+2)->asInteger();

    if (maxprogress->value() < 0
	|| progress->value() < 0
	|| progress->value() > maxprogress->value())
    {
	y2error("Invalid maxprogress/progress value for the ProgressBar widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    return createProgressBar(parent, opt, label, maxprogress, progress);
}



/**
 * @widget	Image
 * @short	Pixmap image
 * @class	YImage
 * @arg		symbol|byteblock|string image specification which image to display
 * @arg		string label label or default text of the image
 * @option	tiled tile pixmap: repeat it as often as needed to fill all available space
 * @option	scaleToFit scale the pixmap so it fits the available space: zoom in or out as needed
 * @option	zeroWidth make widget report a nice width of 0
 * @option	zeroHeight make widget report a nice height of 0
 * @option	animated image data contain an animated image (e.g. MNG)
 * @usage	`Image(`suseheader, "SuSE Linux 7.0")
 * @example	Image1.ycp Image-animated.ycp Image-local.ycp Image-scaled.ycp Image-tiled.ycp
 *
 * @description
 *
 * Displays an image - if the respective UI is capable of that. If not, it is
 * up to the UI to decide whether or not to display the specified default text
 * instead (e.g. with the NCurses UI).
 * <p>
 * The image is specified as any of:
 * <ul>
 * <li>symbol - load a predefined static image. Valid values are:
 *	<ul>
 *	<li><tt>`suseheader</tt> the SuSE standard header image
 *	<li><tt>`yast2</tt> the YaST2 logo
 *	</ul>
 * <li>byteblock - something you read with
 *     <tt>SCR::Read( .target.byte, "image1.png" )</tt>.
 *     This works on any configuration, even remote.
 * <li>string - a complete path name to an image in a supported format.
 *     This is the most convenient method (since you don't need that
 *     <tt>SCR::</tt> call mentioned above, but it has its limitations:
 *     It only works if the UI runs locally, i.e. has access to the local file
 *     system. This can <b>not</b> always be safely assumed.
 * </ul>
 * <p>
 * Use `opt(`zeroWidth) and / or `opt(`zeroHeight) if the real size of the
 * image widget is determined by outside factors, e.g. by the size of
 * neighboring widgets. With those options you can override the default "nice
 * size" of the image widget and make it show just a part of the image.
 * This is used for example in the YaST2 title graphics that are 2000 pixels
 * wide even when only 640 pixels are shown normally. If more screen space is
 * available, more of the image is shown, if not, the layout engine doesn't
 * complain about the image widget not getting its nice size.
 * <p>
 * `opt(`tiled) will make the image repeat endlessly in both dimensions to fill
 * up any available space. You might want to add `opt(`zeroWidth) or
 * `opt(`zeroHeight) (or both), too to make use of this feature.
 * <p>
 * `opt(`scaleToFit) scales the image to fit into the available space, i.e. the
 * image will be zoomed in or out as needed.
 * <p>
 * This option implicitly sets `opt(`zeroWidth) and `opt(zeroHeight),
 * too since there is no useful default size for such an image.
 * <p>
 * Please note that setting both `opt(`tiled) and `opt(`scaleToFit) at once
 * doesn't make any sense.
 */

YWidget *YUIInterpreter::createImage(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if ( term->size() - argnr != 2
	|| ( ! term->value( argnr )->isSymbol() &&
	     ! term->value( argnr )->isString() &&
	     ! term->value( argnr )->isByteblock() )
	|| ! term->value( argnr+1 )->isString())
    {
	y2error("Invalid arguments for the Image widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_zeroWidth	)  opt.zeroWidth.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_zeroHeight	)  opt.zeroHeight.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_animated	)  opt.animated.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_tiled	)  opt.tiled.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_scaleToFit	)  opt.scaleToFit.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if ( opt.scaleToFit.value() )
    {
	opt.zeroWidth.setValue( true );
	opt.zeroHeight.setValue( true );
    }

    if ( term->value(argnr)->isByteblock() )
	return createImage( parent, opt, term->value( argnr )->asByteblock(), term->value(argnr+1)->asString() );

    if ( term->value(argnr)->isString() )
	return createImage( parent, opt, term->value( argnr )->asString(), term->value(argnr+1)->asString() );

    ImageType img;
    string symbol = term->value(argnr)->asSymbol()->symbol();
    if	   (symbol == "suseheader") img = IT_SUSEHEADER;
    else if (symbol == "yast2")	    img = IT_YAST2;
    else
    {
	y2error("Unknown predefined image %s", symbol.c_str());
	return 0;
    }

    return createImage(parent, opt, img, term->value(argnr+1)->asString());
}



/*
 * @widget	IntField
 * @short	Numeric limited range input field
 * @class	YIntField
 * @arg		string	label		Explanatory label above the input field
 * @arg		integer minValue	minimum value
 * @arg		integer maxValue	maximum value
 * @arg		integer initialValue	initial value
 * @usage	`IntField("Percentage", 1, 100, 50)
 *
 * @examples	IntField1.ycp IntField2.ycp
 *
 * @description
 *
 * A numeric input field for integer numbers within a limited range.
 * This can be considered a lightweight version of the
 * <a href="YSlider-widget.html">Slider</a> widget, even as a replacement for
 * this when the specific UI doesn't support the Slider.
 * Remember it always makes sense to specify limits for numeric input, even if
 * those limits are very large (e.g. +/- MAXINT).
 * <p>
 * Fractional numbers are currently not supported.
 */

YWidget *YUIInterpreter::createIntField(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 4
	 || ! term->value(argnr)->isString()
	 || ! term->value(argnr+1)->isInteger()
	 || ! term->value(argnr+2)->isInteger()
	 || ! term->value(argnr+3)->isInteger()
	 )
    {
	y2error( "Invalid arguments for the IntField widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);

    YCPString label	= term->value(argnr)->asString();
    int minValue	= term->value(argnr+1)->asInteger()->value();
    int maxValue	= term->value(argnr+2)->asInteger()->value();
    int initialValue	= term->value(argnr+3)->asInteger()->value();

    return createIntField(parent, opt, label, minValue, maxValue, initialValue);
}



/*
 * @widget	PackageSelector
 * @short	Complete software package selection
 * @class	YPackageSelector
 * @option	youMode start in YOU (YaST Online Update) mode
 * @option	updateMode start in update Mode
 * @usage	`PackageSelector()
 *
 * @examples	PackageSelector1.ycp
 *
 * @description
 *
 * A very complex widget that handles software package selection completely
 * transparently. Set up the package manager (the backend) before creating this
 * widget and let the package manager and the package selector handle all the
 * rest. The result of all this are the data stored in the package manager.
 * <p>
 * <b>Notice:</b> <i>(preliminary)</i>
 * UserInput() will only return after the user is done selecting packages.
 */

YWidget *YUIInterpreter::createPackageSelector(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					       const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 0 )
    {
	y2error( "Invalid arguments for the PackageSelector widget: %s",
		 term->toString().c_str() );
	return 0;
    }


    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() )
	{
	    std::string sym = optList->value(o)->asSymbol()->symbol();

	    if      ( sym == YUIOpt_youMode    ) opt.youMode.setValue( true );
	    else if ( sym == YUIOpt_updateMode ) opt.updateMode.setValue( true );
	    else logUnknownOption(term, optList->value(o));
	}
	else logUnknownOption(term, optList->value(o));
    }

    return createPackageSelector(parent, opt);
}




/*
 * @widget	PkgSpecial
 * @short	Package selection special - DON'T USE IT
 * @class	YEmpty
 * @usage	`PkgSpecial( "subwidget_name" )
 *
 * @description
 *
 * Use only if you know what you are doing - that is, DON'T USE IT.
 */

YWidget *YUIInterpreter::createPkgSpecial(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					       const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 1
	 || ! term->value( argnr )->isString() )
    {
	y2error( "Invalid arguments for the PkgSpecial widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);
    YCPString subwidget = term->value( argnr )->asString();

    return createPkgSpecial( parent, opt, subwidget );
}



// =============================================================================



YWidget *YUIInterpreter::widgetWithId(const YCPValue &id, bool log_error)
{
    if (currentDialog())
    {
	YWidget *widget = currentDialog()->findWidget(id);
	if (widget) return widget;
	if (log_error)
	    y2error("No widget with `" YUISymbol_id "(%s)", id->toString().c_str() );
    }
    else if (log_error)
	y2error("No dialog existing, therefore no widget with `" YUISymbol_id "(%s)",
		id->toString().c_str());

    return 0;
}

YWidget *YUIInterpreter::widgetWithId(YContainerWidget *widgetRoot, const YCPValue &id, bool log_error)
{
    if (widgetRoot)
    {
	YWidget *widget = widgetRoot->findWidget(id);
	if (widget) return widget;
	if (log_error)
	    y2error("No widget with `" YUISymbol_id "(%s)", id->toString().c_str() );
    }
    else if (log_error)
	y2error("No dialog existing, therefore no widget with `" YUISymbol_id "(%s)",
		id->toString().c_str());

    return 0;
}

bool YUIInterpreter::checkId(const YCPValue& v, bool complain) const
{
    if (v->isTerm()
	&& v->asTerm()->size() == 1
	&& v->asTerm()->symbol()->symbol() == YUISymbol_id) return true;
    else
    {
	if ( complain )
	{
	    y2error("Expected `" YUISymbol_id "(any v), you gave me %s", v->toString().c_str());
	}
	return false;
    }
}


YCPValue YUIInterpreter::getId(const YCPValue& v) const
{
    return v->asTerm()->value(0);
}


bool YUIInterpreter::parseRgb(const YCPValue & val, YColor *color, bool complain)
{
    if ( ! color )
    {
	y2error( "Null pointer for color" );
	return false;
    }

    bool ok = val->isTerm();

    if ( ok )
    {
	YCPTerm term = val->asTerm();

	ok = term->size() == 3 && term->symbol()->symbol() == YUISymbol_rgb;

	if ( ok )
	{
	    ok =   term->value(0)->isInteger()
		&& term->value(1)->isInteger()
		&& term->value(2)->isInteger();
	}

	if ( ok )
	{
	    color->red   = term->value(0)->asInteger()->value();
	    color->green = term->value(1)->asInteger()->value();
	    color->blue  = term->value(2)->asInteger()->value();
	}

	if ( ok )
	{
	    if ( color->red      < 0 || color->red   > 255
		 || color->green < 0 || color->green > 255
		 || color->blue  < 0 || color->blue  > 255 )
	    {
		y2error( "RGB value out of range! (0..255)" );
		return false;
	    }
	}
    }


    if ( ! ok && complain )
    {
	y2error("Expected `" YUISymbol_rgb "(integer red, blue, green), you gave me %s",
		val->toString().c_str());
    }

    return ok;
}


YCPValue YUIInterpreter::getWidgetId(const YCPTerm &term, int *argnr)
{
    if (term->size() > 0
	&& term->value(0)->isTerm()
	&& term->value(0)->asTerm()->symbol()->symbol() == YUISymbol_id)
    {
	YCPTerm idterm = term->value(0)->asTerm();
	if (idterm->size() != 1)
	{
	    y2error("Widget id `" YUISymbol_id "() must have exactly one argument. You gave %s",
		    idterm->toString().c_str());
	    return YCPNull();
	}

	YCPValue id = idterm->value(0);
	// unique?
	if (widgetWithId(id))
	{
	    y2error("Widget id %s is not unique", id->toString().c_str());
	    return YCPNull();
	}

	*argnr = 1;
	return id;
    }
    else
    {
	*argnr = 0;
	return YCPVoid();	// no `id() specified -> use "nil"
    }
}


YCPList YUIInterpreter::getWidgetOptions(const YCPTerm &term, int *argnr)
{
    if (term->size() > *argnr
	&& term->value(*argnr)->isTerm()
	&& term->value(*argnr)->asTerm()->symbol()->symbol() == YUISymbol_opt)
    {
	YCPTerm optterm = term->value(*argnr)->asTerm();
	*argnr = *argnr + 1;
	return optterm->args();
    }
    else return YCPList();
}


void YUIInterpreter::logUnknownOption(const YCPTerm &term, const YCPValue &option)
{
    y2warning("Unknown option %s in %s widget",
	      option->toString().c_str(), term->symbol()->symbol().c_str());
}


void YUIInterpreter::rejectAllOptions(const YCPTerm &term, const YCPList &optList)
{
    for (int o=0; o < optList->size(); o++)
    {
	logUnknownOption(term, optList->value(o));
    }
}



// EOF
