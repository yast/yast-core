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
#include <y2util/YRpmGroupsTree.h>
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

// for Pkg::PkgPrepareOrder, PkgNextDelete, PkgNextInstall
static bool prepare_order_called = false;
static std::list<PMPackagePtr> packs_to_delete;
static std::list<PMPackagePtr>::const_iterator deleteIt;
static std::list<PMPackagePtr> packs_to_install;
static std::list<PMPackagePtr>::const_iterator installIt;

// ------------------------
/**   
   @builtin Pkg::PkgPrepareOrder () -> [ delcount, inscount ]
     prepare for PkgNextInstall, PkgNextDelete
 */
YCPValue
PkgModuleFunctions::PkgPrepareOrder (YCPList args)
{
    _y2pm.packageManager().getPackagesToInsDel (packs_to_delete, packs_to_install);
    deleteIt = packs_to_delete.begin();
    installIt = packs_to_install.begin();
    prepare_order_called = true;
    YCPList counts;
    counts->add (YCPInteger (packs_to_delete.size()));
    counts->add (YCPInteger (packs_to_install.size()));
    return counts;
}

// ------------------------
/**   
   @builtin Pkg::PkgNextDelete -> string
	nil on error
	"" on no-more-elements
	"name" else
 */
YCPValue
PkgModuleFunctions::PkgNextDelete (YCPList args)
{
    if (!prepare_order_called)
	return YCPError ("PkgNextDelete without prepare");

    if (deleteIt == packs_to_delete.end())
	return YCPString("");
    YCPString ret ((*deleteIt)->name());
    ++deleteIt;
    return ret;
}

// ------------------------
/**   
   @builtin Pkg::PkgNextInstall -> string
	nil on error
	"" on no-more-elements
	"name" else
 */
YCPValue
PkgModuleFunctions::PkgNextInstall (YCPList args)
{
    if (!prepare_order_called)
	return YCPError ("PkgNextInstall without prepare");

    if (installIt == packs_to_install.end())
	return YCPString("");
    YCPString ret ((*installIt)->name());
    ++installIt;
    return ret;
}



// ------------------------
/**   
   @builtin Pkg::GetGroups(string prefix) -> ["group1", "group2", ...]

   returns a list of strings containing all known RPM groups
   matching the given prefix<br>
   If the prefix is the empty string, all groups are returned
*/
YCPValue
PkgModuleFunctions::GetGroups (YCPList args)
{
    YCPList groups;
#warning Pkg::GetGroups still needed ?
//    const YRpmGroupsTree *groups_tree = _y2pm.packageManager().rpmGroupsTree();
    
    return groups;
}


// ------------------------
/**   
   @builtin Pkg::IsProvided (string tag) -> boolean

   returns a 'true' if the tag is provided in the installed system

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)
*/
YCPValue
PkgModuleFunctions::IsProvided (YCPList args)
{
    if ((args->size() != 1)
        || !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::IsProvided");
    }
#warning must check tags, not package names
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(args->value(0)->asString()->value());
    if (!selectable)
	return YCPBoolean (false);
    if (selectable->installedObj() == 0)
	return YCPBoolean (false);
    return YCPBoolean (true);
}

// ------------------------
/**   
   @builtin Pkg::IsSelected (string tag) -> boolean

   returns a 'true' if the tag is selected for installation

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)
*/
YCPValue
PkgModuleFunctions::IsSelected (YCPList args)
{
    if ((args->size() != 1)
        || !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::IsSelected");
    }
#warning must check tags, not package names
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(args->value(0)->asString()->value());
    if (!selectable)
	return YCPBoolean (false);
    if ((selectable->status() == PMSelectable::S_Install)
	|| (selectable->status() == PMSelectable::S_Update)
	|| (selectable->status() == PMSelectable::S_Auto))
    {
	return YCPBoolean (true);
    }
    return YCPBoolean (false);
}

// ------------------------
/**   
   @builtin Pkg::IsAvailable (string tag) -> boolean

   returns a 'true' if the tag is available on any of the currently
   active installation sources. (i.e. it is installable)

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)
*/
YCPValue
PkgModuleFunctions::IsAvailable (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::IsAvailable");
    }
#warning must check tags, not package names
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(args->value(0)->asString()->value());
    if (!selectable)
	return YCPBoolean (false);
    if (selectable->candidateObj() == 0)
	return YCPBoolean (false);
    return YCPBoolean (true);
}


// internal
bool
PkgModuleFunctions::DoProvideString (std::string name)
{
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	//y2error ("Provide: package '%s' not found", name.c_str());
	return false;
    }
    selectable->set_status (PMSelectable::S_Install);
    return true;
}

