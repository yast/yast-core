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

  File:	      YContainerWidget.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPSymbol.h>
#include "YContainerWidget.h"
#include "YWizard.h"
#include "YLabel.h"
#include "YMenuButton.h"
#include "YPushButton.h"


YContainerWidget::YContainerWidget( const YWidgetOpt & opt )
    : YWidget( opt )
{
    _debugLabelWidget = 0;
}


YContainerWidget::~YContainerWidget()
{
    if ( ! isValid() )
    {
	y2error( "ERROR: Trying to destroy invalid container widget" );
	return;
    }

    removeChildren();
}


void YContainerWidget::childDeleted( YWidget *deletedChild )
{
    if ( ! isValid() )
    {
	y2error( "YContainerWidget::childDeleted( deletedChild=%s #%d ): Invalid container widget",
		 deletedChild->widgetClass(), deletedChild->internalId() );
	return;
    }

    if ( deletedChild == _debugLabelWidget )
	_debugLabelWidget = 0;

    /*
     * Search the entry for the deleted widget in the children list and delete it
     */

    bool found = false;
    vector<YWidget *>::iterator it = children.begin();
    int deletedChildId = deletedChild->internalId();

    while ( ! found && it != children.end() )
    {
	if ( ( *it )->internalId() == deletedChildId )	// Don't compare pointers here!
	    found = true;
	else
	    ++it;
    }

    if ( found )
    {
	childRemoved( *it ); // Notify derived classes
	children.erase( it );
    }

    /*
     * It's OK if the child hasn't been found. This means nothing worse
     * than somebody else already erased the entry from the children
     * list - e.g. removeChildren().
     */
}


void YContainerWidget::dumpDialogWidgetTree()
{
    YContainerWidget *dialog = dynamic_cast <YContainerWidget *> ( yDialog() );

    if ( dialog )
	dialog->dumpWidgetTree();
    else
	dumpWidgetTree();
}


void YContainerWidget::dumpWidgetTree( int indentationLevel )
{
    dumpWidget( this, indentationLevel );

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( children[i]->isContainer() )
	    ( dynamic_cast <YContainerWidget *> ( children[i] ) )->dumpWidgetTree ( indentationLevel + 1 );
	else
	    dumpWidget( children[i], indentationLevel + 1 );
    }
}


void YContainerWidget::dumpWidget( YWidget *w, int indentationLevel )
{
    string indentation ( indentationLevel * 4, ' ' );

    string descr( w->debugLabel() );

    if ( ! descr.empty() )
	descr = "\"" + descr + "\"";

    if ( w->hasId() )
    {
	if ( ! descr.empty() )
	    descr += " ";

	descr += "`id( " + w->id()->toString() + " )";
    }

    string stretch;

    if ( w->stretchable( YD_HORIZ ) )	stretch += "hstretch ";
    if ( w->stretchable( YD_VERT  ) )	stretch += "vstretch";

    if ( ! stretch.empty() )
	stretch = "(" + stretch + ") ";

    if ( descr.empty() )
    {
	y2milestone( "Widget tree: %s%s #%d %sat %p",
		     indentation.c_str(), w->widgetClass(), w->internalId(), stretch.c_str(), w );
    }
    else
    {
	y2milestone( "Widget tree: %s%s %s %sat %p",
		     indentation.c_str(), w->widgetClass(), descr.c_str(), stretch.c_str(), w );
    }
}


void YContainerWidget::setChildrenEnabling( bool enabled )
{
    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( children[i]->isContainer() )
	{
	    YContainerWidget * container = dynamic_cast<YContainerWidget *>( children[i] );

	    if ( container )
	    {
		// y2debug( "Recursing into %s", container->debugLabel().c_str() );
		container->setChildrenEnabling( enabled );
	    }
	}
	else
	    children[i]->setEnabling( enabled );
    }
}


void YContainerWidget::addChild( YWidget *child )
{
    if ( find( children.begin(), children.end(), child ) != children.end() )
    {
	y2error( "ERROR: Child added twice (%s, #%d) (%s, #%d)",
		 this->widgetClass(), this->internalId(),
		 child->widgetClass(), child->internalId() );
	return;
    }

    if ( ! _debugLabelWidget )
    {
	if 	( dynamic_cast<YWizard  	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YLabel   	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YMenuButton 	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YPushButton 	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YLabel		*> (child ) )	_debugLabelWidget = child;
    }

    children.push_back( child );
    childAdded( child ); // tell subclassed ui specific widget
}


void YContainerWidget::removeChildren()
{
    while ( numChildren() > 0 )
    {
	YWidget *child = children[0];
	children.erase( children.begin() );
	_debugLabelWidget = 0;

	if ( child->isValid() )
	{
	    childRemoved( child ); // Notify derived classes
	    delete child;
	}
	else
	{
	    y2error ("ERROR: Invalid widget child - ignored");
	}
    }
}


