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

  File:	      YMultiSelectionBox.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YMultiSelectionBox.h"


YMultiSelectionBox::YMultiSelectionBox( YWidgetOpt & opt, YCPString label )
    : YWidget( opt )
    , label( label )
{
    // y2debug( "YMultiSelectionBox( %s )", label->value_cstr() );

    // Derived classes need to check opt.shrinkable!

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );

}


YCPValue YMultiSelectionBox::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string sym = property->symbol();

    /**
     * @property string Label
     * The label above the list describing what it is all about
     */
    if ( sym == YUIProperty_Label )
    {
	if ( newValue->isString() )
	{
	    setLabel( newValue->asString() );

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "MultiSelectionBox: Invalid parameter %s for Label property. Must be string",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }

    /**
     * @property string CurrentItem
     * The item that currently has the keyboard focus
     */
    else if ( sym == YUIProperty_CurrentItem )	 // Select item with that id
    {
	int index = itemWithId( newValue, true ); // true: log error
	if ( index < 0 ) return YCPBoolean( false );
	else
	{
	    setCurrentItem( index );
	    return YCPBoolean( true );
	}
    }
    /**
     * @property id_list SelectedItems
     * The items that are currently selected
     */
    else if ( sym == YUIProperty_SelectedItems )
    {
	if ( ! newValue->isList() )
	{
	    y2error( "MultiSelectionBox: Can't set property %s: "
		     "Expected list of IDs or item labels, not %s",
		     sym.c_str(), newValue->toString().c_str() );

	    return YCPBoolean( false );
	}

	OptimizeChanges below( *this ); // delay screen updates until this block is left

	deselectAllItems();
	YCPList selected_items = newValue->asList();

	for ( int i = 0; i < selected_items->size(); i++ )
	{
	    YCPValue id = selected_items->value(i);
	    int index	= itemWithId( id, true ); // true: log error

	    if ( index < 0 )			// No such item
	    {
		return YCPBoolean( false );
	    }

	    selectItem( index );
	}

	return YCPBoolean( false );
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YMultiSelectionBox::queryWidget( const YCPSymbol & property )
{
    string sym = property->symbol();
    if      ( sym == YUIProperty_Label       ) return getLabel();
    else if ( sym == YUIProperty_CurrentItem )
    {
	int index = getCurrentItem();
	// y2debug( "current item: %d", index );

	if ( index >= 0 )
	{
	    if ( item_ids->value( index ).isNull() ) return item_labels->value( index );
	    else return item_ids->value( index );
	}
	else return YCPVoid();
    }
    else if ( sym == YUIProperty_SelectedItems )
    {
	YCPList selected_items;

	for ( int i = 0; i < numItems(); i++ )
	{
	    if ( itemIsSelected(i) )	// ask specific UI for selection state
	    {
		selected_items->add( item_ids->value(i).isNull() ?
				     item_labels->value(i) : item_ids->value(i) );
	    }
	}

	return selected_items;
    }
    else return YWidget::queryWidget( property );
}


void YMultiSelectionBox::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YMultiSelectionBox::getLabel()
{
    return label;
}


void YMultiSelectionBox::addItem( const YCPString &	text,
				  const YCPValue  &	id,
				  bool 			selected )
{
    // y2debug( "Adding item '%s'", text->value().c_str() );

    item_labels->add( text );
    item_ids->add( id );
    itemAdded( text, selected );
}


int YMultiSelectionBox::numItems() const
{
    return item_ids->size();
}


void YMultiSelectionBox::deleteAllItems()
{
    item_labels = YCPList();
    item_ids	= YCPList();
}


int YMultiSelectionBox::itemWithId( const YCPValue & id, bool report_error )
{
    for ( int i=0; i < numItems(); i++ )
    {
	if ( ! item_ids->value(i).isNull() && item_ids->value(i)->equal( id ) ) return i;
	else if ( item_labels->value(i)->equal( id ) ) return i;
    }
    if ( report_error )
	y2error( "MultiSelectionBox: No item %s existing", id->toString().c_str() );

    return -1;
}


void YMultiSelectionBox::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_CurrentItem   );
    macroRecorder->recordWidgetProperty( this, YUIProperty_SelectedItems );
}

