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

  File:		YWizard.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YWizard_h
#define YWizard_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPValue.h>

class YMacroRecorder;
class YWizardPrivate;

#define YWizardID			"wizard"
#define YWizardContentsReplacePointID	"contents"


enum YWizardMode
{
    YWizardMode_Standard,	// Normal wizard (help panel or nothing)
    YWizardMode_Steps,		// Steps visible in left side panel
    YWizardMode_Tree		// Tree in left side panel
};


/**
 * A wizard is a more complex frame typically used for multi-step workflows:
 *
 *    +------------+------------------------------------------------+
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |  Side bar  |                 Content Area                   |
 *    |            |                (YReplacePoint)                 |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |                                                |
 *    |            |  [Back]           [Abort]              [Next]  |
 *    +------------+------------------------------------------------+
 *
 * The side bar can contain help text, a list of steps that are performed, or
 * an embedded tree (much like the YTree widget).
 *
 * The client application creates the wizard and replaces the widget in the
 * content area for each step.
 *
 * The wizard buttons can theoretically be used to do anything, but good UI
 * design will stick to the model above: [Back], [Abort], [Next].
 *
 * If only two buttons are desired, leave the [Back] button's label emtpy. The
 * remaining two buttons will be rearranged accordingly in the button area.
 *
 * In the last step of a multi-step workflow, the [Next] button's label is
 * customarily replaced with a label that indicates that this is the last
 * step. [Accept] is recommended for that button label: [Finish] (as sometimes
 * used in other environments) by no means clearly indicates that this is the
 * positive ending, the final "do it" button. Worse, translations of that are
 * often downright miserable: To German, [Finish] gets translated as [Beenden]
 * which is the same word as "Quit" (used in menus). This does not at all tell
 * the user that that button really performs the requested action the
 * multi-step wizard is all about.
 **/
class YWizard: public YWidget
{
protected:
    /**
     * Constructor.
     *
     * If only two buttons are desired, leave 'backButtonLabel' empty.
     *
     * Each button can take an ID. All IDs can be 0.
     *
     * For wizard buttons that have an ID, a wizard button event will return
     * that ID. For wizard buttons that don't have an ID, a wizard button event
     * will return the button label (stripped of any '&' indicating keyboard
     * shortcuts). 
     *
     * The Wizard's internal wizard buttons assume ownership of the button IDs
     * and will delete them (if non-null) in their destructors.
     **/
    YWizard( YWidget *	 parent,
	     YWidgetID * backButtonId,	const string & backButtonLabel,
	     YWidgetID * abortButtonId,	const string & abortButtonLabel,
	     YWidgetID * nextButtonId,	const string & nextButtonLabel,
	     YWizardMode wizardMode = YWizardMode_Standard );

public:

    /**
     * Destructor.
     **/
    virtual ~YWizard();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     **/
    virtual const char * widgetClass() const { return "YWizard"; }

    /**
     * Generic direct access to implementation-specific functions.
     * Derived classes should implement this.
     **/
    virtual YCPValue command( const YCPTerm & command );

    /**
     * Implements UI::QueryWidget() for this widget class.
     **/
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Return the button labels.
     **/
    string backButtonLabel() const;
    string abortButtonLabel() const;
    string nextButtonLabel() const;

    /**
     * Return the button IDs or 0 if a button doesn't have an ID.
     **/
    YWidgetID * backButtonId() const;
    YWidgetID * abortButtonId() const;
    YWidgetID * nextButtonId() const;

    /**
     * Set the button labels.
     *
     * Derived classes might want to overwrite this, but they should call this
     * respective base class function in the new implementation.
     **/
    virtual void setBackButtonLabel ( const string & newLabel );
    virtual void setAbortButtonLabel( const string & newLabel );
    virtual void setNextButtonLabel ( const string & newLabel );

    /**
     * Set the button IDs. The receiving wizard button assumes ownership over
     * the respective ID and will delete it in its destructor (or when a new ID
     * is set).
     *
     * Derived classes might want to overwrite this, but they should call this
     * respective base class function in the new implementation.
     **/
    virtual void setBackButtonId ( YWidgetID * newId );
    virtual void setAbortButtonId( YWidgetID * newId );
    virtual void setNextButtonId ( YWidgetID * newId );

    
protected:

    /**
     * Returns the current tree selection or an empty string if nothing is
     * selected or there is no tree.
     **/
    virtual YCPString currentTreeSelection() { return YCPString( "" ); }




    // Data members


private:
    ImplPtr<YWizardPrivate> priv;
};


#endif // YWizard_h
