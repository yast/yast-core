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

   File:	PkgModuleFunctionsSource.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to InstSrc
		Handles source related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstSrc.h>
#include <y2pm/InstSrcPtr.h>
#include <y2pm/InstSrcDescr.h>
#include <y2pm/InstSrcDescrPtr.h>
#include <y2pm/InstTarget.h>
#include <y2pm/PMError.h>
#include <y2pm/PMSelectionManager.h>

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

InstSrcManager::ISrcId
PkgModuleFunctions::getSourceByArgs (YCPList args, int pos)
{
    InstSrcManager::ISrcId id;

    if ((args->size() <= pos)
	|| !(args->value(pos)->isInteger()))
    {
	y2error ("source id must be int");
	return id;
    }

    unsigned int source_slot = args->value(pos)->asInteger()->value();
    if (source_slot < 0
	|| source_slot >= _sources.size()
	|| _sources[source_slot] == 0)
    {
	y2error ("Source not active");
	return id;
    }

    return _sources[source_slot];
}

/*
 * add source ids to _sources[]
 *
 */
void
PkgModuleFunctions::addSrcIds (InstSrcManager::ISrcIdList & nids, bool enable)
{
    if (nids.size() > 0)						// if any sources
    {
	unsigned int number_of_known_sources = 0;
	y2milestone ("addSrcIds %zd, known_sources %zd", nids.size(), _sources.size());
	for (InstSrcManager::ISrcIdList::const_iterator it = nids.begin();	// loop through all sources
	     it != nids.end(); ++it)
	{
	    number_of_known_sources = _sources.size();

	    if (_first_free_source_slot < number_of_known_sources)
	    {
		_sources[_first_free_source_slot] = *it;

		// find next free slot
		while (++_first_free_source_slot < number_of_known_sources)
		{
		    if (_sources[_first_free_source_slot] == 0)
			break;
		}
	    }
	    else		// add a new slot
	    {
		_sources.push_back (*it);
                _first_free_source_slot = _sources.size();
	    }

	    if (enable)
		_y2pm.instSrcManager().enableSource (*it);

	    SetMediaCallback (*it);			// inform about callbacks

	} // loop over InstSrcManager


    } // any sources at all
}

/*
 * start cached sources
 *
 * set _sources and _first_free_source_slot
 * If force is true, the _sources is recreated even if it already existed.
 * This invalidates all source_ids used by the caller.
 */
void
PkgModuleFunctions::startCachedSources (bool enabled_only, bool force)
{
    if (_cache_started && !force)
	return;

    if ( force )
    {
        _sources.clear();
        _first_free_source_slot = 0;
    }

    InstSrcManager::ISrcIdList nids;

    _y2pm.instSrcManager().getSources (nids, enabled_only);		// get all sources from manager

    addSrcIds (nids, false);

    _cache_started = true;

    return;
}

// ------------------------------------------------------------------
// source related

/**
 * @builtin Pkg::SourceStartManager (boolean autoEnable) -> boolean
 *
 * Starts the InstSrcManager.
 * @param
 */
YCPValue
PkgModuleFunctions::SourceStartManager (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::SourceStartManager");
    }

    bool enable = args->value(0)->asBoolean()->value();

    if ( enable ) {
        _y2pm.instSrcManager();
        return YCPBoolean( true );
    } else {
        if ( _y2pm.noAutoInstSrcManager() ) return YCPBoolean( true );
        else return YCPBoolean( false );
    }
}


/**
 * @builtin Pkg::SourceCreate (string url [, string product_dir]) -> integer
 *
 * creates a *NEW* package source under the given url
 *
 * if product_dir is _not_ given or empty, creates sources for all products
 * and returns the id of the first product source
 *
 * This is only needed at initial installation or in
 * the source manager. Normal code should rely on the
 * cached sources.
 * @see SourceStartCache
 */
