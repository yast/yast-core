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

  File:	      YPkgTreeFilterView.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-pkg"
#include <ycp/y2log.h>

#include "YPkgTreeFilterView.h"


YPkgTreeFilterView::YPkgTreeFilterView()
    : _root( 0 )
{
    _root = new YPkgStringTreeItem( YTransText( "<root>" ) );
}


YPkgTreeFilterView::~YPkgTreeFilterView()
{
    y2milestone( "Deleting filter tree" );

    if ( _root )
	delete _root;
}


void
YPkgTreeFilterView::addBranch( std::string 		content,
			       char 			delimiter,
			       YPkgStringTreeItem * 	parent )
{
    if ( ! parent )
	parent = _root;

    if ( delimiter == 0 )
    {
	// Simple case: No delimiter, simply create a new item for 'content'
	// and insert it.

	new YPkgStringTreeItem( YTransText( content ), parent );
    }
    else
    {
	// Split 'content' into substrings and insert each subitem

	std::string::size_type start = 0;
	std::string::size_type end   = 0;

	while ( start < content.length() )
	{
	    // Skip delimiters

	    while ( start < content.length() &&
		    content[ start ] == delimiter )
	    {
		start++;
	    }


	    // Search next delimiter

	    end = start;

	    while ( end < content.length() &&
		    content[ end ] != delimiter )
	    {
		end++;
	    }


	    // Extract substring, if there is any

	    if ( end > start )
	    {
		std::string path_component = content.substr( start, end - start );
		YTransText path_component_trans( path_component );

		// Check if an entry with this text already exists
		YPkgStringTreeItem * node = findDirectChild( parent, path_component_trans);

		if ( ! node )	// No entry with this text yet? Create one.
		    node = new YPkgStringTreeItem( path_component_trans, parent );

		parent = node;
	    }

	    start = end;
	}
    }
}


std::string
YPkgTreeFilterView::completePath( const YPkgStringTreeItem * item,
				  char delimiter,
				  bool startWithDelimiter )
{
    std::string path;
    
    if ( item )
    {
	path = item->value().orig();

	while ( item->parent() && item->parent() != _root )
	{
	    path = item->parent()->value().orig() + delimiter + path;
	    item = item->parent();
	}
	    
    }

    if ( startWithDelimiter )
	path = delimiter + path;

    return path;
}


void
YPkgTreeFilterView::logTree()
{
    y2debug( "Tree:" );
    logBranch( _root, "" );
    y2debug( " " );
}


void
YPkgTreeFilterView::logBranch( YPkgStringTreeItem * branch, std::string indentation )
{
    if ( branch )
    {
	y2debug( "%s%s (%s)", indentation.c_str(),
		 branch->value().translation().c_str(),
		 branch->value().orig().c_str() );

	YPkgStringTreeItem * child = branch->firstChild();
	indentation += "    ";

	while ( child )
	{
	    logBranch( child, indentation );
	    child = child->next();
	}
    }
    else
    {
	y2debug( "%s<NULL>", indentation.c_str() );
    }
}






