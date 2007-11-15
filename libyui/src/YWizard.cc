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

  File:		YWizard.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPTerm.h>
#include "YCPValueWidgetID.h"

#include "YWizard.h"

struct YWizardPrivate
{
    YWizardPrivate( YWidgetID * 	backButtonId,	const string & backButtonLabel,
		    YWidgetID * 	abortButtonId,	const string & abortButtonLabel,
		    YWidgetID * 	nextButtonId,	const string & nextButtonLabel,
		    YWizardMode 	wizardMode )
	: backButtonLabel( backButtonLabel )
	, abortButtonLabel( abortButtonLabel )
	, nextButtonLabel( nextButtonLabel )
	, backButtonId( backButtonId )
	, abortButtonId( abortButtonId )
	, nextButtonId( nextButtonId )
	{}

    string	backButtonLabel;
    string	abortButtonLabel;
    string	nextButtonLabel;
    YWidgetID * backButtonId;
    YWidgetID *	abortButtonId;
    YWidgetID *	nextButtonId;
    YWizardMode	wizardMode;
};




YWizard::YWizard( YWidget *	parent,
		  YWidgetID * 	backButtonId,	const string & backButtonLabel,
		  YWidgetID * 	abortButtonId,	const string & abortButtonLabel,
		  YWidgetID * 	nextButtonId,	const string & nextButtonLabel,
		  YWizardMode 	wizardMode )
    : YWidget( parent )
      , priv( new YWizardPrivate( backButtonId, 	backButtonLabel,
				  abortButtonId,	abortButtonLabel,
				  nextButtonId,		nextButtonLabel,
				  wizardMode ) )
{
    YUI_CHECK_NEW( priv );

    // On the YWidget level, a Wizard has a content area and a couple of
    // buttons as children, so simply subclassing from YSimpleChildManager
    // won't do; a children manager that can handle more children is needed.
    setChildrenManager( new YWidgetChildrenManager( this ) );

    // All wizard widgets have a fixed ID `wizard
    YWidget::setId( new YCPValueWidgetID( YCPSymbol( YWizardID ) ) );

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YWizard::~YWizard()
{
    // Intentionally not deleting the button IDs here: The wizard buttons
    // assume responsibility for them. For YWidget derived wizard buttons, the
    // YWidget destructor will delete the ID.
}


string YWizard::backButtonLabel() const
{
    return priv->backButtonLabel;
}


string YWizard::abortButtonLabel() const
{
    return priv->abortButtonLabel;
}


string YWizard::nextButtonLabel() const
{
    return priv->nextButtonLabel;
}


YWidgetID * YWizard::backButtonId() const
{
    return priv->backButtonId;
}


YWidgetID * YWizard::abortButtonId() const
{
    return priv->abortButtonId;
}


YWidgetID * YWizard::nextButtonId() const
{
    return priv->nextButtonId;
}


void YWizard::setBackButtonLabel ( const string & newLabel )
{
    priv->backButtonLabel = newLabel;
}


void YWizard::setAbortButtonLabel( const string & newLabel )
{
    priv->abortButtonLabel = newLabel;
}


void YWizard::setNextButtonLabel ( const string & newLabel )
{
    priv->nextButtonLabel = newLabel;
}


void YWizard::setBackButtonId ( YWidgetID * newId )
{
    priv->backButtonId = newId;
}


void YWizard::setAbortButtonId( YWidgetID * newId )
{
    priv->abortButtonId = newId;
}


void YWizard::setNextButtonId ( YWidgetID * newId )
{
    priv->nextButtonId = newId;
}






YCPValue YWizard::command( const YCPTerm & command )
{
    y2error( "YWizard::command() not reimplemented!" );

    return YCPNull();
}



YCPValue YWizard::queryWidget( const YCPSymbol & property )
{
    string sym = property->symbol();

    if ( sym == YUIProperty_CurrentItem )
    {
	return currentTreeSelection();
    }
    else return YWidget::queryWidget( property );
}