YCPValue
PkgModuleFunctions::SourceCreate (YCPList args)
{
    if ((args->size() < 1)
	|| !(args->value(0)->isString())
	|| ((args->size() == 2)
	    && (!(args->value(1)->isString()))))
    {
	return YCPError ("Bad args to Pkg::SourceCreate");
    }

    const Url url( args->value(0)->asString()->value() );
    Pathname dir;
    if (args->size() == 2)
	dir = Pathname (args->value(1)->asString()->value());

#if 0
    // check if url already known

    unsigned int number_of_known_sources = _sources.size();
    if (number_of_known_sources > 0)
    {
	for (unsigned int i = 0; i < number_of_known_sources; ++i)
	{
	    constInstSrcDescrPtr source_descr = _sources[i]->descr();
	    if ((source_descr)
		&& (source_descr->url().asString() == url.asString())
		&& (dir.empty()
		    || (!dir.empty()
			&& (source_descr->product_dir() == dir))))
	    {
		y2milestone ("Source '%s' already known", url.asString().c_str());
		return YCPInteger (i);
	    }
	}
    }
#endif

    unsigned int new_slot = _first_free_source_slot;

    InstSrcManager::ISrcIdList nids;

    // check url for products

    _last_error = _y2pm.instSrcManager().scanMedia (nids, url);

    if (nids.size() == 0)
	return YCPError ("No source data found");

    addSrcIds (nids, true);

    return YCPInteger (new_slot);
}


/**
 * @builtin Pkg::SourceStartCache (boolean enabled_only) -> list of integer
 *
 * Start cached sources
 *
 * return list of known (and enabled) source id's
 * This starts the packagemanager from it's cache of known
 * sources
 */
YCPValue
PkgModuleFunctions::SourceStartCache (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::SourceStartCache");
    }

    startCachedSources (args->value(0)->asBoolean()->value());
    return SourceGetCurrent (YCPList());
}


/**
 * @builtin Pkg::SourceGetCurrent (void) -> list of source ids
 *
 * return all currently known and enabled sources
 *
 */
YCPValue
PkgModuleFunctions::SourceGetCurrent (YCPList args)
{
    YCPList sources;
    for (unsigned int i = 0; i < _sources.size(); ++i)
    {
	sources->add (YCPInteger (i));
    }
    return sources;
}


/**
 * @builtin Pkg::SourceFinish (integer id) -> bool
 *
 * disables source
 *
 */
YCPValue
PkgModuleFunctions::SourceFinish (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isInteger()))
    {
	return YCPError ("Bad args to Pkg::SourceFinish");
    }

    unsigned int source_slot = args->value(0)->asInteger()->value();
    if ((source_slot < 0)
	|| source_slot >= _sources.size())
    {
	return YCPError ("Bad source id", YCPBoolean (false));
    }

    if (_sources[source_slot] == 0)
    {
	return YCPError ("Source not active", YCPBoolean (true));
    }

    _last_error = _y2pm.instSrcManager().disableSource( _sources[source_slot]);
    _sources[source_slot] = 0;
    if (source_slot < _first_free_source_slot)
	_first_free_source_slot = source_slot;
    y2milestone ("disable: %d: %s", source_slot, _last_error.errstr().c_str());
    return YCPBoolean (true);
}


/**
 * @builtin Pkg::SourceFinishAll () -> bool
 *
 * disables all known sources
 *
 */
YCPValue
PkgModuleFunctions::SourceFinishAll (YCPList args)
{
    for (unsigned int source_slot = 0; source_slot < _sources.size(); ++source_slot)
    {
	if (_sources[source_slot] != 0)
	{
	    _last_error = _y2pm.instSrcManager().disableSource( _sources[source_slot]);
	    y2milestone ("disable: %d: %s", source_slot, _last_error.errstr().c_str());
	    _sources[source_slot] = 0;
	}
    }
    _first_free_source_slot = 0;
    return YCPBoolean (true);
}


/**
 * @builtin Pkg::SourceGeneralData (integer id) -> map
 *
 * returns general data about the source as a map:
 *   $[ "default_activate" : bool,
 *	"product_dir" : string,
 *	"url" : string,			// also in SourceMediaData
 *	"type" : string
 * ];
 *
 */
YCPValue
PkgModuleFunctions::SourceGeneralData (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    YCPMap data;
    constInstSrcDescrPtr source_descr = source_id->descr();

    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("default_activate"), YCPBoolean (source_descr->default_activate()));
    data->add (YCPString ("product_dir"), YCPString (source_descr->product_dir().asString()));
    data->add (YCPString ("type"), YCPString (InstSrc::toString (source_descr->type())));
    data->add (YCPString ("url"), YCPString (source_descr->url().asString ()));
    data->add (YCPString ("enabled"), YCPBoolean(source_descr->default_activate()));
    return data;
}


