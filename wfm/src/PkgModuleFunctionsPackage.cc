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

   File:	PkgModuleFunctionsPackage.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles package related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstData.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;

// ------------------------
// 
// @builtin Pkg::GetGroups(string prefix) -> ["group1", "group2", ...]
//
// returns a list of strings containing all known RPM groups
// matching the given prefix<br>
// If the prefix is the empty string, all groups are returned

YCPValue
PkgModuleFunctions::GetGroups (YCPList args)
{
    YCPList groups;
    groups->add (YCPString ("System/YaST"));
    groups->add (YCPString ("Applications/Office"));
    groups->add (YCPString ("Development/C++"));
    groups->add (YCPString ("Network/WWW"));
    groups->add (YCPString ("X11/Games"));
    return groups;
}


// ------------------------
// 
// @builtin Pkg::IsProvided (string tag) -> boolean
//
// returns a 'true' if the tag is provided in the installed system
//
// tag can be a package name, a string from requires/provides
// or a file name (since a package implictly provides all its files)
YCPValue
PkgModuleFunctions::IsProvided (YCPList args)
{
    if ((args->size() != 1)
        || !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::IsProvided");
    }

    return YCPBoolean (true);
}

// ------------------------
// 
// @builtin Pkg::IsAvailable (string tag) -> boolean
//
// returns a 'true' if the tag is available on any of the currently
// active installation sources. (i.e. it is installable)
//
// tag can be a package name, a string from requires/provides
// or a file name (since a package implictly provides all its files)
YCPValue
PkgModuleFunctions::IsAvailable (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::IsAvailable");
    }
    string pkgname = args->value(0)->asString()->value_cstr();
    PMError err = _y2pm.instTarget().init (false);
    if (err)
    {
	return YCPError (err.errstr());
    }
    const std::list<PMPackagePtr> pkglist = _y2pm.instTarget().getPackages ();
    const std::list<PMPackagePtr> matches = InstData::findPackages (pkglist, pkgname);
    return YCPBoolean (matches.size() > 0);
}

// ------------------------
// 
// @builtin Pkg::DoProvide (list tags) -> $["failed1":"reason", ...]
//
// Provides (read: installs) a list of tags to the system
//
// tag can be a package name, a string from requires/provides
// or a file name (since a package implictly provides all its files)
//
// returns a map of tag,reason pairs if tags could not be provided.
// Usually this map should be empty (all required packages are
// installed)
// If tags could not be provided (due to package install failures or
// conflicts), the tag is listed as a key and the value describes
// the reason for the failure (as an already translated string).
YCPValue
PkgModuleFunctions::DoProvide (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList()))
    {
	return YCPError ("Bad args to Pkg::DoProvide");
    }

    return YCPMap ();
}

// ------------------------
// 
// @builtin Pkg::DoRemove (list tags) -> ["failed1", ...]
//
// Removes a list of tags from the system
//
// tag can be a package name, a string from requires/provides
// or a file name (since a package implictly provides all its files)
//
// returns a map of tag,reason pairs if tags could not be removed.
// Usually this map should be empty (all required packages are
// removed)
// If a tag could not be removed (because other packages still
// require it), the tag is listed as a key and the value describes
// the reason for the failure (as an already translated string).
YCPValue
PkgModuleFunctions::DoRemove (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList()))
    {
	return YCPError ("Bad args to Pkg::DoRemove");
    }

    return YCPMap ();
}

// ------------------------
// 
// @builtin Pkg::PkgSummary (string package) -> "This is a nice package"
//
// Get summary (aka label) of a package
//
YCPValue
PkgModuleFunctions::PkgSummary (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSummary");
    }
    string name = args->value(0)->asString()->value();
    y2milestone ("looking up (%s)", name.c_str());
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	return YCPError ("Package '"+name+"' not found");
    }
    PMPackagePtr package = selectable->theObject();
    if (!package)
    {
	return YCPError ("Package '"+name+"' no object");
    }

    return YCPString (package->summary());
}

// ------------------------
// 
// @builtin Pkg::PkgVersion (string package) -> "1.42-39"
//
// Get version (better: edition) of a package
//
YCPValue
PkgModuleFunctions::PkgVersion (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSummary");
    }
    string name = args->value(0)->asString()->value();
    y2milestone ("looking up (%s)", name.c_str());
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	return YCPError ("Package '"+name+"' not found");
    }
    PMPackagePtr package = selectable->theObject();
    if (!package)
    {
	return YCPError ("Package '"+name+"' no object");
    }

    return YCPString (package->edition().as_string());
}

// ------------------------
// 
// @builtin Pkg::PkgSize (string package) -> 12345678
//
// Get (installed) size of a package
//
YCPValue
PkgModuleFunctions::PkgSize (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSummary");
    }
    string name = args->value(0)->asString()->value();
    y2milestone ("looking up (%s)", name.c_str());
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	return YCPError ("Package '"+name+"' not found");
    }
    PMPackagePtr package = selectable->theObject();
    if (!package)
    {
	return YCPError ("Package '"+name+"' no object");
    }

    return YCPInteger ((long long)(package->size()));
}

// ------------------------
// 
// @builtin Pkg::SaveState() -> bool
//
// save the current package selection status for later
// retrieval via Pkg::RestoreState()
//
YCPValue
PkgModuleFunctions::SaveState (YCPList args)
{

    return YCPBoolean (true);
}

// ------------------------
// 
// @builtin Pkg::RestoreState() -> bool
//
// restore the package selection status from a former
// call to Pkg::SaveState()
// Returns false if there is no saved state (no Pkg::SaveState()
// called before)
//
YCPValue
PkgModuleFunctions::RestoreState (YCPList args)
{

    return YCPBoolean (true);
}

// ------------------------
// 
// @builtin Pkg::IsManualSelection () -> bool
//
// return true if the original list of packages (since the
// last Pkg::SetSelection was changed.
//
YCPValue
PkgModuleFunctions::IsManualSelection (YCPList args)
{

    return YCPBoolean (false);
}

