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

  File:	      YTree.h

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YTree_h
#define YTree_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>


class YMacroRecorder;
class YTreeItem;

typedef vector<YTreeItem *>		    YTreeItemList;
typedef vector<YTreeItem *>::iterator	    YTreeItemListIterator;
typedef vector<YTreeItem *>::const_iterator YTreeItemListConstIterator;


/**
 * @short Implementation of the Tree widget
 */
class YTree : public YWidget
{
    friend class YTreeItem;

public:

    /**
     * Constructor
     * @param opt the widget options
     * @param text the initial text of the label
     */
    YTree( const YWidgetOpt & opt, YCPString label );


    /**
     * Destructor. Frees all tree items.
     **/
    virtual ~YTree();


    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YTree"; }


    /**
     * Called once after items have been added.
     *
     * Overload this to fill the ui specific widget with items.
     */
    virtual void rebuildTree();

    /**
     * Adds an item to the selection box.
     */
    YTreeItem *	addItem ( YTreeItem *		parentItem,
			  const YCPValue &	id,
			  const YCPString &	text,
			  const YCPString &	iconName,
			  bool			open );

    /**
     * Adds an item to the selection box.
     *
     * Uses an opaque data pointer for application use. Use YTreeItem::data()
     * to retrieve this kind of data. The application is responsible for
     * the data contents - and of course for avoiding dangling pointers.
     */
    YTreeItem *	addItem ( YTreeItem *		parentItem,
			  const YCPString &	text,
			  const YCPString &	iconName,
			  void *		data,
			  bool			open );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the label text. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label text. This method cannot be overidden.
     *
     * The value of the label cannot be changed other than by calling
     * setLabel, i.e. not by the ui. Therefore setLabel stores the
     * current label in #label.
     */
    YCPString getLabel();

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }

    /**
     * Parses a given item list and calls addItem(...) to insert entries.
     *
     * Returns 'true' on succes, 'false' on error.
     */
    bool parseItemList( const YCPList & itemList, YTreeItem *parentItem = 0);

    /**
     * Returns 'true' if any item of this widget has an icon
     **/
    bool hasIcons() const { return _hasIcons; }

    /**
     * Returns a list of the current item and all its ancestors from the
     * root item to the current item - either the respective IDs or, for items
     * that don't have IDs, the item text. If no item is currently selected,
     * YCPVoid ('nil') is returned.
     **/
    YCPValue currentBranch() const;


protected:

    /**
     * Returns the index of the currently
     * selected item or -1 if no item is selected.
     */
    virtual const YTreeItem * getCurrentItem() const = 0;

    /**
     * Selects an item.
     */
    virtual void setCurrentItem ( YTreeItem * it ) = 0;

    /**
     * Recursively search for an item with a given ID.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithId	( const YCPValue & id );

    /**
     * Recursively search for an item with a given text.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithText	( const YCPString & text );

    /**
     * Returns a YCPList of items in the format needed for creating a tree
     * widget.
     **/
    YCPList itemsTermList( YTreeItemList items );

    /**
     * Recursively fills a map 'openItems' with items that are open.
     **/
    void findOpenItems( YCPMap & openItems, YTreeItemList items );

    /**
     * Cleares the YTreeItemList. This function is
     * calles out of the corresponding YQ classes.
     */
    virtual void deleteAllItems();

    /**
     * Recursively add items to list 'branchList' from 'item' up to the tree's root
     **/
    static void branchToList( YCPList & branchList, const YTreeItem * item );


    //
    // Data members
    //
    
    YTreeItemList items;


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    //
    // Data members
    //

    YCPString label;
    bool _hasIcons;
};


class YTreeItem
{
    friend class YTree;

public:

    /**
     * Regular constructor for root level items.
     **/
    YTreeItem ( YTree *			parent,
		const YCPValue	&	id,
		const YCPString &	text,
		const YCPString &	iconName,
		bool			open = false );

    /**
     * Regular constructor for items in deeper tree levels.
     **/
    YTreeItem ( YTreeItem * 		parent,
		const YCPValue &	id,
		const YCPString &	text,
		const YCPString &	iconName,
		bool 			open = false );


    /**
     * Special constructor for root items that take an opaque data pointer for
     * application use: This kind of root item can be used to reference to
     * external objects that are connected with this tree item.
     * Use YTreeItem::data() to retrieve this pointer. Casting will be
     * necessary to make any use of it.
     **/
    YTreeItem ( YTree *	    		parent,
		const YCPString &	text,
		const YCPString &	iconName,
		void * 			data,
		bool 			open = false );


    /**
     * Special constructor for deeper level items that take an opaque data pointer for
     * application use: This kind of root item can be used to reference to
     * external objects that are connected with this tree item.
     * Use YTreeItem::data() to retrieve this pointer. Casting will be
     * necessary to make any use of it.
     **/
    YTreeItem ( YTreeItem * 		parent,
		const YCPString &	text,
		const YCPString &	iconName,
		void * 			data,
		bool 			open = false );


    /**
     * Destructor.
     **/
    virtual ~YTreeItem();

    YTreeItem *			parent()		const { return _parentItem; }
    YTree *			tree()			const { return _parentTree; }
    const YCPString &		getText()		const { return _text;	}
    const YCPValue &		getId()			const { return _id;	}
    const YTreeItemList &	itemList()		const { return _items;	}
    bool			isOpenByDefault()	const { return _openByDefault; }

    /**
     * Recursively search for an item with a given ID.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithId	( const YCPValue & id );

    /**
     * Recursively search for an item with a given text.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithText	( const YCPString & text );

    /**
     * Set this item's "open" flag. The UI has to take care to set this each
     * time the user opens or closes a branch.
     **/
    void setOpen( bool open ) { _open = open; }

    /**
     * Returns this item's "open" flag.
     **/
    bool isOpen() const { return _open; }

    /**
     * Returns the opaque data pointer for applicaton use.
     **/
    void * data() const { return _data; }

    /**
     * Set the opaque data pointer. The application may choose to store
     * internal data here. Watch for dangling pointers!
     **/
    void setData( void * data ) { _data = data; }

    /**
     * Returns the name of this item's icon or an empty string if it doesn't have one.
     **/
    YCPString iconName() const;

    /**
     * Sets this item's icon name.
     **/
    void setIconName( const YCPString & icon ) { _iconName = icon; }


protected:

    //
    // Data members
    //

    YCPValue		_id;
    void *		_data;
    YCPString		_text;
    YCPString		_iconName;
    YTree *		_parentTree;
    YTreeItem *		_parentItem;
    bool		_openByDefault;
    bool		_open;

    YTreeItemList	_items;
};



#endif // YTree_h
