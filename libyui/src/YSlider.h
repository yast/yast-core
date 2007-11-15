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

  File:		YSlider.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YSlider_h
#define YSlider_h

#include "YIntField.h"

class YMacroRecorder;
class YSliderPrivate;


/**
 * Slider: Input widget for an integer value between a minimum and a maximum
 * value. Very similar to IntField in semantics, but with a graphical slider
 * that can be dragged to the desired value. It also contains an IntField to
 * allow entering the value directly.
 *
 * Don't confuse this widget with ProgressBar: ProgressBar is output-only.
 *
 *
 * This is an optional widget, i.e. not all UIs support it.
 **/
class YSlider : public YIntField
{
protected:
    /**
     * Constructor.
     *
     * Create a Slider with 'label' as the caption, and the specified minimum
     * and maximum values.
     *
     * Note that YWidgetFactory::createSlider() also has an 'initialValue'
     * parameter that is not used here (because the current value is not stored
     * in this base class, but in the derived class).
     **/
    YSlider( YWidget *		parent,
	     const string & 	label,
	     int		minValue,
	     int		maxValue );

public:
    /**
     * Destructor.
     **/
    virtual ~YSlider();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     **/
    virtual const char * widgetClass() const { return "YSlider"; }

private:

    ImplPtr<YSliderPrivate> priv;
};


#endif // YSlider_h
