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
#include <y2pm/InstSrcDescr.h>
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

/**
 * helper function, get name from args
 */
static std::string
getName (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	y2error ("Pkg:: expects a string");
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
    _y2pm.packageManager ();
    startCachedSources (true);

    PMSelectablePtr selectable;
    if (!name.empty())
	selectable = _y2pm.packageManager().getItem(name);
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

#if 0 // currently unused
static PMPackagePtr
getTheCandidate (PMSelectablePtr selectable)
{
    PMPackagePtr package;
    if (selectable)
    {
	package = selectable->candidateObj();
	if (!package)
	{
	    y2error ("Package '%s' not found", ((const std::string&)(selectable->name())).c_str());
	}
    }
    return package;
}
#endif
// ------------------------
/**
 *  @builtin Pkg::PkgMediaSizes () -> [ media_1_size, media_2_size, ...]
 *    return cumulated sizes (in kb !) to be installed from different media
 *
 *   Returns the install size per media, not the archivesize !!
 */
YCPValue
PkgModuleFunctions::PkgMediaSizes (YCPList args)
{
    // compute max number of media across all sources

    unsigned int mediacount = 0;

    for (vector<InstSrcManager::ISrcId>::const_iterator it = _sources.begin();
	 it != _sources.end(); ++it)
    {
	if (*it == 0) 
	    continue;

	unsigned int count = (*it)->descr()->media_count();
	if (count > mediacount)
	    mediacount = count;
    }
    y2milestone ("Max media count %d", mediacount);

    vector<FSize> mediasizes(mediacount);

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
	    y2error ("resize needed %d", medianr);
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
   @builtin Pkg::IsProvided (string tag) -> boolean

   returns a 'true' if the tag is provided in the installed system

   tag can be a package name, a string from requires/provides
   or a file name (since a package implictly provides all its files)
*/
YCPValue
PkgModuleFunctions::IsProvided (YCPList args)
{
    // check package name first, then tag
    if (!_y2pm.instTarget().isInstalled (getName(args)))
	return YCPBoolean (_y2pm.instTarget().isProvided (getName(args)));
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
    startCachedSources (true);		// start cached sources

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

    startCachedSources (true);

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

    _y2pm.packageManager ();

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
    _y2pm.packageManager ();
    startCachedSources (true);

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
    _y2pm.packageManager ();
    startCachedSources (true);

    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
    }

    return YCPString (package->edition().asString());
}

// ------------------------
/**
   @builtin Pkg::PkgSize (string package) -> 12345678

   Get (installed) size of a package

*/
YCPValue
PkgModuleFunctions::PkgSize (YCPList args)
{
    _y2pm.packageManager ();
    startCachedSources (true);

    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
    }

    return YCPInteger ((long long)(package->size()));
}

// ------------------------
/**
   @builtin Pkg::PkgGroup (string package) -> string

   Get rpm group of a package

*/
YCPValue
PkgModuleFunctions::PkgGroup (YCPList args)
{
    _y2pm.packageManager ();
    startCachedSources (true);

    PMPackagePtr package = getTheObject (getPackageSelectable (getName(args)));
    if (!package)
    {
	return YCPVoid();
    }

    return YCPString (package->group());
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
   last Pkg::SetSelection()) was changed.

*/
YCPValue
PkgModuleFunctions::IsManualSelection (YCPList args)
{
    return YCPBoolean (_y2pm.packageManager().anythingByUser());
}

// ------------------------
/**
   @builtin Pkg::PkgAnyToDelete () -> bool

   return true if any packages are to be deleted

*/
YCPValue
PkgModuleFunctions::PkgAnyToDelete (YCPList args)
{
    return YCPBoolean (_y2pm.packageManager().anythingToDelete ());
}

