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

   File:	PkgModuleFunctionsTarget.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to InstTarget
		Handles target related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstTarget.h>
#include <y2pm/UpdateInfParser.h>
#include <y2pm/PMPackageManager.h>
#include <y2pm/PMError.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

#include <unistd.h>
#include <sys/statvfs.h>

using std::string;

/** ------------------------
 *
 * @builtin Pkg::TargetInit(string root, bool new) -> bool
 *
 * initialized target system with root-directory
 * if new == true, initialize new rpm database
 */
YCPValue
PkgModuleFunctions::TargetInit (const YCPString& root, const YCPBoolean& n)
{
#warning TargetInit: newdb flag is obsolete! Check!
    bool newdb = n->value();		// used again below

    Pathname newRoot = root->value();

    if (newdb)
    {
#if 0
        // create empty rpmdb
        _last_error = _y2pm.instTarget().init (Pathname (args->value(0)->asString()->value()), newdb);
#endif
        // Initialize target. If package/selecion manager do already exist
        // data are loaded, otherwise when the manager is created.
        _last_error = _y2pm.instTargetInit( newRoot );
    }
    else
    {
#if 0
        _last_error = PMError::E_ok;
        // use existing rpmdb (and seldb !)
        _y2pm.instTarget(true, Pathname (args->value(0)->asString()->value()));
#endif
        // initialize target
        _last_error = _y2pm.instTargetInit( newRoot );

        if ( !_last_error ) {
          // assert package/selecion managers exist and are up to date.
          _last_error = _y2pm.instTargetUpdate();
        }

    }

    if (_last_error)
        return YCPError (_last_error.errstr().c_str(), YCPBoolean (false));


    return YCPBoolean (true);
}

/** ------------------------
 *
 * @builtin Pkg::TargetFinish() -> bool
 *
 * finish target usage
 */
YCPBoolean
PkgModuleFunctions::TargetFinish ()
{
    _y2pm.instTargetClose();
    return YCPBoolean (true);
}

/** ------------------------
 *
 * @builtin Pkg::TargetInstall(string filename) -> bool
 *
 * install rpm package by filename
 * the filename must be an absolute path to a file which can
 * be accessed by the package manager.
 *
 * !! uses callbacks !!
 * You should do an 'import "PackageCallbacks"' before !
 */
YCPBoolean
PkgModuleFunctions::TargetInstall(const YCPString& filename)
{
    _last_error = _y2pm.installFile (Pathname (filename->value()));
    return YCPBoolean (!_last_error);
}


/** ------------------------
 *
 * @builtin Pkg::TargetRemove(string name) -> bool
 *
 * install package by name
 * !! uses callbacks !!
 * You should do an 'import "PackageCallbacks"' before !
 */
YCPBoolean
PkgModuleFunctions::TargetRemove(const YCPString& name)
{
    _y2pm.removePackage (name->value());
    return YCPBoolean (true);
}


/** ------------------------
 *
 * @builtin Pkg::TargetLogfile (string name) -> bool
 *
 * init logfile for target
 */
YCPBoolean
PkgModuleFunctions::TargetLogfile (const YCPString& name)
{
    return YCPBoolean (_y2pm.instTarget().setInstallationLogfile (name->value()));
}


/** ------------------------
 * INTERNAL
 * get_disk_stats
 *
 * return capacity and usage of partition at directory
 */
static void
get_disk_stats (const char *fs, long long *used, long long *size, long long *bsize)
{
    struct statvfs sb;
    if (statvfs (fs, &sb) < 0)
    {
	*used = *size = -1;
	return;
    }
    *bsize = sb.f_frsize ? : sb.f_bsize;		// block size
    *size = sb.f_blocks * *bsize;			// total size
    *used = (sb.f_blocks - sb.f_bfree) * *bsize;	// used size
}


/** ------------------------
 *
 * @builtin Pkg::TargetCapacity (string dir) -> integer
 *
 * return capacity of partition at directory
 */
YCPInteger
PkgModuleFunctions::TargetCapacity (const YCPString& dir)
{
    long long used, size, bsize;
    get_disk_stats (dir->value().c_str(), &used, &size, &bsize);

    return YCPInteger (size);
}

/** ------------------------
 *
 * @builtin Pkg::TargetUsed (string dir) -> integer
 *
 * return usage of partition at directory
 */
YCPInteger
PkgModuleFunctions::TargetUsed (const YCPString& dir)
{
    long long used, size, bsize;
    get_disk_stats (dir->value().c_str(), &used, &size, &bsize);

    return YCPInteger (used);
}

/** ------------------------
 *
 * @builtin Pkg::TargetBlockSize (string dir) -> integer
 *
 * return block size of partition at directory
 */
YCPInteger
PkgModuleFunctions::TargetBlockSize (const YCPString& dir)
{
    long long used, size, bsize;
    get_disk_stats (dir->value().c_str(), &used, &size, &bsize);

    return YCPInteger (bsize);
}

/** ------------------------
 *
 * @builtin Pkg::TargetUpdateInf (string filename) -> map
 *
 * return content of update.inf (usually <destdir>/var/lib/YaST/update.inf)
 * as $[ "basesystem" : "blah", "distname" : "foo", "distversion" : "bar",
 *   "distrelease" : "baz", "ftppatch" : "ftp.suse.com:/pub/suse/i386/update/8.0.99",
 *   "ftpsources" : [ "ftp.suse.com:/pub/suse/i386/current", ... ]]
 */
