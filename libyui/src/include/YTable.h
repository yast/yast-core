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

  File:	      YTable.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YTable_h
#define YTable_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;


/**
 * @short helper class for the table widget: one single table row
 */
class YTableRow
{
    YCPValue id;
    vector<string> elements;
    YTableRow( YCPValue id, vector<string> elements )
	: id( id ), elements( elements ) {};
    friend class YTable;

    /**
     * Construct a `item( `id( .. ), "asdf", "asdf", ... ) term
     */
    YCPTerm makeTerm() const;
};



/**
 * @short the table widget
 */
class YTable : public YWidget
{
public:

    /**
     * Creates a new and empty Table
     * @param num_cols The number of columns of the table
     */
    YTable( YWidgetOpt & opt, int num_cols );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YTable"; }

    /**
     * Adds an item to the table that is given as term
     * yet to be parsed.
     */
    bool addItem( const YCPValue & item );

    /**
     * Adds a list of items to the table. The list
     * contains item specifications yet to be parsed.
     */
    virtual bool addItems( const YCPList & itemlist );

    /**
     * Adds an item to the table.
     */
    void addItem( const YCPValue & id, vector<string> elements );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command changeWidget with property given as term.
     */
    YCPValue changeWidgetTerm( const YCPTerm & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Implements the ui command queryWidget with property given as term.
     */
    YCPValue queryWidgetTerm( const YCPTerm & property );

    /**
     * Returns the number of columns of the table.
     */
    int numCols() const;

protected:
    /**
     * Is called, when an item ( a row ) has been added. Overload this to
     * fill the ui specific widget with items.
     * @param elements the strings of the elements, one for each column.
     * @param index index of the new item.
     */
    virtual void itemAdded( vector<string> elements, int index );

    /**
     * Is called, when all items have been cleared. Overload this
     * and clear the ui specific table.
     */
    virtual void itemsCleared() = 0;

    /**
     * Is called, when the contents of a cell has been changed. Overload
     * this and change the cell text.
     */
    virtual void cellChanged( int index, int colnum, const YCPString & newtext ) = 0;

    /**
     * Returns the index of the currently
     * selected item or -1 if no item is selected.
     */
    virtual int getCurrentItem() = 0;

    /**
     * Makes another item the selected one.
     */
    virtual void setCurrentItem( int index ) = 0;


protected:
    /**
     * The current data in the table
     */
    vector<YTableRow> rows;

    /**
     * The number of columns of the table
     */
    int num_cols;

    /**
     * Returns the current number of items
     */
    int numItems() const;

    /**
     * Searches for an item with a certain id or a certain label.
     * Returns the index of the found item or -1 if none was found
     * @param report_error set this to true, if you want me to
     * report an error if non item can be found.
     */
    int itemWithId( const YCPValue & id, bool report_error );


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YTable_h