// ------------------------
/**   
   @builtin Pkg::DoProvide (list tags) -> $["failed1":"reason", ...]

   Provides (read: installs) a list of tags to the system

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)

   returns a map of tag,reason pairs if tags could not be provided.
   Usually this map should be empty (all required packages are
   installed)
   If tags could not be provided (due to package install failures or
   conflicts), the tag is listed as a key and the value describes
   the reason for the failure (as an already translated string).
*/
YCPValue
PkgModuleFunctions::DoProvide (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList()))
    {
	return YCPError ("Bad args to Pkg::DoProvide");
    }
    YCPMap ret;
    YCPList tags = args->value(0)->asList();
    if (tags->size() > 0)
    {
	for (int i = 0; i < tags->size(); ++i)
	{
	    if (tags->value(i)->isString())
	    {
		DoProvideString (tags->value(i)->asString()->value());
	    }
	    else
	    {
		y2error ("Pkg::DoProvide not string '%s'", tags->value(i)->toString().c_str());
	    }
	}
    }
    return ret;
}

// internal
bool
PkgModuleFunctions::DoRemoveString (std::string name)
{
    PMSelectablePtr selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	y2error ("Remove: package '%s' not found", name.c_str());
	return false;
    }
    selectable->set_status (PMSelectable::S_Del);
    return true;
}

// ------------------------
/**   
   @builtin Pkg::DoRemove (list tags) -> ["failed1", ...]

   Removes a list of tags from the system

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)

   returns a map of tag,reason pairs if tags could not be removed.
   Usually this map should be empty (all required packages are
   removed)
   If a tag could not be removed (because other packages still
   require it), the tag is listed as a key and the value describes
   the reason for the failure (as an already translated string).
*/
YCPValue
PkgModuleFunctions::DoRemove (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList()))
    {
	return YCPError ("Bad args to Pkg::DoRemove");
    }
    YCPMap ret;
    YCPList tags = args->value(0)->asList();
    if (tags->size() > 0)
    {
	for (int i = 0; i < tags->size(); ++i)
	{
	    if (tags->value(i)->isString())
	    {
		DoRemoveString (tags->value(i)->asString()->value());
	    }
	    else
	    {
		y2error ("Pkg::DoRemove not string '%s'", tags->value(i)->toString().c_str());
	    }
	}
    }
    return ret;
}

// ------------------------
/**   
   @builtin Pkg::PkgSummary (string package) -> "This is a nice package"

   Get summary (aka label) of a package

*/
YCPValue
PkgModuleFunctions::PkgSummary (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSummary");
    }
    string name = args->value(0)->asString()->value();
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
/**   
   @builtin Pkg::PkgVersion (string package) -> "1.42-39"

   Get version (better: edition) of a package

*/
YCPValue
PkgModuleFunctions::PkgVersion (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSummary");
    }
    string name = args->value(0)->asString()->value();
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
/**   
   @builtin Pkg::PkgSize (string package) -> 12345678

   Get (installed) size of a package

*/
YCPValue
PkgModuleFunctions::PkgSize (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgSize");
    }
    string name = args->value(0)->asString()->value();
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
/**   
   @builtin Pkg::PkgLocation (string package) -> string filename

   Get location of package

*/
YCPValue
PkgModuleFunctions::PkgLocation (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgLocation");
    }
    string name = args->value(0)->asString()->value();
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
    const std::string location = package->location();
    string::size_type archpos = location.find(' ');
    Pathname archpath;
    if (archpos != string::npos)
    {
	archpath = string (location, archpos+1);
	archpath = archpath + string (location, 0, archpos);
    }
    else
    {
	archpath = Pathname ((const std::string &)(package->arch()));
	archpath = archpath + location;
    }
    return YCPString (archpath.asString());
}

// ------------------------
/**   
   @builtin Pkg::PkgMediaNr (string package) -> integer

   Get media number of package location

*/
YCPValue
PkgModuleFunctions::PkgMediaNr (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::PkgMediaNr");
    }
    string name = args->value(0)->asString()->value();
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
    return YCPInteger (package->medianr());
}

// ------------------------
/**   
   @builtin Pkg::SaveState() -> bool

   save the current package selection status for later
   retrieval via Pkg::RestoreState()

*/
YCPValue
PkgModuleFunctions::SaveState (YCPList args)
{

    return YCPBoolean (true);
}

// ------------------------
/**   
   @builtin Pkg::RestoreState() -> bool

   restore the package selection status from a former
   call to Pkg::SaveState()
   Returns false if there is no saved state (no Pkg::SaveState()
   called before)

*/
YCPValue
PkgModuleFunctions::RestoreState (YCPList args)
{

    return YCPBoolean (true);
}

// ------------------------
/**   
   @builtin Pkg::IsManualSelection () -> bool

   return true if the original list of packages (since the
   last Pkg::SetSelection was changed.

*/
YCPValue
PkgModuleFunctions::IsManualSelection (YCPList args)
{

    return YCPBoolean (false);
}

