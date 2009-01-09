/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						   (C) SuSE Linux GmbH |
\----------------------------------------------------------------------/

   File:	YCPBuiltinMultiset.cc
   Summary:	YCP Multiset Builtins

   Authors:	Arvin Schnell <aschnell@suse.de>
   Maintainer:	Arvin Schnell <aschnell@suse.de>

/-*/

#include <algorithm>

#include "ycp/YCPBuiltinMultiset.h"
#include "ycp/YCPList.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPCodeCompare.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
ms_includes(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/includes.html

    return YCPBoolean(includes(a->begin(), a->end(), b->begin(), b->end(), ycp_less()));
}


static YCPValue
ms_difference(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/set_difference.html

    YCPList ret;
    back_insert_iterator<YCPList> bii(ret);
    set_difference(a->begin(), a->end(), b->begin(), b->end(), bii, ycp_less());
    return ret;
}


static YCPValue
ms_symmetric_difference(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/set_symmetric_difference.html

    YCPList ret;
    back_insert_iterator<YCPList> bii(ret);
    set_symmetric_difference(a->begin(), a->end(), b->begin(), b->end(), bii, ycp_less());
    return ret;
}


static YCPValue
ms_intersection(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/set_intersection.html

    YCPList ret;
    back_insert_iterator<YCPList> bii(ret);
    set_intersection(a->begin(), a->end(), b->begin(), b->end(), bii, ycp_less());
    return ret;
}


static YCPValue
ms_union(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/set_union.html

    YCPList ret;
    back_insert_iterator<YCPList> bii(ret);
    set_union(a->begin(), a->end(), b->begin(), b->end(), bii, ycp_less());
    return ret;
}


static YCPValue
ms_merge(const YCPList& a, const YCPList& b)
{
    // see http://www.sgi.com/tech/stl/merge.html

    YCPList ret;
    back_insert_iterator<YCPList> bii(ret);
    merge(a->begin(), a->end(), b->begin(), b->end(), bii, ycp_less());
    return ret;
}


YCPBuiltinMultiset::YCPBuiltinMultiset()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations_ns[] = {
	{ "multiset", "",									NULL, DECL_NAMESPACE },
	{ "includes",              "boolean (const list <flex>, const list <flex>)",		(void*) ms_includes, DECL_FLEX },
	{ "difference",            "list <flex> (const list <flex>, const list <flex>)",	(void*) ms_difference, DECL_FLEX },
	{ "symmetric_difference",  "list <flex> (const list <flex>, const list <flex>)",	(void*) ms_symmetric_difference, DECL_FLEX },
	{ "intersection",          "list <flex> (const list <flex>, const list <flex>)",	(void*) ms_intersection, DECL_FLEX },
	{ "union",                 "list <flex> (const list <flex>, const list <flex>)",	(void*) ms_union, DECL_FLEX },
	{ "merge",                 "list <flex> (const list <flex>, const list <flex>)",	(void*) ms_merge, DECL_FLEX },
	{ 0 }
    };

    static_declarations.registerDeclarations("YCPBuiltinMultiset", declarations_ns);
}
