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

  File:	      YMenuButton.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMenuButton_h
#define YMenuButton_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPVoid.h>

class YMenu;
class YMenuItem;

typedef vector<YMenuItem *>		    YMenuItemList;
typedef vector<YMenuItem *>::iterator	    YMenuItemListIterator;
typedef vector<YMenuItem *>::const_iterator YMenuItemListConstIterator;


/**
 * @short Implementation of the MenuButton widget
 */
class YMenuButton : public YWidget
{
public:
    /**
     * Constructor
     * @param opt widget options
     * @param label the button label
     */
    YMenuButton( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YMenuButton"; }

    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * change the label of the push button. Overload this, but call
     * YMenuButton::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label of the MenuButton entry. This method cannot be
     * overidden.  The value of the label cannot be changed other than by
     * calling setLabel, i.e. not by the ui. Therefore setLabel stores the
     * current label in label.
     */
    YCPString getLabel();

    /**
     * Get the MenuButton's toplevel menu.
     */
    YMenu * getToplevelMenu()	{ return toplevel_menu; }


    /**
     * Add one menu item.
     *
     * Pass 0 for 'parent_menu' to add this item to the MenuButton's toplevel
     * menu.
     */
    void 	addMenuItem( const YCPString &	item_label,
			     const YCPValue &	item_id,
			     YMenu *		parent_menu = 0 );
    
    /**
     * Add a submenu.
     *
     * Pass 0 for 'parent_menu' to add this submenu to the MenuButton's toplevel
     * menu.
     */
    YMenu *	addSubMenu( const YCPString &	sub_menu_label,
			    YMenu *		parent_menu = 0	);
    
    /**
     * Retrieve the corresponding application ID to an internal menu item
     * index.
     */
    YCPValue indexToId( int index );

    /**
     * Actually create the menu hierarchy in the specific UI.
     * This is called when the complete menu hierarchy is known.
     *
     * Overwrite this method.
     */
    virtual void createMenu() = 0;

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }
    

protected:
    /**
     * The button label
     */
    YCPString 	label;

    /**
     * The next ( free ) item ID to use
     */
    int 	next_index;

    /**
     * List of all items somewhere in the menu hierarchy.
     */
    YMenuItemList items;
    
    /**
     * The top level menu
     */
    YMenu *	toplevel_menu;
};


/**
 * Helper class for menu items.
 *
 * This class provides the infrastructure for managing children
 * ( i.e. submenus ), yet it doesn't actually handle such children. Derived
 * classes may choose to do this.
 **/
class YMenuItem
{
public:

    /**
     * Constructor.
     */
    YMenuItem( const YCPString &	label,
	       YMenu *			parent_menu	= 0,
	       int			index		= -1,
	       const YCPValue & 	id		= YCPVoid() );

    virtual ~YMenuItem() {}


    const YMenu *	getParent()	const 	{ return parent;	}
    const YCPValue &	getId()		const 	{ return id;	}
    int			getIndex()	const 	{ return index;	}
    const YCPString &	getLabel()	const 	{ return label;	}
    virtual bool	hasChildren()	const	{ return false;	}
    YMenuItemList &	itemList()		{ return items; }
    virtual bool	isMenu()	const	{ return false; }

    /**
     * Set this menu item's label. This will NOT have any immediate
     * visual effect with this base class method - it only stores the new label
     * for later retrieval. If a visual effect is desired, derived classes
     * should overwrite this method. Don't forget to call this base class
     * method in that case!
     **/
    virtual void	setLabel( YCPString newLabel ) { label = newLabel; }

    
protected:

    YCPString		label;
    YCPValue		id;
    YMenu *		parent;
    int			index;
    YMenuItemList	items;
};


/**
 * Helper class for menus.
 *
 * Derived from @ref YMenuItem; this class can actually handle children.
 **/
class YMenu: public YMenuItem
{
public:

    /**
     * Constructor
     */
    YMenu( const YCPString & 	label,
	   YMenu *		parent_menu 	= 0,
	   int			index		= -1,
	   const YCPValue &	id 		= YCPVoid() );

    virtual ~YMenu() {}


    virtual bool	hasChildren()	const	{ return items.size() > 0; }
    void		addMenuItem( YMenuItem *item );
    virtual bool	isMenu()	const	{ return true; }
};


#endif // YMenuButton_h
