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

   File:	PkgModule.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>

#include <PkgModule.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;


/**
 * Constructor.
 */
PkgModule::PkgModule ()
{
  // todo: init packagemanager
}

/**
 * Destructor.
 */
PkgModule::~PkgModule ()
{
  // todo: release packagemanager
}

/**
 * evaluate 'function (list-of-arguments)'
 * and return YCPValue
 */
YCPValue
PkgModule::evaluate (string function, YCPList args)
{
    YCPValue ret = YCPVoid();
    y2debug ("PkgModule::evaluate (%s, %s)", function.c_str(), args->toString().c_str());

    if (function == "GetGroups")
    {
	/**
	 * @builtin Pkg::GetGroups(string prefix) -> ["group1", "group2", ...]
	 *
	 * returns a list of strings containing all known RPM groups
	 * matching the given prefix<br>
	 * If the prefix is the empty string, all groups are returned
	 */

	YCPList groups;
	groups->add (YCPString ("System/YaST"));
	groups->add (YCPString ("Applications/Office"));
	groups->add (YCPString ("Development/C++"));
	groups->add (YCPString ("Network/WWW"));
	groups->add (YCPString ("X11/Games"));
	ret = groups;
    }
    else if ((function == "GetSelections")
	     && (args->size() == 1)
	     && (args->value(0)->isSymbol()))
    {
	/**
	 * @builtin Pkg::GetSelections (symbol which) -> ["sel1", "sel2", ...]
	 *
	 * returns a list of selection names matching the 'which' symbol
	 * which can be:<br>
	 * `all		: all known selections<br>
	 * `avail_base	: available base selections<br>
	 * `avail_addon	: available addon selections<br>
	 * `inst_base	: installed base selection (usually one !)<br>
	 * `inst_addon	: installed addon selections<br>
	 * 
	 */

	string which = args->value(0)->asSymbol()->symbol();
	YCPList selections;

	if (which == "all")
	{
	    selections->add (YCPString ("Standard"));
	    selections->add (YCPString ("Minimal"));
	    selections->add (YCPString ("Development"));
	    selections->add (YCPString ("Games"));
	    selections->add (YCPString ("Multimedia"));
	}
	else if (which == "avail_base")
	{
	    selections->add (YCPString ("Standard"));
	    selections->add (YCPString ("Minimal"));
	}
	else if (which == "inst_base")
	{
	    selections->add (YCPString ("Standard"));
	}
	else if (which == "avail_addon")
	{
	    selections->add (YCPString ("Development"));
	    selections->add (YCPString ("Games"));
	    selections->add (YCPString ("Multimedia"));
	}
	else if (which == "inst_addon")
	{
	    selections->add (YCPString ("Development"));
	    selections->add (YCPString ("Multimedia"));
	}
	ret = selections;
    }
    else if ((function == "IsProvided")
	     && (args->size() == 1)
	     && (args->value(0)->isString()))
    {
	/**
	 * @builtin Pkg::IsProvided (string tag) -> boolean
	 *
	 * returns a 'true' if the tag is provided in the installed system
	 *
	 * tag can be a package name, a string from requires/provides
	 * or a file name (since a package implictly provides all its files)
	 */

	ret = YCPBoolean (true);
    }
    else if ((function == "IsAvailable")
	     && (args->size() == 1)
	     && (args->value(0)->isString()))
    {
	/**
	 * @builtin Pkg::IsAvailable (string tag) -> boolean
	 *
	 * returns a 'true' if the tag is available on any of the currently
	 * active installation sources. (i.e. it is installable)
	 *
	 * tag can be a package name, a string from requires/provides
	 * or a file name (since a package implictly provides all its files)
	 */

	ret = YCPBoolean (false);
    }
    else if ((function == "DoProvide")
	     && (args->size() == 1)
	     && (args->value(0)->isList()))
    {
	/**
	 * @builtin Pkg::DoProvide (list tags) -> $["failed1":"reason", ...]
	 *
	 * Provides (read: installs) a list of tags to the system
	 *
	 * tag can be a package name, a string from requires/provides
	 * or a file name (since a package implictly provides all its files)
	 *
	 * returns a map of tag,reason pairs if tags could not be provided.
	 * Usually this map should be empty (all required packages are
	 * installed)
	 * If tags could not be provided (due to package install failures or
	 * conflicts), the tag is listed as a key and the value describes
	 * the reason for the failure (as an already translated string).
	 */

	ret = YCPMap ();
    }
    else if ((function == "DoRemove")
	     && (args->size() == 1)
	     && (args->value(0)->isList()))
    {
	/**
	 * @builtin Pkg::DoRemove (list tags) -> ["failed1", ...]
	 *
	 * Removes a list of tags from the system
	 *
	 * tag can be a package name, a string from requires/provides
	 * or a file name (since a package implictly provides all its files)
	 *
	 * returns a map of tag,reason pairs if tags could not be removed.
	 * Usually this map should be empty (all required packages are
	 * removed)
	 * If a tag could not be removed (because other packages still
	 * require it), the tag is listed as a key and the value describes
	 * the reason for the failure (as an already translated string).
	 */

	ret = YCPMap ();
    }
    else
    {
	ret = YCPError ("Undefined Pkg:: function or wrong arguments", YCPVoid());
    }

    return ret;
}

