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

  File:	      YTable.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPInteger.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YTable.h"


YTable::YTable( YWidgetOpt & opt, int num_cols )
    : YWidget( opt )
    , num_cols( num_cols )
{
    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YCPValue YTable::changeWidget( const YCPSymbol & property,
			      const YCPValue & newValue )
{
    string s = property->symbol();

    /*
     * @property integer CurrentItem the ID of the currently selected item
     */
    if ( s == YUIProperty_CurrentItem )  // Sort for item with that id
    {
	int index = itemWithId( newValue, true ); // true: log error
	if ( index < 0 ) return YCPBoolean( false );
	else
	{
	    setCurrentItem( index );
	    return YCPBoolean( true );
	}
    }

    /*
     * @property list( item ) Items a list of all table items
     */
    else if ( s == YUIProperty_Items ) // Change whole selection box!
    {
	if ( !newValue->isList() )
	{
	    y2warning( "Items property of Table widget must be a list" );
	    return YCPBoolean( false );
	}
	OptimizeChanges below( *this ); // delay screen updates until this block is left

	itemsCleared();
	rows.clear();
	return YCPBoolean( addItems( newValue->asList() ) );
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YTable::changeWidget( const YCPTerm & property, const YCPValue & newvalue )
{
    string s = property->symbol()->symbol();
    /*
     * @property item		Item( id )	read: a single item ( string or term )
     * @property integer|string	Item( id,column ) write: replacement for one specific cell ( see example )
     */
    if ( s == YUIProperty_Item )
    {
	if ( property->size() != 2 || !property->value(1)->isInteger() )
	{
	    y2error( "Table %s: property `Item() needs two arguments: item id and column number",
		    id()->toString().c_str() );
	    return YCPBoolean( false );
	}
	YCPValue itemid = property->value(0);
	int item_nr = itemWithId( itemid, true );
	if ( item_nr < 0 ) return YCPBoolean( false );

	int colnum = ( int)( property->value(1)->asInteger()->value() );
	if ( colnum >= numCols() || colnum < 0 )
	{
	    y2error( "Table %s: Invalid column number %d",
		    id()->toString().c_str(), colnum );
	    return YCPBoolean( false );
	}

	string newtext;
	if ( newvalue->isString() ) newtext = newvalue->asString()->value();
	else if ( newvalue->isInteger() ) newtext = newvalue->asInteger()->toString();
	else
	{
	    y2error( "Table %s: Invalid value for cell ( %s|%d ). Must be string or integer",
		    id()->toString().c_str(), itemid->toString().c_str(), colnum );
	    return YCPBoolean( false );
	}

	rows[ item_nr].elements[colnum ] = newtext;
	cellChanged( item_nr, colnum, YCPString( newtext ) );
	return YCPBoolean( true );
    }
    else return YWidget::changeWidget( property, newvalue );
}


YCPValue YTable::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_CurrentItem )
    {
	int index = getCurrentItem();
	y2debug( "current item: %d", index );
	if ( index >= 0 ) return rows[ index ].id;
	else	      return YCPVoid();
    }
    else return YWidget::queryWidget( property );
}


YCPValue YTable::queryWidget( const YCPTerm & property )
{
    string s = property->symbol()->symbol();
    if ( s == YUIProperty_Item )
    {
	if ( property->size() != 1 )
	{
	    y2error( "Table %s: property `Item() needs one argument",
		    id()->toString().c_str() );
	    return YCPVoid();
	}
	int item_nr = itemWithId( property->value(0), true );
	if ( item_nr < 0 ) return YCPVoid();
	else	       return rows[ item_nr ].makeTerm();
    }
    else return YWidget::queryWidget( property );
}


/*
 * Insert a number of items.
 * Reimplemented in YQTable to keep QListView happy.
 */
bool YTable::addItems( const YCPList & itemlist )
{
    for ( int i = 0; i < itemlist->size(); i++ )
    {
	YCPValue item = itemlist->value(i);

	if ( ! addItem( item ) )
	    return false;	// Error
    }

    return true;		// Success
}


bool YTable::addItem( const YCPValue & item )
{
    if ( !item->isTerm() || item->asTerm()->symbol()->symbol() != YUISymbol_item )
    {
	y2error( "Invalid item specification %s: Must be `" YUISymbol_item "() term",
		  item->toString().c_str() );
	return false;
    }

    YCPList collist = item->asTerm()->args();
    if ( collist->size() != numCols() + 1 )
    {
	y2error( "Invalid item specification %s: Wrong number of elements",
		  item->toString().c_str() );
	return false;
    }

    if ( !collist->value(0)->isTerm()
	|| collist->value(0)->asTerm()->symbol()->symbol() != YUISymbol_id
	|| collist->value(0)->asTerm()->size() != 1)
    {
	y2error( "Invalid item specification %s: Must begin with `" YUISymbol_id "() term",
		  item->toString().c_str() );
	return false;
    }

    YCPValue id = collist->value(0)->asTerm()->value(0);

    // Prepare stl vector of strings of the content
    vector<string> row;
    for ( int r=1; r< collist->size(); r++ )
    {
	YCPValue value = collist->value(r);
	if      ( value->isString() ) row.push_back( value->asString()->value() );
	else if ( value->isInteger() ) row.push_back( value->toString() );
	else if ( value->isVoid() ) row.push_back( "" );
	else
	{
	    y2error( "Invalid value %s in item. Must be string, integer or void",
		      value->toString().c_str() );
	    row.push_back( "" );
	}
    }
    addItem( id, row );
    return true;
}



void YTable::addItem( const YCPValue & id, vector<string> elements )
{
    rows.push_back( YTableRow( id, elements ) );
    itemAdded( elements, rows.size() - 1 );
}


void YTable::itemAdded( vector<string>, int )
{
    // default dummy implementaion
}


int YTable::numItems() const
{
    return rows.size();
}


int YTable::itemWithId( const YCPValue & id, bool report_error )
{
    for ( int i=0; i<numItems(); i++ )
    {
	if      ( rows[i].id->equal( id ) ) return i;
    }
    if ( report_error )
	y2error( "Table: No item %s existing", id->toString().c_str() );

    return -1;
}

int YTable::numCols() const
{
    return num_cols;
}


YCPTerm YTableRow::makeTerm() const
{
    // Return the item as term
    YCPTerm itemterm( YUISymbol_item, true );
    YCPTerm idterm( YUISymbol_id, true );
    idterm->add( id );
    itemterm->add( idterm );
    for ( unsigned int c=0; c < elements.size(); c++ )
	itemterm->add( YCPString( elements[c] ) );
    return itemterm;
}


void YTable::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_CurrentItem );
}