/**
 * @builtin Pkg::SourceMediaData (integer id) -> map
 *
 * returns media data about the source as a map:
 *   $[ "media_count" : string,
 *	"media_id" : bool,
 *	"media_vendor" : string,
 *	"url" : string			// also in SourceGeneralData
 * ];
 *
 */
YCPValue
PkgModuleFunctions::SourceMediaData (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    YCPMap data;
    constInstSrcDescrPtr source_descr = source_id->descr();
    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("media_count"), YCPInteger (source_descr->media_count()));
    data->add (YCPString ("media_id"), YCPString (source_descr->media_id()));
    data->add (YCPString ("media_vendor"), YCPString (source_descr->media_vendor()));
    data->add (YCPString ("url"), YCPString (source_descr->url().asString ()));

    return data;
}


/**
 * @builtin Pkg::SourceProductData (integer id) -> map
 *
 * returns product data about the source as a map:
 *   $[ "productname" : string,
 *	"productversion" : string,
 *	"baseproductname" : string,
 *	"baseproductversion" : string,
 *	"vendor" : string,
 *	"defaultbase" : string,
 *	"architectures" : list (string),
 *	"requires" : string,
 *	"linguas" : list (string),
 *	"label" : string,
 *	"labelmap" : map (string lang, string label),
 *	"language" : string,
 *	"timezone" : string,
 *	"descrdir" : string,
 *	"datadir" : string
 * ];
 *
 */
YCPValue
PkgModuleFunctions::SourceProductData (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    YCPMap data;
    constInstSrcDescrPtr source_descr = source_id->descr();
    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("productname"), YCPString ((const std::string &)(source_descr->content_product().name)));
    data->add (YCPString ("productversion"), YCPString (source_descr->content_product().edition.version()));
    data->add (YCPString ("baseproductname"), YCPString ((const std::string &)(source_descr->content_baseproduct().name)));
    data->add (YCPString ("baseproductversion"), YCPString (source_descr->content_baseproduct().edition.version()));
    data->add (YCPString ("vendor"), YCPString (source_descr->content_vendor ()));
    data->add (YCPString ("defaultbase"), YCPString (source_descr->content_defaultbase ()));
    InstSrcDescr::ArchMap::const_iterator it1 = source_descr->content_archmap().find (_y2pm.baseArch());
    if (it1 != source_descr->content_archmap().end())
    {
	YCPList architectures;
	for (std::list<PkgArch>::const_iterator it2 = it1->second.begin();
	     it2 != it1->second.end(); ++it2)
	{
	    architectures->add (YCPString ((const std::string &)(*it2)));
	}
	data->add (YCPString ("architectures"), architectures);
    }
    data->add (YCPString ("requires"), YCPString (source_descr->content_requires().asString()));
    YCPList linguas;
    for (InstSrcDescr::LinguasList::const_iterator it = source_descr->content_linguas().begin();
	it != source_descr->content_linguas().end(); ++it)
    {
	linguas->add (YCPString (*it));
    }
    data->add (YCPString ("linguas"), linguas);
    data->add (YCPString ("label"), YCPString (source_descr->content_label ()));
    YCPMap labelmap;
    for (InstSrcDescr::LabelMap::const_iterator it = source_descr->content_labelmap().begin();
	it != source_descr->content_labelmap().end(); ++it)
    {
	labelmap->add (YCPString ((const std::string &)(it->first)), YCPString (it->second));
    }
    data->add (YCPString ("labelmap"), labelmap);
    data->add (YCPString ("language"), YCPString (source_descr->content_language ()));
    data->add (YCPString ("timezone"), YCPString (source_descr->content_timezone ()));
    data->add (YCPString ("descrdir"), YCPString (source_descr->content_descrdir().asString()));
    data->add (YCPString ("datadir"), YCPString (source_descr->content_datadir().asString()));
    return data;
}


/**
 * @builtin Pkg::SourceProvideFile (integer source, integer medianr, string file) -> string path
 * provide file from source to local path
 * @param source	source id from SourceStartCache()
 * @param medianr	should be 0 (let the packagemanager decide about the media)
 * @param file		filename relative to source (i.e. CD) root path
 */