// ------------------------
/**
   @builtin Pkg::AnyToInstall () -> bool

   return true if any packages are to be installed

*/
YCPValue
PkgModuleFunctions::PkgAnyToInstall (YCPList args)
{
    return YCPBoolean (_y2pm.packageManager().anythingToInstall ());
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
   `available	all available packages (from the installation source)

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

    _y2pm.packageManager ();
    startCachedSources (true);

    string which = args->value(0)->asSymbol()->symbol();
    bool names_only = args->value(1)->asBoolean()->value();

    YCPList packages;

    for (PMManager::PMSelectableVec::const_iterator it = _y2pm.packageManager().begin();
	 it != _y2pm.packageManager().end();
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
 * @builtin PkgUpdateAll (bool delete_unmaintained) -> [ integer affected, integer unknown ]
 *
 * mark all packages for installation which are installed and have
 * an available candidate.
 *
 * @return [ integer affected, integer unknown ]
 *
 * This will mark packages for installation *and* for deletion (if a
 * package provides/obsoletes another package)
 *
 * !!! DOES NOT SOLVE !!
 */

YCPValue
PkgModuleFunctions::PkgUpdateAll (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::GetPackages");
    }

    PMUpdateStats stats;
    stats.delete_unmaintained = args->value(0)->asBoolean()->value();
    _y2pm.packageManager().doUpdate (stats);

    YCPList ret;
    ret->add (YCPInteger ((long long)stats.totalToInstall()));
    if (stats.delete_unmaintained)
    {
	ret->add (YCPInteger ((long long)_y2pm.packageManager().updateSize() - stats.chk_dropped));
    }
    else
    {
	ret->add (YCPInteger ((long long)_y2pm.packageManager().updateSize()));
    }

    return ret;
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
   @builtin Pkg::PkgNeutral (string package) -> boolean

   Set package to neutral (drop install/delete flags)

*/
YCPValue
PkgModuleFunctions::PkgNeutral (YCPList args)
{
    PMSelectablePtr selectable = getPackageSelectable (getName(args));
    if (!selectable)
    {
	return YCPBoolean (false);
    }
    selectable->setNothingSelected();
    return YCPBoolean (true);
}


/**
   @builtin Pkg::PkgSolve () -> boolean
   Optional: Pkg::PkgSolve (true) to filter all conflicts with installed packages
	(installed packages will be preferred)

   Solve current package dependencies

*/
YCPValue
PkgModuleFunctions::PkgSolve (YCPList args)
{
    bool filter_conflicts_with_installed = false;
    if (args->size() > 0)
    {
	if (args->value(0)->isBoolean())
	{
	    filter_conflicts_with_installed = args->value(0)->asBoolean()->value();
	}
	else
	{
	    return YCPError ("Bad args to Pkg::PkgSolve");
	}
    }

    PkgDep::ResultList good;
    PkgDep::ErrorResultList bad;

    if (!_y2pm.packageManager().solveInstall(good, bad, filter_conflicts_with_installed))
    {
	_solve_errors = bad.size();
	y2error ("%d packages failed:", bad.size());

	std::ofstream out ("/var/log/YaST2/badlist");
	out << bad.size() << " packages failed" << std::endl;
	for( PkgDep::ErrorResultList::const_iterator p = bad.begin();
	     p != bad.end(); ++p )
	{
	    out << *p << std::endl;
	}

	return YCPBoolean (false);
    }
    return YCPBoolean (true);
}


/**
   @builtin Pkg::PkgSolveErrors() -> integer
   return number of fails
*/
YCPValue
PkgModuleFunctions::PkgSolveErrors(YCPList args)
{
    return YCPInteger (_solve_errors);
}

/**
   @builtin Pkg::PkgCommit (integer medianr) -> [ int successful, list failed, list remaining, list srcremaining ]

   Commit package changes (actually install/delete packages)

   if medianr == 0, all packages regardless of media are installed
   if medianr > 0, only packages from this media are installed

*/
YCPValue
PkgModuleFunctions::PkgCommit (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isInteger())
	|| (args->value(0)->asInteger()->value() < 0))
    {
	return YCPError ("Bad args to Pkg::PkgCommit");
    }
    unsigned medianr = args->value(0)->asInteger()->value();

    std::list<std::string> errors;
    std::list<std::string> remaining;
    std::list<std::string> srcremaining;
    int count = _y2pm.commitPackages (medianr, errors, remaining, srcremaining);

    YCPList ret;
    ret->add (YCPInteger (count));

    YCPList errlist;
    for (std::list<std::string>::const_iterator it = errors.begin(); it != errors.end(); ++it)
    {
	errlist->add (YCPString (*it));
    }
    ret->add (errlist);
    YCPList remlist;
    for (std::list<std::string>::const_iterator it = remaining.begin(); it != remaining.end(); ++it)
    {
	remlist->add (YCPString (*it));
    }
    ret->add (remlist);
    YCPList srclist;
    for (std::list<std::string>::const_iterator it = srcremaining.begin(); it != srcremaining.end(); ++it)
    {
	srclist->add (YCPString (*it));
    }
    ret->add (srclist);
    return ret;
}

/**
   @builtin Pkg::GetBackupPath () -> string

   get current path for update backup of rpm config files
*/

YCPValue
PkgModuleFunctions::GetBackupPath (YCPList args)
{
    return YCPString (_y2pm.instTarget().getBackupPath().asString());
}

/**
   @builtin Pkg::SetBackupPath (string path) -> void

   set current path for update backup of rpm config files
*/
YCPValue
PkgModuleFunctions::SetBackupPath (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetBackupPath");
    }
    Pathname path (args->value(0)->asString()->value());
    _y2pm.instTarget().setBackupPath (path);
    return YCPVoid();
}


/**
   @builtin Pkg::CreateBackups  (boolean flag) -> void

   whether to create package backups during install or removal
*/
YCPValue
PkgModuleFunctions::CreateBackups (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::CreateBackups");
    }
    _y2pm.instTarget().createPackageBackups(args->value(0)->asBoolean()->value());
    return YCPVoid();
}


