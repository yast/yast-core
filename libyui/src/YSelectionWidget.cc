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

  File:	      YSelectionWidget.cc

  Author:     Holger Macht <hmacht@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YSelectionBox.h"
#include "YUI.h"


YSelectionWidget::YSelectionWidget( const YWidgetOpt & opt, YCPString label )
    : YWidget( opt )
    , label( label )
{
    // y2debug( "YSelectionWidget( %s )", label->value_cstr() );
}


void YSelectionWidget::setLabel( const YCPString & label )
{
    this->label = label;
}

YCPString YSelectionWidget::getLabel()
{
    return label;
}


void YSelectionWidget::itemAdded( const YCPString & , int index, bool selected)
{
    // default dummy implementation
}


int YSelectionWidget::numItems() const
{
    return item_ids->size();
}


int YSelectionWidget::itemWithId( const YCPValue & id, bool report_error )
{
    for ( int i=0; i < numItems(); i++ )
    {
	if ( ! item_ids->value(i).isNull() && item_ids->value(i)->equal( id ) ) return i;
	else if ( item_labels->value(i)->equal( id ) ) return i;
    }
    if ( report_error )
	y2error( "SelectionWidget: No item %s existing", id->toString().c_str() );

    return -1;
}


int YSelectionWidget::parseItems( const YCPList & itemlist )
{
    for ( int i=0; i<itemlist->size(); i++ )
    {
	YCPValue item = itemlist->value(i);
	    
	if ( item->isString() ) // item is just a label
	{
	    addItem( YCPNull(), item->asString(), false );
	}
	else if ( item->isTerm() // item is a term (maybe with id)
		  && item->asTerm()->name() == YUISymbol_item )
	{
	    YCPTerm iterm = item->asTerm();
	    // there must be an id, a label for the item and a optional third bool
	    // parameter (selected yes?no)
	    if ( iterm->size() < 1 || iterm->size() > 3 )
	    {
		y2error( "%s: Invalid argument number in %s",
			 widgetClass(), iterm->toString().c_str() );
	    }
	    else
	    {
		int argnr = YUI::checkId( iterm->value(0) ) ? 1 : 0;
		if ( iterm->size() <= argnr || ! iterm->value( argnr )->isString() )
		    y2error( "%s: Invalid item arguments in %s",
			     widgetClass(), iterm->toString().c_str() );
		else
		{
		    YCPValue item_id = YCPNull();
		    if ( argnr == 1 ) 
		    {
			item_id = YUI::getId( iterm->value(0) );
		    }
			
		    YCPString item_label = iterm->value( argnr )->asString();
			
		    bool item_selected = false;
			
		    if ( iterm->size() >= argnr + 2 )
		    {
			if ( iterm->value( argnr+1 )->isBoolean() )
			    item_selected = iterm->value( argnr+1 )->asBoolean()->value();
			else
			{
			    y2error( "%s: Invalid item arguments in %s",
				     widgetClass(), iterm->toString().c_str() );
			}
		    }
		    // add one item to the widget
		    addItem( item_id, item_label, item_selected );
		}
	    }
	}
	else
	{
	    y2error( "Invalid item %s: %s items must be strings or specified with `"
		     YUISymbol_item "()", widgetClass(), item->toString().c_str() );
	}
    } // loop over items
    
    return 1;
}


void YSelectionWidget::addItem( const YCPValue & id, const YCPString & text, bool selected )
{
    item_ids->add( id );
    item_labels->add( text );
    itemAdded( text, numItems()-1, selected );

}


void YSelectionWidget::deleteAllItems()
{
    item_labels = YCPList();
    item_ids	= YCPList();
}



YCPValue YSelectionWidget::changeLabel( const YCPValue & newValue )
{
    if ( newValue->isString() )
    {
	setLabel( newValue->asString() );
	
	return YCPBoolean( true );
    }
    else
    {
	y2error( "%s: Invalid parameter %s for Label property. Must be string",
		 widgetClass(), newValue->toString().c_str() );
	
	return YCPBoolean( false );
    }
}


YCPValue YSelectionWidget::changeItems ( const YCPValue & newValue ) 
{
    if ( newValue->isList() )
    {
	deleteAllItems();
	YCPList itemlist = newValue->asList();
	if ( ! parseItems( itemlist ) ) 
	{
	    y2error ("%s: Failed to parse itemlist while changing items",
		     widgetClass() );
	    return YCPBoolean( false );
	}
    }
    
    return YCPBoolean( true );
}