YCPValue
PkgModuleFunctions::SourceProvideFile (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 3)
	|| !(args->value(1)->isInteger())
	|| !(args->value(2)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceProvideFile");
    }

    Pathname file_r;
    Pathname path (args->value(2)->asString()->value());
    _last_error = source_id->provideFile (args->value(1)->asInteger()->value(), path, file_r);
    if (_last_error)
    {
	y2error ("provideFile(%s) failed: %s", path.asString().c_str(), _last_error.errstr().c_str());
	return YCPVoid();
    }
    return YCPString (file_r.asString());
}


/**
 * @builtin Pkg::SourceProvideDir (integer source, integer medianr, string path) -> string path
 * provide directory from source to local path
 * @param source	source id from SourceStartCache()
 * @param medianr	should be 0 (let the packagemanager decide about the media)
 * @param dir		pathname relative to source (i.e. CD) root path
 */
YCPValue
PkgModuleFunctions::SourceProvideDir (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 3)
	|| !(args->value(1)->isInteger())
	|| !(args->value(2)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceProvideDir");
    }

    Pathname dir_r;
    Pathname path (args->value(2)->asString()->value());
    _last_error = source_id->provideDir (args->value(1)->asInteger()->value(), path, dir_r);
    if (_last_error)
    {
	y2error ("provideDir(%s) failed: %s", path.asString().c_str(), _last_error.errstr().c_str());
	return YCPVoid();
    }
    return YCPString (dir_r.asString());
}


/**
 * @builtin Pkg::SourceCacheCopyTo (string dir) -> bool
 *
 * copy cache data of all installation sources to 'dir'
 * to be called at end of initial installation with
 * the target root dir.
 */
YCPValue
PkgModuleFunctions::SourceCacheCopyTo (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceCacheCopyTo");
    }
    Pathname path (args->value(0)->asString()->value());
    _last_error = _y2pm.instSrcManager().cacheCopyTo (path);
    if (_last_error != PMError::E_ok)
    {
	return YCPError (string ("SourceCacheCopyTo failed: ")+_last_error.errstr(), YCPBoolean (false));
    }

    // copy product data of all open sources

    if (_inst_order.empty())
    {
	for (unsigned int i = 0; i < _sources.size(); ++i)
	{
	    if (_sources[i] != 0)
	    {
		_y2pm.instTarget().installProduct(_sources[i]->descr());
	    }
	}
    }
    else	// install products according to installation order
    {
	for (InstSrcManager::ISrcIdList::const_iterator it = _inst_order.begin(); it != _inst_order.end(); ++it)
	{
	    _y2pm.instTarget().installProduct((*it)->descr());
	}
    }

    // copy selection data to target
    for (PMManager::PMSelectableVec::const_iterator it = _y2pm.selectionManager().begin();
	 it != _y2pm.selectionManager().end(); ++it)
    {
	if ((*it)->to_install())
	{
	    Pathname selfile;
	    if ( PMSelectionPtr( (*it)->candidateObj() )->provideSelToInstall( selfile ) == PMError::E_ok )
	    {
		_y2pm.instTarget().installSelection( selfile );
	    }
	    else
	    {
		y2error ("provideSelToInstall failed for '%s'", (*it)->name().asString().c_str() );
	    }
	}
    }
    return YCPBoolean (true);
}


/**
 * @builtin Pkg::SourceSetRamCache (boolean) -> boolean
 *
 * enable/disable caching data on ramdisk (source is mounted, use
 * data directly from source instead)
 */
YCPValue
PkgModuleFunctions::SourceSetRamCache (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::SourceSetRamCache");
    }
    _y2pm.setCacheToRamdisk (args->value(0)->asBoolean()->value());
    return YCPBoolean (true);
}


/** ------------------------
 *
 * @builtin Pkg::SourceProduct (integer source_id) -> map
 *
 * return $["product" : string, "vendor" : string ]
 */

YCPValue
PkgModuleFunctions::SourceProduct (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();
    return Descr2Map (source_id->descr());
}

/** ------------------------
 *
 * @builtin Pkg::SourceSetEnabled (integer source_id, boolean enabled) -> bool
 *
 * Set default activation state of source. Return true, if successful, false, if not.
 */
YCPValue
PkgModuleFunctions::SourceSetEnabled (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::SourceSetEnabled");
    }

    bool enabled = args->value(1)->asBoolean()->value();

    _last_error = _y2pm.instSrcManager().setAutoenable( source_id, enabled );

    if ( _last_error ) return YCPBoolean( false );
    else return YCPBoolean( true );
}

