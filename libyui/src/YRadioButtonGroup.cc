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

  File:	      YRadioButtonGroup.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#define noVERBOSE_ADD_RADIO_BUTTON

#include <algorithm>

#include <ycp/YCPSymbol.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YRadioButton.h"
#include "YRadioButtonGroup.h"


YRadioButtonGroup::YRadioButtonGroup( YWidgetOpt & opt )
    : YContainerWidget( opt )
{
}


YRadioButtonGroup::~YRadioButtonGroup()
{
    /*
     * When a YRadioButtonGroup is destroyed, the destructors
     * are called in this order:
    *
    * YRadioButtonGroup::~YRadioButtonGroup
    * YContainerWidget::~YContainerWidget
    * YWidget::~YWidget
    *
    * But in YContainerWidget::~YContainerWidget radio buttons belonging
    * to this group may be deleted. They have a pointer to this group and
    * call removeRadioButton. But at this point of time, my vector of
    * RadioButtons is not valid any longer.
    *
    * My solution is: In my destructor I tell all RadioButtons
    * that I'm dead. They set their pointer to RBG to 0.
    * Disadvantage: The 0 pointer must be handled separately
    * at all places in the code. If you have a better solution,
    * please tell me.
    */

    for ( unsigned i=0; i<buttonlist.size(); i++ )
	buttonlist[i]->buttonGroupIsDead();
}


bool YRadioButtonGroup::isRadioButtonGroup() const
{
    return true;
}


void YRadioButtonGroup::addRadioButton( YRadioButton *button )
{
#ifdef VERBOSE_ADD_RADIO_BUTTON
    y2debug( "this=%p, addRadioButton( YRadioButton *%p )",
	    this, button );
#endif
    buttonlist.push_back( button );
}


void YRadioButtonGroup::removeRadioButton( YRadioButton *button )
{
    buttonlist_type::iterator pos = find( buttonlist.begin(), buttonlist.end(), button );
    if ( pos != buttonlist.end() ) buttonlist.erase( pos );
    else
    {
	y2internal( "ButtonGroup %s contains no RadioButton %s",
		   id()->toString().c_str(), button->id()->toString().c_str() );
    }
}


YCPValue YRadioButtonGroup::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();
    /**
     * @property any CurrentButton
     * The id of the currently selected radio button belonging to this group. If
     * no button is selected, CurrentButton is nil.
     */
    if ( s == YUIProperty_CurrentButton ) return YCPBoolean( setCurrentButton( newvalue ) );
    return YWidget::changeWidget( property, newvalue );
}


YCPValue YRadioButtonGroup::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_CurrentButton )
    {
	YRadioButton *cb = currentButton();
	if ( cb ) return cb->id();
	else return YCPVoid();
    }
    return YWidget::queryWidget( property );
}


YRadioButton *YRadioButtonGroup::currentButton() const
{
    for ( unsigned i=0; i<buttonlist.size(); i++ )
	if ( buttonlist[i]->getValue()->value() ) return buttonlist[i];
    return 0;
}


bool YRadioButtonGroup::setCurrentButton( const YCPValue & id )
{
    bool found = false;
    for ( unsigned i=0; i<buttonlist.size(); i++ )
    {
	if ( buttonlist[i]->id()->equal( id ) )
	{
	    buttonlist[i]->setValue( YCPBoolean( true ) );
	    found = true;
	}
	else buttonlist[i]->setValue( YCPBoolean( false ) );
    }
    if ( !found && !id->isVoid() )
    {
	y2warning( "CurrentButton: no RadioButton with id %s belongs to RadioButtonGroup( `id( %s ) )",
		  id->toString().c_str(), this->id()->toString().c_str() );
	return false;
    }
    else return true;
}


void YRadioButtonGroup::uncheckOtherButtons( const YRadioButton *radiobutton )
{
    for ( unsigned i=0; i<buttonlist.size(); i++ )
    {
	if ( buttonlist[i] != radiobutton ) buttonlist[i]->setValue( YCPBoolean( false ) );
    }
}
