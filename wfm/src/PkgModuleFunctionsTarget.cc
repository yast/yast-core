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
    bool newdb = args->value(1)->asBoolean()->value();
    PMError err = _y2pm.instTarget().init (Pathname (args->value(0)->asString()->value()), newdb);
    if (err)
	return YCPError (err.errstr().c_str(), YCPBoolean (false));

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
 * @builtin Pkg::TargetInstall(string name) -> bool
 *
 * install package by name
 */
YCPValue
PkgModuleFunctions::TargetInstall(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::TargetInstall");
    }
    PMError err = _y2pm.instTarget().installPackage (args->value(0)->asString()->value());
    return YCPBoolean (!err);
}


/** ------------------------
 * 
 * @builtin Pkg::TargetRemove(string name) -> bool
 *
 * install package by name
 */
YCPValue
PkgModuleFunctions::TargetRemove(YCPList args)
{
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
get_disk_stats (const char *fs, long long *used, long long *size)
{
    struct statvfs sb;
    if (statvfs (fs, &sb) < 0)
    {
	*used = *size = -1;
	return;
    }
    long long blocksize = sb.f_frsize ? : sb.f_bsize;
    *size = sb.f_blocks * blocksize;
    *used = (sb.f_blocks - sb.f_bfree) * blocksize;
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

    long long used, size;
    get_disk_stats (args->value(0)->asString()->value().c_str(), &used, &size);

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

    long long used, size;
    get_disk_stats (args->value(0)->asString()->value().c_str(), &used, &size);

    return YCPInteger (used);
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
    retmap->add (YCPString ("distname"), YCPString (parser.distversion()));
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

