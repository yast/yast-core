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

  File:	      YPkgTreeFilterView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YPkgTreeFilterView_h
#define YPkgTreeFilterView_h

#include <string>
#include "YTransText.h"
#include "TreeItem.h"


typedef SortedTreeItem<YTransText>	YPkgStringTreeItem;


/**
 * @short Abstract base class for filter views with hierarchical filter
 * criteria - e.g., RPM group tags, MIME types.
 **/
class YPkgTreeFilterView
{
public:

    /**
     * Constructor.
     **/

    YPkgTreeFilterView();

    /**
     * Destructor.
     **/
    virtual ~YPkgTreeFilterView();

    /**
     * Add a new branch with text content 'content' to the tree, beginning at
     * 'parent' (root if parent == 0).
     * This content can be a path specification delimited with character
     * 'delimiter' (if not 0), i.e. this method will split 'content' up into
     * path components and insert tree items for each level as
     * appropriate. Leading delimiters will be ignored.
     * If 'delimiter' is 0, 'content' is not split but used 'as is'.
     * Items are automatically sorted alphabetically.
     *
     * Example:
     *    addBranch( "/usr/local/bin", '/' )
     *    addBranch( "/usr/lib", '/' )
     *
     *  "usr"
     *		"lib"
     *  	"local"
     *			"bin"
     **/
    void addBranch( std::string		 content,
		    char 		 delimiter	= 0,
		    YPkgStringTreeItem * parent 	= 0 );


    /**
     * Construct a complete original (untranslated) path for the specified tree item.
     * 'startWithDelimiter' specifies whether or not the complete path should
     * start with the delimiter character.
     **/
    std::string completePath( const YPkgStringTreeItem * item,
			      char delimiter,
			      bool startWithDelimiter = true );
    
    /**
     * Debugging - dump the tree into the log file.
     **/
    void logTree();


    /**
     * Apply the filter criteria.
     * Derived classes should overwrite this.
     * The default implementation does nothing.
     *
     * TODO: return a useful value, e.g., a pkg list
     **/
    virtual void filter() {}


    /**
     * Returns the root of the filter view tree.
     * Note: In most cases, the root item itself will not contain any useful
     * information. Consider it the handle for the entire tree, not an actual
     * data element.
     **/
    YPkgStringTreeItem * root() const { return _root; }
    
protected:


    /**
     * Debugging - dump one branch of the tree into the log file.
     **/
    void logBranch( YPkgStringTreeItem * branch, std::string indentation );

    
    // Data members

    YPkgStringTreeItem * _root;
};




#endif // YPkgTreeFilterView_h
