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

  File:	      YWidget.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YShortcut.h"
#include "YWidget.h"
#include "YUIComponent.h"
#include "YUI.h"
#include "YDialog.h"

using std::string;

#define MAX_DEBUG_LABEL_LEN	50



int YWidget::next_internal_widget_id = 0;


YWidget::YWidget( const YWidgetOpt & opt )
    : magic( YWIDGET_MAGIC )
    , user_widget_id( YCPNull() )
    , yparent(0)
    , rep(0)
    , windowID( -1 )
{
    internal_widget_id = next_internal_widget_id++;

    enabled			= ! opt.isDisabled.value();
    notify			= opt.notifyMode.value();
    _sendKeyEvents		= opt.keyEvents.value();
    _autoShortcut		= opt.autoShortcut.value();
    _stretch[ YD_HORIZ ]	= opt.isHStretchable.value();
    _stretch[ YD_VERT  ]	= opt.isVStretchable.value();
    _weight[ YD_HORIZ ]		= opt.hWeight.value();
    _weight[ YD_VERT  ]		= opt.vWeight.value();

    if ( ! enabled )	setEnabling( false );
    if ( notify )	setNotify( notify );
}


YWidget::~YWidget()
{
    if ( ! isValid() )
    {
	y2error( "ERROR: Trying to destroy invalid widget" );
	return;
    }

    if ( yparent && yparent->isValid() )
	yparent->childDeleted( this );

    invalidate();
}


string YWidget::debugLabel()
{
    string label = YShortcut::cleanShortcutString( YShortcut::getShortcutString( this ) );

    if ( label.size() > MAX_DEBUG_LABEL_LEN )
    {
	label.resize( MAX_DEBUG_LABEL_LEN );
	label.append( "..." );
    }

    return label;
}


void YWidget::setId( const YCPValue & id )
{
    user_widget_id = id;
}


bool YWidget::hasId() const
{
    return ( ! user_widget_id.isNull() && ! user_widget_id->isVoid() );
}


YCPValue YWidget::id() const
{
    if ( ! isValid() )
    {
	y2error( "YWidget::id(): Invalid widget" );
	return YCPString( "<invalid widget>" );
    }

    if ( user_widget_id.isNull() )
	return YCPVoid();
    else
	return user_widget_id;
}


void YWidget::setParent( YWidget *parent )
{
    yparent = parent;
}


YWidget * YWidget::yParent() const
{
    return yparent;
}


YWidget * YWidget::yDialog()
{
    YWidget *parent = this;

    while ( parent && ! parent->isDialog() )
    {
	parent = parent->yParent();
    }

    if ( ! parent )
    {
	y2warning( "Warning: No dialog parent for %s", widgetClass() );
    }

    return parent;
}


YCPValue YWidget::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string symbol = property->symbol();

    if ( ! isValid() )
    {
	y2error( "YWidget::changeWidget( %s ): ERROR: Invalid widget", symbol.c_str() );

	return YCPBoolean( false );	// Error
    }

    /*
     * @property boolean Enabled the current enabled/disabled state
     */
    if ( symbol == YUIProperty_Enabled )
    {
	if ( newvalue->isBoolean() )
	{
	    bool e = newvalue->asBoolean()->value();
	    setEnabling(e);
	    enabled = e;
	    return YCPBoolean( true );
	}
	else y2error( "Wrong argument %s for widget property `Enabled - boolean expected.",
		      newvalue->toString().c_str() );
    }

    /*
     * @property boolean Notify the current notify state (see also `opt( `notify ))
     */
    if ( symbol == YUIProperty_Notify )
    {
	if ( newvalue->isBoolean() )
	{
	    setNotify( newvalue->asBoolean()->value() );
	    return YCPBoolean( true );
	}
	else y2error( "Wrong argument %s for widget property `Notify - boolean expected.",
		      newvalue->toString().c_str() );
    }

    return YCPBoolean( false );
}