/** ------------------------
 *
 * @builtin Pkg::SourceDelete (integer source_id ) -> bool
 *
 * Delete source. Return true, if successful, false, if not.
 * This function invalidates all source_ids. Use SourceGetCurrent to get
 * the new list of source_ids.
 */
YCPValue
PkgModuleFunctions::SourceDelete (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    _last_error = _y2pm.instSrcManager().disableSource( source_id );
    _last_error = _y2pm.instSrcManager().deleteSource( source_id );
y2milestone ("deleteSource(): %s", _last_error.errstr().c_str());
    if ( _last_error ) return YCPBoolean( false );

    startCachedSources( false, true );		// re-create _sources[]

    return YCPBoolean( true );
}

/** ------------------------
 *
 * @builtin Pkg::SourceRaisePriority (integer source_id ) -> bool
 *
 * Raise priority of source. Return true on success, false on error.
 * This function invalidates all source_ids. Use SourceGetCurrent to get
 * the new list of source_ids.
 */
YCPValue
PkgModuleFunctions::SourceRaisePriority (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    _last_error = _y2pm.instSrcManager().rankUp( source_id );
    if ( _last_error ) return YCPBoolean( false );

    startCachedSources( false, true );

    return YCPBoolean( true );
}

/** ------------------------
 *
 * @builtin Pkg::SourceLowerPriority (integer source_id ) -> void
 *
 * Raise priority of source. Return true on success, false on error.
 * This function invalidates all source_ids. Use SourceGetCurrent to get
 * the new list of source_ids.
 */
YCPValue
PkgModuleFunctions::SourceLowerPriority (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    _last_error = _y2pm.instSrcManager().rankDown( source_id );
    if ( _last_error ) return YCPBoolean( false );

    startCachedSources( false, true );

    return YCPBoolean( true );
}

/** ------------------------
 *
 * @builtin Pkg::SourceSaveRanks () -> boolean
 *
 * Save ranks to disk. Return true on success, false on error.
 */
YCPValue
PkgModuleFunctions::SourceSaveRanks (YCPList args)
{
    _last_error = _y2pm.instSrcManager().setNewRanks();
    if ( _last_error ) return YCPBoolean( false );

    return YCPBoolean( true );
}

/** ------------------------
 *
 * @builtin Pkg::SourceChangeUrl (integer source_id , string url ) -> bool
 *
 * change url of source
 * used primarely when re-starting during installation and a cd-device
 * changed from hdX to srX since ide-scsi was activated
 * returns false if source_id doesn't exists or the url is malformed
 */
YCPValue
PkgModuleFunctions::SourceChangeUrl (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad source to Pkg::SourceChangeUrl", YCPBoolean (false));
    }

    Url url (args->value(1)->asString()->value());
    if (!url.isValid())
    {
	return YCPError ("Bad url to Pkg::SourceChangeUrl", YCPBoolean (false));
    }

    _last_error = _y2pm.instSrcManager().rewriteUrl ( source_id, url );
    if ( _last_error ) return YCPBoolean( false );

    return YCPBoolean( true );
}



/** ------------------------
 *
 * @builtin Pkg::SourceInstallOrder (map order_map) -> bool
 *
 * set installation order
 *   order_map = map of 'int order : int source_id',
 * 	with order 1 == highest
 */
YCPValue
PkgModuleFunctions::SourceInstallOrder (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isMap()))
    {
	return YCPError ("Bad args to Pkg::SourceInstallOrder");
    }

    _inst_order.clear ();
    YCPMap map = args->value(0)->asMap();
    for (YCPMapIterator it = map->begin(); it != map->end(); ++it)
    {
	unsigned int source_slot;
	source_slot = _sources.size();	// preset with error value

	if (it.value()->isInteger())	// check correctness of map
	    source_slot = it.value()->asInteger()->value();	// use value from map

	if (source_slot < _sources.size()
	    && _sources[source_slot] != 0)
	{
	    _inst_order.push_back (_sources[source_slot]);
	}
	else
	{
	    y2error ("order map entry '%s:%s': no source existing", it.key()->toString().c_str(), it.value()->toString().c_str());
	    _inst_order.clear ();
	    return YCPVoid();
	}
    }
    return YCPBoolean (true);
}

