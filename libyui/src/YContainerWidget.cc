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
#include "YContainerWidget.h"


YContainerWidget::YContainerWidget( const YWidgetOpt & opt )
    : YWidget( opt )
{
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
	children.erase( it );

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

    y2debug( "Widget tree: %s%s #%d at %p",
	     indentation.c_str(), w->widgetClass(), w->internalId(), w );
}


void YContainerWidget::addChild( YWidget *child )
{
    if (find (children.begin (), children.end (), child) != children.end ())
    {
	y2error ("ERROR: Child added twice (%s, #%d) (%s, #%d)",
		 this->widgetClass(), this->internalId(),
		 child->widgetClass(), child->internalId());
	return;
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

	if ( child->isValid() )
	{
	    childRemoved( child ); // tell subclassed ui specific widget
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


void YContainerWidget::saveUserInput( YMacroRecorder *macroRecorder )
{
    vector<YWidget *>::iterator it = children.begin();

    while ( it != children.end() )
    {
	YWidget *widget = *it;

	if ( widget->isContainer() || widget->hasId() )
	{
	    /*
	     * It wouldn't do any good to save the user input of any widget
	     * that doesn't have an ID since this ID is required to make use of
	     * this saved data later when playing the macro.
	     * Other than that, container widgets need to recurse over all their children.
	     */

	    widget->saveUserInput( macroRecorder );
	}

	++it;
    }
}

