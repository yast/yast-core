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

  File:	      YBarGraph.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YBarGraph_h
#define YBarGraph_h

#include <ycp/YCPList.h>
#include "YWidget.h"

/**
 *  @short Implementation of the BarGraph widget
 */
class YBarGraph : public YWidget
{
public:
    /**
     * Constructor
     */
    YBarGraph( const YWidgetOpt & opt );


    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YBarGraph"; }

    /**
     * Returns the current number of bar segments, i.e. the nuber of values.
     */
    int segments();

    /**
     * Returns the value of segment no. n
     * or -1 if there are not that many segments.
     */
    int value( int n );

    /**
     * Returns the label of segment no. n
     * or an empty string if there are not that many segments
     * or the specified segment doesn't have a label.
     */
    string label( int n );

    /**
     * Parse and store new values.
     */
    void parseValuesList( const YCPList & newValues );


    /**
     * Parse and store new labels.
     */
    void parseLabelsList( const YCPList & newLabels );

    /**
     * Perform a display update after values and/or labels have changed.
     * Overwrite this method and do your actual drawing here.
     */
    virtual void doUpdate();

protected:

private:

    /**
     * Set specific widget properties.
     * Inherited from YWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newValue );

    vector<int>		_values;
    vector<string>	_labels;
};


#endif // YBarGraph_h