YCPValue YWidget::changeWidgetTerm( const YCPTerm & property, const YCPValue & newvalue )
{
    y2warning( "Widget %s: Couldn't change unknown widget property %s to %s",
	       id()->toString().c_str(), property->toString().c_str(), newvalue->toString().c_str() );
    return YCPVoid();
}


YCPValue YWidget::queryWidget( const YCPSymbol & property )
{
    string symbol = property->symbol();
    if ( symbol == YUIProperty_Enabled 		) 	return YCPBoolean( getEnabling() );
    if ( symbol == YUIProperty_Notify 		)	return YCPBoolean( getNotify()   );
    if ( symbol == YUIProperty_WindowID		) 	return YCPInteger( windowID      );
    /**
     * @property string WidgetClass the widget class of this widget (YLabel, YPushButton, ...)
     */ 
    if ( symbol == YUIProperty_WidgetClass 	) 	return YCPString( widgetClass() );
    /**
     * @property string DebugLabel a (possibly translated) text describing this widget for debugging
     */ 
    if ( symbol == YUIProperty_DebugLabel	) 	return YCPString( debugLabel() );
    /**
     * @property string DialogDebugLabel 	a (possibly translated) text describing this dialog for debugging
     */ 
    if ( symbol == YUIProperty_DialogDebugLabel ) 
    {
	return YUIComponent::ui()->currentDialog()->queryWidget( property );
    }
    else
    {
	y2error( "Widget %s: Couldn't query unkown widget property %s",
		 id()->toString().c_str(), symbol.c_str() );
	return YCPVoid();
    }
}


YCPValue YWidget::queryWidgetTerm( const YCPTerm & property )
{
    y2warning( "Widget %s: Couldn't query unkown widget property %s",
	       id()->toString().c_str(), property->toString().c_str() );
    return YCPVoid();
}



void YWidget::setNotify( bool notify )
{
    this->notify = notify;
}


bool YWidget::getNotify() const
{
    return notify;
}


bool YWidget::getEnabling() const
{
    return enabled;
}


long YWidget::nicesize( YUIDimension dim )
{
    y2error( "YWidget::nicesize( YUIDimension dim ) called - "
	     "this method should be overwritten in derived classes!" );
    return 1;
}


void YWidget::setStretchable( YUIDimension dim, bool newStretch )
{
    _stretch[ dim ] = newStretch;
}


void YWidget::setDefaultStretchable( YUIDimension dim, bool newStretch )
{
    _stretch[ dim ] |= newStretch;
}


bool YWidget::stretchable( YUIDimension dim ) const
{
    return _stretch[ dim ];
}


long YWidget::weight( YUIDimension dim )
{
    return _weight[ dim ];
}


bool YWidget::hasWeight( YUIDimension dim )
{
    // DO NOT simply return _weight[ dim ] here
    // since weight() might be overwritten in derived classes!

    return weight( dim ) > 0;
}

void YWidget::setSize( long newwidth, long newheight )
{
}


void YWidget::setEnabling( bool )
{
    // Default implementation for widgets that can't be enabled
    // or disabled
}


void *YWidget::widgetRep()
{
    return rep;
}


void YWidget::setWidgetRep( void *r )
{
    rep = r;
}


bool YWidget::isDialog() const
{
    return false;
}


bool YWidget::isContainer() const
{
    return false;
}


bool YWidget::isRadioButtonGroup() const
{
    return false;
}

bool YWidget::isReplacePoint() const
{
    return false;
}


bool YWidget::isLayoutStretch( YUIDimension dim ) const
{
    return false;
}

bool YWidget::setKeyboardFocus()
{
    y2warning( "Widget %s cannot accept the keyboard focus.", id()->toString().c_str() );
    return false;
}


void YWidget::saveUserInput( YMacroRecorder *macroRecorder )
{
    /*
     * This default implementation does nothing. Overwrite this method in any
     * derived class that has user input to save, e.g. TextEntry etc. and call
     * YMacroRecorder::recordWidgetProperty() in the overwritten method.
     */
}

