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
#include <y2util/FSize.h>
#include <y2pm/InstData.h>
#include <y2pm/PMObject.h>
#include <y2pm/PMSelectable.h>

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

/**
 * helper function, get name from args
 */
static std::string
getName (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return "";
    }
    return args->value(0)->asString()->value();
}

/**
 * helper function, get selectable by name
 */

PMSelectablePtr
PkgModuleFunctions::getPackageSelectable (const std::string& name)
{
    PMSelectablePtr selectable;
    if (!name.empty())
	selectable = _y2pm.packageManager().getItem(name);
    if (!selectable)
    {
	y2error ("Package '%s' not found", name.c_str());
    }
    return selectable;
}


static PMPackagePtr
getTheObject (PMSelectablePtr selectable)
{
    PMPackagePtr package;
    if (selectable)
    {
	package = selectable->theObject();
	if (!package)
	{
	    y2error ("Package '%s' not found", ((const std::string&)(selectable->name())).c_str());
	}
    }
    return package;
}

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

/**   
   @builtin Pkg::PkgMediaSizes () -> [ media_1_size, media_2_size, ...]
     return cumulated sizes (in kb !) to be installed from different media

     Returns the install size per media, not the archivesize !!
 */
YCPValue
PkgModuleFunctions::PkgMediaSizes (YCPList args)
{
    vector<FSize> mediasizes;

    for (PMPackageManager::PMSelectableVec::const_iterator it = _y2pm.packageManager().begin();
	 it != _y2pm.packageManager().end(); ++it)
    {
	if (!((*it)->to_install()))
	    continue;

	PMPackagePtr package = (*it)->candidateObj();
	if (!package)
	{
	    continue;
	}

	unsigned int medianr = package->medianr();
	FSize size = package->size();
	if (medianr > mediasizes.size())
	{
	    mediasizes.resize (medianr);
	}
	mediasizes[medianr-1] += size;
    }

    YCPList ycpsizes;
    for (unsigned int i = 0; i < mediasizes.size(); ++i)
    {
	ycpsizes->add (YCPInteger (((long long)mediasizes[i]) / 1024LL));
    }
    return ycpsizes;
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
   @builtin Pkg::IsProvided (string tag) -> boolean

   returns a 'true' if the tag is provided in the installed system

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)
*/
YCPValue
PkgModuleFunctions::IsProvided (YCPList args)
{
#warning must check tags, not package names
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
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
#warning must check tags, not package names
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
    if (!selectable)
	return YCPBoolean (false);
    if (selectable->to_install())
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
#warning must check tags, not package names
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
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
    PMSelectablePtr selectable = getPackageSelectable(name);
    if (!selectable)
    {
	//y2error ("Provide: package '%s' not found", name.c_str());
	return false;
    }
    selectable->appl_set_install();
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
    PMSelectablePtr selectable = getPackageSelectable(name);
    if (!selectable)
    {
	y2error ("Remove: package '%s' not found", name.c_str());
	return false;
    }
    selectable->appl_set_delete();
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
    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
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
    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
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
    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
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
    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
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
    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    {
	return YCPVoid();
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

// ------------------------

/* helper functions */
static void
pgk2list (YCPList &list, const PMObjectPtr& package, bool names_only)
{
    if (names_only)
    {
	list->add (YCPString (package->name()));
    }
    else
    {
	string fullname = package->name();
	fullname += (" " + package->version());
	fullname += (" " + package->release());
	fullname += (" " + (const std::string &)(package->arch()));
	list->add (YCPString (fullname));
    }
    return;
}

/**   
   @builtin Pkg::GetPackages(symbol which, bool names_only) -> list of strings

   return list of packages (["pkg1", "pkg2", ..."] if names_only==true,
    ["pkg1 version release arch", "pkg1 version release arch", ... if
    names_only == false]

   'which' defines which packages are returned:

   `installed	all installed packages
   `selected	all selected but not yet installed packages
   `available	all available packeges (from the installation source)

*/

YCPValue
PkgModuleFunctions::GetPackages(YCPList args)
{
    if ((args->size() != 2)
	|| !(args->value(0)->isSymbol())
	|| !(args->value(1)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::GetPackages");
    }

    string which = args->value(0)->asSymbol()->symbol();
    bool names_only = args->value(1)->asBoolean()->value();

    YCPList packages;

    for (PMManager::PMSelectableVec::const_iterator it = Y2PM::packageManager().begin();
	 it != Y2PM::packageManager().end();
	 ++it)
    {
	PMObjectPtr package;
	if (which == "installed")
	{
	    package = (*it)->installedObj();
	}
	else if (which == "selected")
	{
	    if (!((*it)->to_install()))
	    {
		continue;
	    }
	    package = (*it)->candidateObj();
	}
	else if (which == "available")
	{
	    for (PMSelectable::PMObjectList::const_iterator ait = (*it)->av_begin();
		 ait != (*it)->av_end(); ++ait)
	    {
		pgk2list (packages, (*ait), names_only);
	    }
	    continue;
	}
	else
	{
	    return YCPError ("Wrong parameter for Pkg::GetPackages");
	}

	if (package)
	    pgk2list (packages, package, names_only);
    }

    return packages;
}


/**
 * @builtin PkgUpdateAll (bool only_newer) -> count
 *
 * mark all packages for installation which are installed and have
 * an available candidate. if 'only_newer' == true, only affect
 * packages which are newer, else affect all packages
 *
 * @return number of packages affected
 *
 * This will mark packages for installation *and* for deletion (if a
 * package provides/obsoletes another package)
 */

YCPValue
PkgModuleFunctions::PkgUpdateAll (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::PkgUpdateAll");
    }

    bool only_newer = args->value(0)->asBoolean()->value();

    return YCPInteger (_y2pm.packageManager().updateAllInstalled(only_newer));
}


/**   
   @builtin Pkg::PkgInstall (string package) -> boolean

   Select package for installation

*/
YCPValue
PkgModuleFunctions::PkgInstall (YCPList args)
{
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
    if (!selectable)
    {
	return YCPBoolean (false);
    }
    return YCPBoolean (selectable->user_set_install());
}


/**   
   @builtin Pkg::PkgDelete (string package) -> boolean

   Select package for deletion

*/
YCPValue
PkgModuleFunctions::PkgDelete (YCPList args)
{
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
    if (!selectable)
    {
	return YCPBoolean (false);
    }
    return YCPBoolean (selectable->user_set_delete());
}


/**   
   @builtin Pkg::PkgSolve () -> boolean

   Solve package dependencies

*/
YCPValue
PkgModuleFunctions::PkgSolve (YCPList args)
{
    return YCPBoolean (true);
}


/**   
   @builtin Pkg::PkgCommit () -> boolean

   Commit package changes (actually install/delete packages) 

*/
YCPValue
PkgModuleFunctions::PkgCommit (YCPList args)
{
    return YCPBoolean (false);
}

