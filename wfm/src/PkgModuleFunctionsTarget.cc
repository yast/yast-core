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

#include <y2pm/PMError.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

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
PkgModuleFunctions::TargetInit (YCPList args)
{
    if ((args->size() != 2)
	|| !(args->value(0)->isString())
	|| !(args->value(1)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::TargetInit");
    }

    bool newdb = args->value(1)->asBoolean()->value();		// used again below

    _last_error = _y2pm.instTarget().init (Pathname (args->value(0)->asString()->value()), newdb);
    if (_last_error)
	return YCPError (_last_error.errstr().c_str(), YCPBoolean (false));

    // propagate installed packages if not newly created
    if (!newdb)
	_y2pm.packageManager().poolSetInstalled (_y2pm.instTarget().getPackages () );

    return YCPBoolean (true);
}

/** ------------------------
 * 
 * @builtin Pkg::TargetFinish() -> bool
 *
 * finish target usage
 */
YCPValue
PkgModuleFunctions::TargetFinish (YCPList args)
{
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
YCPValue
PkgModuleFunctions::TargetInstall(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetInstall");
    }
    _last_error = _y2pm.installFile (Pathname (args->value(0)->asString()->value()));
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
YCPValue
PkgModuleFunctions::TargetRemove(YCPList args)
{
    _y2pm.removePackage (args->value(0)->asString()->value());
    return YCPBoolean (true);
}


/** ------------------------
 * 
 * @builtin Pkg::TargetLogfile (string name) -> bool
 *
 * init logfile for target
 */
YCPValue
PkgModuleFunctions::TargetLogfile (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetLogfile");
    }
    return YCPBoolean (_y2pm.instTarget().setInstallationLogfile (args->value(0)->asString()->value()));
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
    *used = (sb.f_blocks - sb.f_bfree) * *bsize;	// free size
}


/** ------------------------
 * 
 * @builtin Pkg::TargetCapacity (string dir) -> integer
 *
 * return capacity of partition at directory
 */
YCPValue
PkgModuleFunctions::TargetCapacity (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetCapacity");
    }

    long long used, size, bsize;
    get_disk_stats (args->value(0)->asString()->value().c_str(), &used, &size, &bsize);

    return YCPInteger (size);
}

/** ------------------------
 * 
 * @builtin Pkg::TargetUsed (string dir) -> integer
 *
 * return usage of partition at directory
 */
YCPValue
PkgModuleFunctions::TargetUsed (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetUsed");
    }

    long long used, size, bsize;
    get_disk_stats (args->value(0)->asString()->value().c_str(), &used, &size, &bsize);

    return YCPInteger (used);
}

/** ------------------------
 * 
 * @builtin Pkg::TargetBlockSize (string dir) -> integer
 *
 * return block size of partition at directory
 */
YCPValue
PkgModuleFunctions::TargetBlockSize (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetBlockSize");
    }

    long long used, size, bsize;
    get_disk_stats (args->value(0)->asString()->value().c_str(), &used, &size, &bsize);

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
PkgModuleFunctions::TargetUpdateInf (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetUpdateInf");
    }
    UpdateInfParser parser;

    if (parser.fromPath (Pathname (args->value(0)->asString()->value())))
    {
	return YCPVoid();
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

YCPValue
PkgModuleFunctions::TargetProducts (YCPList args)
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

YCPValue
PkgModuleFunctions::TargetRebuildDB (YCPList args)
{
    _y2pm.instTarget().bringIntoCleanState();
    return YCPBoolean (true);
}


/** ------------------------
 * 
 * @builtin Pkg::DUInit (list string) -> void
 *
 * return init DU calculation for given directories
 */
YCPValue
PkgModuleFunctions::TargetDUInit (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList())
	|| (args->value(0)->asList()->size() == 0))
    {
	return YCPError ("Bad args to Pkg::TargetDUInit");
    }

    std::set<PkgDuMaster::MountPoint> mountpoints;

    YCPList dirlist = args->value(0)->asList();
    for (int i = 0; i < dirlist->size(); ++i)
    {
	if (!dirlist->value(i)->isString())
	{
	    y2error ("TargetDUInit: bad item %d: %s", i, dirlist->value(i)->toString().c_str());
	    continue;
	}

	string dir = dirlist->value(i)->asString()->value();

	long long used, size, bsize;
	get_disk_stats (dir.c_str(), &used, &size, &bsize);

	y2milestone ("dir %s, bsize %lld, total %lld, used %lld", dir.c_str(), bsize, size, used);

	PkgDuMaster::MountPoint point (dir, FSize (bsize), FSize (size), FSize (used));
	mountpoints.insert (point);
    }
    _y2pm.packageManager().setMountPoints(mountpoints);
    return YCPVoid();
}