YCPValue
PkgModuleFunctions::TargetUpdateInf (const YCPString& filename)
{
    UpdateInfParser parser;

    if (parser.fromPath (Pathname (filename->value())))
    {
	return YCPVoid();			// return nil on error
    }

    YCPMap retmap;
    retmap->add (YCPString ("basesystem"), YCPString (parser.basesystem()));
    retmap->add (YCPString ("distname"), YCPString (parser.distname()));
    retmap->add (YCPString ("distversion"), YCPString (parser.distversion()));
    retmap->add (YCPString ("distrelease"), YCPString (parser.distrelease()));
    retmap->add (YCPString ("ftppatch"), YCPString (parser.ftppatch()));
    YCPList ftplist;
    std::list<std::string> sources = parser.ftpsources();
    for (std::list<std::string>::iterator it = sources.begin();
	 it != sources.end(); ++it)
    {
	ftplist->add (YCPString(*it));
    }
    retmap->add (YCPString ("ftpsources"), ftplist);
    return retmap;
}

/** ------------------------
 *
 * @builtin Pkg::TargetProducts () -> list
 *
 * return list of maps of all installed products in reverse
 * installation order (product installed last comes first)
 */

YCPList
PkgModuleFunctions::TargetProducts ()
{
    YCPList prdlist;
    std::list<constInstSrcDescrPtr> products = _y2pm.instTarget().getProducts();
    for (std::list<constInstSrcDescrPtr>::const_iterator it = products.begin();
	 it != products.end(); ++it)
    {
	prdlist->add(Descr2Map (*it));
    }
    return prdlist;
}

/** ------------------------
 *
 * @builtin Pkg::TargetRebuildDB () -> bool
 *
 * call "rpm --rebuilddb"
 */

YCPBoolean
PkgModuleFunctions::TargetRebuildDB ()
{
    _y2pm.instTarget().bringIntoCleanState();
    return YCPBoolean (true);
}


/** ------------------------
 *
 * @builtin Pkg::TargetInitDU (list(map)) -> void
 *
 * init DU calculation for given directories
 * parameter: [ $["name":"dir-without-leading-slash",
 *                "free":int_free,
 *		  "used":int_used,
 *		  "readonly":bool] ]
 */
YCPValue
PkgModuleFunctions::TargetInitDU (const YCPList& dirlist)
{
    if (dirlist->size() == 0)
    {
	return YCPError ("Bad args to Pkg::TargetInitDU");
    }

    std::set<PkgDuMaster::MountPoint> mountpoints;

    for (int i = 0; i < dirlist->size(); ++i)
    {
	bool good = true;
	YCPMap partmap;
	std::string dname;
	long long dfree = 0LL;
	long long dused = 0LL;
	bool readonly = false;

	if (dirlist->value(i)->isMap())
	{
	    partmap = dirlist->value(i)->asMap();
	}
	else
	{
	   good = false;
	}

	if (good
	    && partmap->value(YCPString("name"))->isString())
	{
	    dname = partmap->value(YCPString("name"))->asString()->value();
	    if (dname[0] != '/')
		dname = string("/") + dname;
	}
	else
	{
	    good = false;
	}

	if (good
	    && partmap->value(YCPString("free"))->isInteger())
	{
	    dfree = partmap->value(YCPString("free"))->asInteger()->value();
	}
	else
	{
	    good = false;
	}

	if (good
	    && partmap->value(YCPString("used"))->isInteger())
	{
	    dused = partmap->value(YCPString("used"))->asInteger()->value();
	}
	else
	{
	    good = false;
	}

	if (good
	    && !partmap->value(YCPString("readonly")).isNull()
	    && partmap->value(YCPString("readonly"))->isBoolean())
	{
	    readonly = partmap->value(YCPString("readonly"))->asBoolean()->value();
	}
	// else: optional arg, using default

	if (!good)
	{
	    y2error ("TargetDUInit: bad item %d: %s", i, dirlist->value(i)->toString().c_str());
	    continue;
	}

	long long dirsize = dfree + dused;

	PkgDuMaster::MountPoint point (dname, FSize (4096), FSize (dirsize), FSize (dused), readonly);
	mountpoints.insert (point);
    }
    _y2pm.packageManager().setMountPoints(mountpoints);
    return YCPVoid();
}


/** ------------------------
 *
 * @builtin Pkg::TargetGetDU (void) -> map<string, list<integer> >
 *
 * return current DU calculations
 * $[ "dir" : [ total, used, pkgusage, readonly ], .... ]
 * total == total size for this partition
 * used == current used size on target
 * pkgusage == future used size on target based on current package selection
 * readonly == true/false telling whether the partition is mounted readonly
 */
YCPValue
PkgModuleFunctions::TargetGetDU ()
{
    YCPMap dirmap;
    std::set<PkgDuMaster::MountPoint> mountpoints = _y2pm.packageManager().updateDu().mountpoints();

    for (std::set<PkgDuMaster::MountPoint>::iterator it = mountpoints.begin();
	 it != mountpoints.end(); ++it)
    {
	YCPList sizelist;
	sizelist->add (YCPInteger ((long long)(it->total())));
	sizelist->add (YCPInteger ((long long)(it->initial_used())));
	sizelist->add (YCPInteger ((long long)(it->pkg_used())));
	sizelist->add (YCPInteger (it->readonly() ? 1 : 0));
	dirmap->add (YCPString (it->_mountpoint), sizelist);
    }
    return dirmap;
}



/**
   @builtin Pkg::TargetFileHasOwner  (string filepath) -> bool

   returns the (first) package
*/

YCPBoolean
PkgModuleFunctions::TargetFileHasOwner (const YCPString& filepath)
{
    return YCPBoolean (_y2pm.instTarget().hasFile(filepath->value()));
}

