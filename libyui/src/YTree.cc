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

  File:	      YTree.cc

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPMap.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YTree.h"
#include "YUI.h"


YTree::YTree( const YWidgetOpt & opt, YCPString newLabel )
    : YWidget( opt )
    , label( newLabel )
{
    // y2debug( "YTree( %s )", newLabel->value_cstr() );
    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YTree::~YTree()
{
    // Get rid of all children.
    //
    // Unfortuanately, STL iterators are doo dumb to modify their
    // container - so we really need to do this manually.

    for ( unsigned i=0; i < items.size(); i++ )
    {
	delete items[i];
    }

    items.clear();
}


YCPValue
YTree::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();

    /*
     * @property string Label the label above the Tree
     */
    if ( s == YUIProperty_Label )
    {
	if ( newvalue->isString() )
	{
	    setLabel( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "Tree: Invalid parameter %s for Label property - must be string.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }

    /*
     * @property itemId CurrentItem the currently selected item
     */
    else if ( s == YUIProperty_CurrentItem )
    {
	YTreeItem *it = findItemWithId ( newvalue );

	if ( ! it )
	    it = findItemWithText ( newvalue->asString() );

	if ( it )
	{
	    setCurrentItem ( it );
	    return YCPBoolean( true );

	}
	else // no such item found
	{
	    y2error( "Tree: No item %s existing", newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }

    /**
     * @property YTreeItem list Items
     * The items that are displayed
     */
    else if ( s == YUIProperty_Items )
    {
	if ( newvalue->isList() )
	{
	    deleteAllItems();
	    YCPList itemlist = newvalue->asList();
	    if ( ! parseItemList( itemlist ) )
	    {
		y2error ("%s: Error parsing item list", widgetClass() );
		return YCPBoolean( false );
	    }
	    rebuildTree();
	}
	return YCPBoolean( true );
    }
    else
    {
	return YWidget::changeWidget( property, newvalue );
    }
}



YCPValue YTree::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();

    if ( s == YUIProperty_Label )
    {
	return getLabel();
    }
    else if ( s == YUIProperty_CurrentItem )
    {
	const YTreeItem *it = getCurrentItem();

	if ( it )
	    return it->getId().isNull() ? it->getText() : it->getId();
	else
	    return YCPVoid();
    }
    else if ( s == YUIProperty_Items )
    {
	return itemsTermList( items );
    }
    /*
     * @property OpenItems a map of open items - can only be queried, not set
     */
    else if ( s == YUIProperty_OpenItems )
    {
	YCPMap openItems;
	findOpenItems( openItems, items );

	return openItems;
    }
    else
	return YWidget::queryWidget( property );

    return YCPVoid();
}


void
YTree::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YTree::getLabel()
{
    return label;
}


YTreeItem *
YTree::addItem( YTreeItem *		parentItem,
		const YCPValue &	id,
		const YCPString &	text,
		bool			open )
{
    YTreeItem *treeItem;

    if ( parentItem )
	treeItem = new YTreeItem ( parentItem, id, text, open );
    else
	treeItem = new YTreeItem ( this, id, text, open );

    return treeItem;
}


YTreeItem *
YTree::addItem( YTreeItem *		parentItem,
		const YCPString &	text,
		void *			data,
		bool			open )
{
    YTreeItem *treeItem;

    if ( parentItem )
	treeItem = new YTreeItem ( parentItem, text, data, open );
    else
	treeItem = new YTreeItem ( this, text, data, open );

    return treeItem;
}


void
YTree::rebuildTree()
{
    // NOP
}


YCPList YTree::itemsTermList( YTreeItemList items )
{
    YCPList list;

    for ( YTreeItemListIterator it = items.begin();
	  it != items.end();
	  ++it )
    {
	YTreeItem * item = *it;
	YCPTerm itemTerm( YUISymbol_item );	// `item()
	YCPValue id = item->getId();

	if ( ! id.isNull() )
	{
	    YCPTerm idTerm( YUISymbol_id );	// `id()
	    idTerm->add( id );			// `id(`something )
	    itemTerm->add( idTerm );		// `item(`id(`something ) )
	}

	YCPString itemText = item->getText();
	itemTerm->add( itemText );

	if ( item->isOpen() )
	{
	    // y2milestone( "Open item: %s", itemText->value().c_str() );
	    itemTerm->add( YCPBoolean( true ) );
	}

	YCPList childrenList = itemsTermList( item->itemList() );

	if ( childrenList->size() > 0 )
	    itemTerm->add( childrenList );

	list->add( itemTerm );
    }

    return list;
}



void YTree::findOpenItems( YCPMap & openItems, YTreeItemList items )
{
    for ( YTreeItemListIterator it = items.begin();
	  it != items.end();
	  ++it )
    {
	YTreeItem * item = *it;

	if ( item->isOpen() )
	{
	    YCPValue id = item->getId();

	    if ( id.isNull() )
		openItems->add( item->getText(), YCPString( "Text" ) );
	    else
		openItems->add( id, YCPString( "ID" ) );

	    findOpenItems( openItems, item->itemList() );
	}
    }
}



YTreeItem *
YTree::findItemWithId( const YCPValue & id )
{
    for ( unsigned i=0; i < items.size(); i++ )
    {
	YTreeItem *it = items[i]->findItemWithId ( id );

	if ( it )
	    return it;
    }

    return ( YTreeItem * ) 0;
}


YTreeItem *
YTree::findItemWithText( const YCPString & text )
{
    for ( unsigned i=0; i < items.size(); i++ )
    {
	YTreeItem *it = items[i]->findItemWithText ( text );

	if ( it )
	    return it;
    }

    return ( YTreeItem * ) 0;
}


void YTree::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_CurrentItem );
}


bool YTree::parseItemList( const YCPList &	itemList,
			  YTreeItem *		parentItem )
{
    for ( int i=0; i < itemList->size(); i++ )
    {
	YCPValue item = itemList->value(i);

	if ( item->isString() )
	{
	    // The simplest case: just a string, nothing elsewas
	    ( void ) addItem ( parentItem, YCPNull(), item->asString(), false );
	}
	else if ( item->isTerm() && item->asTerm()->name() == YUISymbol_item )
	{
	    // found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() < 1 || iterm->size() > 4 )
	    {
		y2error( "Tree: Invalid argument number in %s",
			 iterm->toString().c_str() );

		return false;
	    }

	    int argnr = 0;


	    // check for item `id() ( optional )

	    YCPValue item_id = YCPNull();

	    if ( YUI::checkId ( iterm->value( argnr ), false ) )
	    {
		item_id = YUI::getId ( iterm->value ( argnr++ ) );
	    }


	    // extract item label ( mandatory )

	    if ( iterm->size() <= argnr || ! iterm->value( argnr )->isString() )
	    {
		y2error( "Tree: Invalid item arguments in %s",
			 iterm->toString().c_str() );

		return false;
	    }

	    YCPString item_label = iterm->value( argnr++ )->asString();


	    bool item_open = false;

	    if ( argnr < iterm->size() )
	    {
		// check for 'open' flag ( true/false ) ( optional )

		if ( iterm->value( argnr )->isBoolean() )
		{
		    item_open = iterm->value( argnr++ )->asBoolean()->value();
		}
	    }

	    YTreeItem * treeItem = addItem ( parentItem, item_id, item_label, item_open );

	    if ( argnr < iterm->size() )
	    {
		// check for sub-item list ( optional )
		if ( ! iterm->value( argnr )->isList() )
		{
		    y2error( "Expecting tree item list instead of: %s",
			     iterm->value( argnr )->toString().c_str() );

		    return false;
		}

		if ( ! parseItemList( iterm->value ( argnr++ )->asList(), treeItem ) )
		{
		    return false;
		}
	    }


	    // Anything left over must be an error.

	    if ( argnr != iterm->size() )
	    {
		y2error( "Tree: Wrong number of arguments in %s", item->toString().c_str() );
	    }
	}
	else
	{
	    y2error( "Invalid item %s: Tree items must be strings or specified with `"
		     YUISymbol_item "()", item->toString().c_str() );
	}
    }

    return true;
}


void YTree::deleteAllItems()
{
    items.clear();
}





/*==========================================================================*/


YTreeItem::YTreeItem( YTree * parent, YCPValue newId, YCPString newText, bool open )
    : id ( newId )
    , _data(0)
    , text ( newText )
    , parentTree ( parent )
    , parentItem ( ( YTreeItem * ) 0 )
    , openByDefault ( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::YTreeItem( YTreeItem * parent, YCPValue newId, YCPString newText, bool open )
    : id ( newId )
    , _data(0)
    , text ( newText )
    , parentTree ( ( YTree * ) 0 )
    , parentItem ( parent )
    , openByDefault ( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::YTreeItem( YTree * parent, YCPString newText, void * data, bool open )
    : id( YCPNull() )
    , _data( data )
    , text ( newText )
    , parentTree ( parent )
    , parentItem ( ( YTreeItem * ) 0 )
    , openByDefault ( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::YTreeItem( YTreeItem * parent, YCPString newText, void * data, bool open )
    : id( YCPNull() )
    , _data( data )
    , text ( newText )
    , parentTree ( ( YTree * ) 0 )
    , parentItem ( parent )
    , openByDefault ( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::~YTreeItem()
{
    // Get rid of all children.
    //
    // Unfortuanately, STL iterators are doo dumb to modify their
    // container - so we really need to do this manually.

    for ( unsigned i=0; i < items.size(); i++ )
    {
	delete items[i];
    }

    items.clear();
}


YTreeItem *
YTreeItem::findItemWithId( const YCPValue & id )
{
    if ( ! getId().isNull() && getId()->equal( id ) )
	return this;

    for ( unsigned i=0; i < items.size(); i++ )
    {
	YTreeItem *it = items[i]->findItemWithId ( id );

	if ( it )
	    return it;
    }

    return ( YTreeItem * ) 0;
}


YTreeItem *
YTreeItem::findItemWithText( const YCPString & text )
{
    if ( getText()->equal( text ) )
	return this;

    for ( unsigned i=0; i < items.size(); i++ )
    {
	YTreeItem *it = items[i]->findItemWithText ( text );

	if ( it )
	    return it;
    }

    return ( YTreeItem * ) 0;
}