YWidget *YContainerWidget::findWidget( const YCPValue & id ) const
{
    for ( int c=0; c<numChildren(); c++ )
    {
	if ( children[c]->isValid() )
	{
	    if ( children[c]->id()->equal( id ) )
		return children[c];

	    if ( children[c]->isContainer() )
	    {
		YWidget *found = ( dynamic_cast <YContainerWidget *>( children[c] ) )->findWidget( id );
		if ( found ) return found;
	    }
	}
	else
	{
	    y2error( "ERROR: Invalid widget child #%d - ignored", c );
	}
    }

    return 0;
}


bool YContainerWidget::isContainer() const
{
    return true;
}


long YContainerWidget::nicesize( YUIDimension dim )
{
    return child(0)->nicesize( dim );
}


bool YContainerWidget::stretchable( YUIDimension dim ) const
{
    return child(0)->stretchable( dim );
}


void YContainerWidget::setSize( long newwidth, long newheight )
{
    child(0)->setSize( newwidth, newheight );
}



void YContainerWidget::childAdded( YWidget * )
{
    // dummy default implementation
}


void YContainerWidget::childRemoved( YWidget * )
{
    // dummy default implementation
}


int YContainerWidget::numChildren() const
{
    return children.size();
}


bool YContainerWidget::hasChildren() const
{
    return children.size() > 0;
}


YWidget *YContainerWidget::child( int i ) const
{
    if ( i >= 0 && i < numChildren() )
	return children[i];
    else
    {
	y2internal( "INTERNAL ERROR: YContainerWidget::child(): "
		    "No child #%d ( have only %d children )",
		    i, numChildren() );

	return 0;
    }
}


string YContainerWidget::debugLabel()
{
    if ( _debugLabelWidget )
    {
	string label;
	
	if ( _debugLabelWidget == this )
	    label = YWidget::debugLabel();
	else
	    label = _debugLabelWidget->debugLabel();

	if ( ! label.empty() )
	    return formatDebugLabel( _debugLabelWidget, label );
    }

    for ( int i=0; i < numChildren(); i++ )
    {
	if ( child(i) && child(i)->isValid() )
	{
	    string label = child(i)->debugLabel();

	    if ( ! label.empty() )
		return formatDebugLabel( child(i), label );
	}
    }


    return "";
}


string YContainerWidget::formatDebugLabel( YWidget * widget, const string & debLabel )
{
    if ( ! widget || debLabel.empty() )
	return "";

    string label;

    if ( widget->isContainer() )
    {
	label = debLabel;
    }
    else
    {
	label = "YContainerWidget with ";
	label = widget->widgetClass();
	label += " \"";
	label += debLabel;
	label += "\"";
    }

    return label;
}


void YContainerWidget::saveUserInput( YMacroRecorder *macroRecorder )
{
    for ( vector<YWidget *>::iterator it = children.begin();
	  it != children.end();
	  ++it )
    {
	YWidget *widget = *it;

	if ( widget->isContainer() || widget->hasId() )
	{
	    /*
	     * It wouldn't do any good to save the user input of any widget
	     * that doesn't have an ID since this ID is required to make use of
	     * this saved data later when playing the macro.
	     * Other than that, container widgets need to recurse over all
	     * their children.
	     */

	    widget->saveUserInput( macroRecorder );
	}
    }
}


void YContainerWidget::collectUserInput( YCPList & contentsList )
{
    for ( vector<YWidget *>::iterator it = children.begin();
	  it != children.end();
	  ++it )
    {
	YWidget *widget = *it;

	if ( widget->isContainer() )
	{
	    ( (YContainerWidget *) widget)->collectUserInput( contentsList );
	}
	else if ( widget->hasId() &&			// No use without an ID
		  widget->userInputProperty() )		// Only for widgets that hold user input
	{
	    YCPMap map;

	    map->add( YCPString( "ID"		), widget->id() );
	    map->add( YCPString( "Property"	), YCPSymbol( widget->userInputProperty() ) );
	    map->add( YCPString( "Value"	), widget->queryWidget( YCPSymbol( widget->userInputProperty() ) ) );

	    // Add WidgetClass

	    const char * widgetClass = widget->widgetClass();

	    if ( widgetClass )
	    {
		if ( *widgetClass == 'Y' )	// skip leading "Y" (YPushButton, YTextEntry, ...)
		    widgetClass++;

		map->add( YCPString( "WidgetClass" ), YCPSymbol( widgetClass ) );
	    }


	    // Add the Widget's debug label.
	    // This is usually the label (translated to the user's locale).

	    string debugLabel = widget->debugLabel();

	    if ( ! debugLabel.empty() )
		map->add( YCPString( "DebugLabel" ), YCPString( debugLabel ) );

	    contentsList->add( map );
	}
    }
}

