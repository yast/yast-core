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

  File:	      YSelectionBox.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YSelectionBox.h"


YSelectionBox::YSelectionBox( YWidgetOpt &opt, YCPString label)
    : YWidget( opt)
    , label( label)
{
    y2debug( "YSelectionBox(%s)", label->value_cstr( ));

    // Derived classes need to check opt.shrinkable!

    setDefaultStretchable( YD_HORIZ, true);
    setDefaultStretchable( YD_VERT,  true);

}


YCPValue YSelectionBox::changeWidget( const YCPSymbol & property, const YCPValue & newValue)
{
    string s = property->symbol( );

    /**
     * @property string Label
     * The label above the list describing what it is all about
     */
    if ( s == YUIProperty_Label)
    {
	if ( newValue->isString())
	{
	    setLabel( newValue->asString());
	    return YCPBoolean( true);
	}
	else
	{
	    y2error( "SelectionBox: Invalid parameter %s for Label property. Must be string",
		    newValue->toString( ).c_str());
	    return YCPBoolean( false);
	}
    }

    /**
     * @property string CurrentItem
     * The currently selected item or its ID, if it has one.
     */
    else if ( s == YUIProperty_CurrentItem)	 // Select item with that id
    {
	int index = itemWithId( newValue, true); // true: log error
	if ( index < 0) return YCPBoolean( false);
	else
	{
	    setCurrentItem( index);
	    return YCPBoolean( true);
	}
    }
    else return YWidget::changeWidget( property, newValue);
}



YCPValue YSelectionBox::queryWidget( const YCPSymbol & property)
{
    string s = property->symbol( );
    if      ( s == YUIProperty_Label      ) return getLabel( );
    else if ( s == YUIProperty_CurrentItem)
    {
	int index = getCurrentItem( );
	y2debug( "current item: %d", index);
	if ( index >= 0)
	{
	    if ( item_ids->value(index).isNull()) return item_labels->value( index);
	    else return item_ids->value( index);
	}
	else return YCPVoid( );
    }
    else return YWidget::queryWidget( property);
}


void YSelectionBox::setLabel( const YCPString &label)
{
    this->label = label;
}


YCPString YSelectionBox::getLabel( )
{
    return label;
}


void YSelectionBox::addItem( const YCPValue &id, const YCPString & text, bool selected)
{
    item_ids->add( id);
    item_labels->add( text);
    itemAdded( text, numItems( )-1, selected);
}


void YSelectionBox::itemAdded( const YCPString &, int, bool)
{
    // default dummy implementation
}


int YSelectionBox::numItems( ) const
{
    return item_ids->size( );
}


int YSelectionBox::itemWithId( const YCPValue &id, bool report_error)
{
    for ( int i=0; i < numItems( ); i++ )
    {
	if ( ! item_ids->value( i).isNull() && item_ids->value( i)->equal(id) ) return i;
	else if ( item_labels->value(i)->equal(id)) return i;
    }
    if ( report_error)
	y2error( "SelectionBox: No item %s existing", id->toString( ).c_str());

    return -1;
}


void YSelectionBox::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_CurrentItem );
}

