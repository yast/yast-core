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

  File:	      YLogView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YLogView_h
#define YLogView_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <deque>

using std::deque;

/**
 * @short Implementation of the LogView widget
 */
class YLogView : public YWidget
{
public:
    /**
     * Constructor.
     */
    YLogView( YWidgetOpt &	opt,
	      const YCPString &	label,
	      int 		visibleLines,
	      int 		maxLines );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YLogView"; }


    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command changeWidget for the widget specific properties.
     */
    YCPValue queryWidget( const YCPSymbol & property);


    /**
     * Set the log text.
     */
    virtual void setLogText( const YCPString & text) = 0;

    /**
     * Get the label above the log lines.
     */
    YCPString 	label( ) 	const { return _label; }

    /**
     * Get the number of visible lines.
     */
    int		visibleLines( )	const { return _visibleLines; }

    /**
     * Get the maximum number of lines to store.
     */
    int		maxLines( )	const { return _maxLines; }

    /**
     * Set the label above the log lines. Overload this, but call
     * YLogView::setLabel at the end of your own function.
     */
    virtual void	setLabel( const YCPString &newLabel );

    /**
     * Retrieve the entire log text as one large string of concatenated lines
     * delimited with newlines.
     */
    string	logText( );

    /**
     * Append one or more lines to the log text.
     */
    void appendText( const YCPString & text );


    /**
     * Append one single line to the log text.
     */
    void appendLine( const string & line );

    /**
     * Clear the log text.
     */
    void clearText( );

    /**
     * Return the current number of lines.
     */
    int lines( ) const { return _logText.size( ); }


private:

    // Data members

    YCPString		_label;
    int			_visibleLines;
    int			_maxLines;

    deque<string>	_logText;
};


#endif // YLogView_h
