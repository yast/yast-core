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
    YTree(YWidgetOpt &opt, YCPString label);


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
			  const YCPString&	text,
			  bool			open );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget(const YCPSymbol& property, const YCPValue& newvalue);

    /**
     * Implements the ui command queryWidget.
     */
    YCPValue queryWidget(const YCPSymbol& property);

    /**
     * Change the label text. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel(const YCPString& label);

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
    YTreeItem *findItemWithId	( const YCPValue &id );

    /**
     * Recursively search for an item with a given text.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithText	( const YCPString &text );

    /**
     * The items.
     */
    YTreeItemList items;


private:

#if 0
    /**
     * Searches for an item with a certain id or a certain label.
     * Returns the index of the found item or -1 if none was found
     * @param report_error set this to true, if you want me to
     * report an error if non item can be found.
     */
    int itemWithId(const YCPValue &id, bool report_error);
#endif

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    /**
     * The current label of the tree
     */
    YCPString label;
};


class YTreeItem
{
    friend class YTree;

public:

    YTreeItem ( YTree *	    parent, YCPValue id, YCPString text, bool open = false );
    YTreeItem ( YTreeItem * parent, YCPValue id, YCPString text, bool open = false );
    virtual ~YTreeItem();

    const YCPString &		getText()		const { return text;	}
    const YCPValue &		getId()			const { return id;	}
    const YTreeItemList &	itemList()		const { return items;	}
    bool			isOpenByDefault()	const { return openByDefault; }

    /**
     * Recursively search for an item with a given ID.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithId	( const YCPValue &id );

    /**
     * Recursively search for an item with a given text.
     * Returns 0 if not found.
     */
    YTreeItem *findItemWithText	( const YCPString &text );


protected:

    YCPValue		id;
    YCPString		text;
    YTree *		parentTree;
    YTreeItem *		parentItem;
    bool		openByDefault;

    YTreeItemList	items;
};



#endif // YTree_h
