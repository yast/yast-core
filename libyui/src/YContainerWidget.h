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

  File:		YContainerWidget.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YContainerWidget_h
#define YContainerWidget_h

#include "YWidget.h"

/**
 * Container widget.
 *
 * Pretty much obsolete now that children management went to YWidget.
 **/
class YContainerWidget : public YWidget
{
protected:
    /**
     * Constructor.
     **/
    YContainerWidget( const YWidgetOpt & opt );

public:
    /**
     * Cleans up: Deletes all child widgets.
     **/
    virtual ~YContainerWidget();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     **/
    virtual const char * widgetClass() const { return "YContainerWidget"; }

    /**
     * Preferred width of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual int preferredWidth();

    /**
     * Preferred height of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual int preferredHeight();
    
    /**
     * Set the new size of the widget.
     * In this case, the size of the single child is set.
     *
     * Reimplemented from YWidget.
     **/
    virtual void setSize( int newWidth, int newHeight );

    /**
     * Return stretchability of this widget.
     *
     * Default implementation, assuming exactly one child.
     **/
    bool stretchable( YUIDimension dim ) const;

    /**
     * Adds a new child widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual void addChild( YWidget *child );


public:
    /**
     * Returns a (possibly translated) text describing this dialog for
     * debugging.
     **/
    virtual std::string debugLabel();

protected:

    /**
     * Format a debug label.
     **/
    string formatDebugLabel( YWidget * widget, const string & debLabel );

    //
    // Data members
    //

    YWidget * _debugLabelWidget;
};


#endif // YContainerWidget_h

