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

  File:	      YDumbTab.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YDumbTab_h
#define YDumbTab_h

#include "YContainerWidget.h"
#include <ycp/YCPString.h>

/**
 * Implementation of the YDumbTab widget
 */
class YDumbTab : public YContainerWidget
{
public:

    /**
     * Constructor
     */
    YDumbTab( const YWidgetOpt & opt );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YDumbTab"; }

    /**
     * Implements the UI::ChangeWidget()
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the UI::QueryWidget()
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Add a new tab - only the header; this widget does not take care of tab
     * contents. This ist the application's responsibility - hence the name DumbTab.
     **/
    void addTab( const YCPValue & 	id,
		 const YCPString &	label,
		 bool 			selected );

protected:

    /**
     * Find a tab header with the specified ID and return its index
     * or -1 if there is no tab header with that ID.
     * For tabs without IDs, the tab labels will be compared.
     **/
    int findTab( const YCPValue & id );
    
    /**
     * Add a tab header with the specified label.
     *
     * Derived classes should reimplement this.
     **/
    virtual void addTab( const YCPString & label );
    
    /**
     * Get the index (0..n) of the currently selected tab.
     *
     * Derived classes should reimplement this.
     **/
    virtual int getSelectedTabIndex();

    /**
     * Select a tab by index (0..n) and send an according event.
     *
     * Derived classes should reimplement this.
     **/
    virtual void setSelectedTab( int index );


    
    class Tab
    {
    public:
	Tab( const YCPValue & id, const YCPString & label )
	    : _id( id )
	    , _label( label )
	    {}

	Tab( const Tab & other )
	    : _id( other.internalId() )
	    , _label( other.label() )
	    {}

	Tab & operator=( const Tab & other )
	{
	    _id    = other.internalId();
	    _label = other.label();

	    return *this;
	}

	YCPString	label() 	const { return _label; }
	YCPValue 	internalId() 	const { return _id; }
	YCPValue 	id()		const
	    { return ( ! _id.isNull() && ! _id->isVoid() ) ? _id : _label; }
	
    private:
	YCPValue  _id;
	YCPString _label;
    };

    
    vector<Tab> _tabs;
};


#endif // YDumbTab_h
