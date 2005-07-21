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

  File:	      YMultiProgressMeter.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMultiProgressMeter_h
#define YMultiProgressMeter_h

#include "YWidget.h"
#include <ycp/YCPList.h>
#include <vector>


/**
 * Implementation of the VMultiProgressMeter and HMultiProgressMeter widgets
 */
class YMultiProgressMeter : public YWidget
{
public:

    typedef long long Value_t;

    /**
     * Constructor
     */
    YMultiProgressMeter( const YWidgetOpt & 	opt,
			 bool 			horizontal,
			 const YCPList &	maxValues );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YMultiProgressMeter"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newValue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Returns the number of segments
     **/
    int segments() const { return (int) _maxValues.size(); }

    /**
     * Returns the maximum value for the specified segment (counting from 0)
     **/
    Value_t maxValue( int segment ) const;

    /**
     * Returns the current value for the specified segment (counting from 0).
     * If no value has been set yet, -1 is returned.
     **/
    Value_t currentValue( int segment ) const;

    /**
     * Sets the current value for the specified segment.
     * This must be in the range 0..maxValue( segment ).
     **/
    void setCurrentValue( int segment, Value_t value );

    /**
     * Returns 'true' if the orientation is horizontal.
     **/
    bool horizontal() const { return _horizontal; }
    
    /**
     * Returns 'true' if the orientation is vertical.
     **/
    bool vertical() const { return ! _horizontal; }

    
protected:

    /**
     * Notification that values have been updated and the widget needs to be
     * redisplayed. Derived classes need to reimplement this.
     **/
    virtual void doUpdate() = 0;


private:

    bool			_horizontal;
    std::vector<Value_t>	_maxValues;
    std::vector<Value_t>	_currentValues;
};


#endif // YMultiProgressMeter_h
