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

  File:	      YPkgRpmGroupTagsFilterView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YPkgRpmGroupTagsFilterView_h
#define YPkgRpmGroupTagsFilterView_h


#include "YPkgTreeFilterView.h"


/**
 * @short Filter view for RPM group tags
 **/
class YPkgRpmGroupTagsFilterView: public YPkgTreeFilterView
{
public:

    /**
     * Constructor.
     **/

    YPkgRpmGroupTagsFilterView();

    /**
     * Destructor.
     **/
    virtual ~YPkgRpmGroupTagsFilterView();


    /**
     * Apply the filter criteria.
     * Derived classes should overwrite this.
     * The default implementation does nothing.
     *
     * TODO: return a useful value, e.g., a pkg list
     **/
    virtual void filter() {}

    
protected:
    
    /**
     * Add all RPM group tags to this filter
     **/
    void addRpmGroupTags();
};




#endif // YPkgRpmGroupTagsFilterView_h
