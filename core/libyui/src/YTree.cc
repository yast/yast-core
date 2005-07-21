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
#include "YSelectionWidget.h"
#include "YMacroRecorder.h"
#include "YTree.h"
#include "YUI.h"


YTree::YTree( const YWidgetOpt & opt, YCPString newLabel )
    : YWidget( opt )
    , label( newLabel )
    , _hasIcons( false )
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
		const YCPString &	iconName,
		bool			open )
{
    YTreeItem *treeItem;

    if ( parentItem )
	treeItem = new YTreeItem( parentItem, id, text, iconName, open );
    else
	treeItem = new YTreeItem( this, id, text, iconName, open );

    return treeItem;
}


YTreeItem *
YTree::addItem( YTreeItem *		parentItem,
		const YCPString &	text,
		const YCPString &	iconName,
		void *			data,
		bool			open )
{
    YTreeItem *treeItem;

    if ( parentItem )
	treeItem = new YTreeItem( parentItem, text, iconName, data, open );
    else
	treeItem = new YTreeItem( this, text, iconName, data, open );

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
	    ( void ) addItem ( parentItem, YCPNull(), item->asString(), YCPString( "" ), false );
	}
	else if ( item->isTerm()
		  && item->asTerm()->name() == YUISymbol_item )	// `item(...)
	{
	    YCPValue	id		= YCPNull();
	    YCPString	label		= YCPNull();
	    YCPString	iconName	= YCPNull();
	    YCPBoolean	isOpen		= YCPNull();
	    YCPList	subItemList	= YCPNull();

	    if ( YSelectionWidget::parseItem( item->asTerm(),				// in
					      id, label, iconName, isOpen, subItemList) )	// out
	    {
		if ( isOpen.isNull() )
		    isOpen = YCPBoolean( false );

		if ( ! iconName.isNull() )
		    _hasIcons = true;

		YTreeItem * treeItem = addItem( parentItem, id, label, iconName, isOpen->value() );

		if ( ! subItemList.isNull() )
		{
		    if ( ! parseItemList( subItemList, treeItem ) )
			return false;
		}
	    }
	    else
	    {
		return false;
	    }
	}
	else
	{
	    y2error( "Invalid item %s: Tree items must be strings or specified with `"
		     YUISymbol_item "()", item->toString().c_str() );
	    return false;
	}
    }

    return true;
}


void YTree::deleteAllItems()
{
    _hasIcons = false;
    items.clear();
}





/*==========================================================================*/


YTreeItem::YTreeItem( YTree * 		parent,
		      const YCPValue & 	id,
		      const YCPString &	text,
		      const YCPString &	iconName,
		      bool 		open )
    : _id( id )
    , _data( 0 )
    , _text( text)
    , _iconName( iconName )
    , _parentTree( parent )
    , _parentItem( ( YTreeItem * ) 0 )
    , _openByDefault( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::YTreeItem( YTreeItem * 	parent,
		      const YCPValue &	id,
		      const YCPString &	text,
		      const YCPString & iconName,
		      bool 		open )
    : _id( id )
    , _data(0)
    , _text( text)
    , _iconName( iconName )
    , _parentTree( ( YTree * ) 0 )
    , _parentItem( parent )
    , _openByDefault( open )
    , _open( open )
{
    parent->_items.push_back ( this );
}


YTreeItem::YTreeItem( YTree * 		parent,
		      const YCPString & text,
		      const YCPString & iconName,
		      void * 		data,
		      bool 		open )
    : _id( YCPNull() )
    , _data( data )
    , _text( text )
    , _iconName( iconName )
    , _parentTree( parent )
    , _parentItem( ( YTreeItem * ) 0 )
    , _openByDefault( open )
    , _open( open )
{
    parent->items.push_back ( this );
}


YTreeItem::YTreeItem( YTreeItem * 	parent,
		      const YCPString &	text,
		      const YCPString & iconName,
		      void * 		data,
		      bool 		open )
    : _id( YCPNull() )
    , _data( data )
    , _text( text )
    , _iconName( iconName )
    , _parentTree( ( YTree * ) 0 )
    , _parentItem( parent )
    , _openByDefault( open )
    , _open( open )
{
    parent->_items.push_back ( this );
}


YTreeItem::~YTreeItem()
{
    // Get rid of all children.
    //
    // Unfortuanately, STL iterators are doo dumb to modify their
    // container - so we really need to do this manually.

    for ( unsigned i=0; i < _items.size(); i++ )
    {
	delete _items[i];
    }

    _items.clear();
}


YTreeItem *
YTreeItem::findItemWithId( const YCPValue & id )
{
    if ( ! getId().isNull() && getId()->equal( id ) )
	return this;

    for ( unsigned i=0; i < _items.size(); i++ )
    {
	YTreeItem *it = _items[i]->findItemWithId ( id );

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

    for ( unsigned i=0; i < _items.size(); i++ )
    {
	YTreeItem *it = _items[i]->findItemWithText ( text );

	if ( it )
	    return it;
    }

    return ( YTreeItem * ) 0;
}


YCPString
YTreeItem::iconName() const
{
    return _iconName.isNull() ? YCPString( "" ) : _iconName;
}
