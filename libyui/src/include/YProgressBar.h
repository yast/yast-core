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

  File:	      YProgressBar.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YProgressBar_h
#define YProgressBar_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>

/**
 * @short Implementation of the ProgressBar widget
 */
class YProgressBar : public YWidget
{
public:

    typedef long long Value_t;
    
    /**
     * Constructor.
     */
    YProgressBar( const YWidgetOpt & 	opt,
		  const YCPString & 	label,
		  const YCPInteger & 	maxProgress,
		  const YCPInteger & 	initialProgress );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YProgressBar"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the ProgressBar label. Overload this, but call
     * YProgressBar::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label. This method cannot be overidden.  The label of the
     * ProgressBar cannot be changed other than by calling setProgress, i.e. not
     * by the ui. Therefore setProgress stores the current ProgressBar in
     * #ProgressBar.
     */
    YCPString getLabel();

    /**
     * Change the progress. Overload this, but call
     * YProgressBar::setProgress at the end of your own function.
     */
    virtual void setProgress( const YCPInteger & progress );

    /**
     * Get the current progress.  This method cannot be overidden.  The progress
     * of the cannot be changed other than by calling setProgress, i.e. not by
     * the ui. Therefore setProgress stores the current ProgressBar in
     * #ProgressBar.
     */
    YCPInteger getProgress();


protected:
    /**
     * Current label of the ProgressBar
     */
    YCPString label;

    /**
     * Maximum progress value of the progress bar.
     * The progress can go from 0 to maxprogress.
     */
    YCPInteger maxProgress;

    /**
     * current progress value of the ProgressBar.
     */
    YCPInteger progress;
};


#endif // YProgressBar_h
