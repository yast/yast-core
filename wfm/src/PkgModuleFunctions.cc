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

   File:	PkgModuleFunctions.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	PkgModuleFunctions constructor, destructor
/-*/


#include <ycp/y2log.h>
#include <PkgModuleFunctions.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/InstSrcDescr.h>
#include <ycp/YCPError.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPString.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

/**
 * Constructor.
 */
PkgModuleFunctions::PkgModuleFunctions ()
    : _first_free_source_slot(0)
{
    _y2pm.packageManager(false);	// start without target
}

/**
 * Destructor.
 */
PkgModuleFunctions::~PkgModuleFunctions ()
{
}


// ------------------------------------------------------------------
// general

/**
 * @builtin Pkg::CheckSpace (list partitions) -> list usage
 *
 * checks current space usage across partitions
 * @param partitions	list of used partitions
 * @return list of usage data
 *
 */
YCPValue
PkgModuleFunctions::CheckSpace (YCPList args)
{
#warning CheckSpace TBD
    y2warning ("CheckSpace (%s)", args->toString().c_str());
    return YCPList ();
}


// ------------------------------------------------------------------
// source related

/**
 * @builtin Pkg::SourceInit (string url) -> int
 *
 * initializes a package source under the given url
 *
 */
YCPValue
PkgModuleFunctions::SourceInit (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceInit");
    }

    const Url url( args->value(0)->asString()->value() );

    // check if url already known

    int number_of_known_sources = _sources.size();
    if (number_of_known_sources > 0)
    {
	for (int i = 0; i < number_of_known_sources; ++i)
	{
	    constInstSrcDescrPtr source_descr = _sources[i]->descr();
	    if ((source_descr)
		&& (source_descr->url().asString() == url.asString()))
	    {
		y2milestone ("Source '%s' already known", url.asString().c_str());
		return YCPInteger (i);
	    }
	}
    }

    InstSrcManager& MGR = _y2pm.instSrcManager();
    InstSrcManager::ISrcIdList nids;

    // check url for products

    PMError err = MGR.scanMedia( nids, url );

    if ( nids.size() )
    {
#warning PkgModuleFunctions::SourceInit supports only one product per source

	InstSrcManager::ISrcId source_id = *nids.begin();
	int new_slot = -1;

	if (_first_free_source_slot < number_of_known_sources)
	{
	    new_slot = _first_free_source_slot;
	    _sources[_first_free_source_slot] = source_id;

	    // find next free slot
	    while (++_first_free_source_slot < number_of_known_sources)
	    {
		if (_sources[_first_free_source_slot] == 0)
		    break;
	    }
	}
	else		// add a new slot
	{
	    new_slot = _sources.size();
	    _sources.push_back (source_id);
	}

	err = MGR.enableSource( source_id );
	y2milestone ("enable: %d: %s", new_slot, err.errstr().c_str());
	return YCPInteger (new_slot);
    }
    return YCPError ("No source data found");
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

    int source_slot = args->value(0)->asInteger()->value();

    if (_sources[source_slot] == 0)
    {
	return YCPError ("Source not active", YCPBoolean (true));
    }

    PMError err = _y2pm.instSrcManager().disableSource( _sources[source_slot]);
    _sources[source_slot] = 0;
    if (source_slot < _first_free_source_slot)
	_first_free_source_slot = source_slot;
    y2milestone ("disable: %d: %s", source_slot, err.errstr().c_str());
    return YCPBoolean (true);
}


/**
 * @builtin Pkg::SourceGeneralData (integer id) -> map
 *
 * returns general data about the source as a map:
 *   $[ "base_arch" : string,
 *	"default_activate" : bool,
 *	"product_dir" : string,
 *	"url" : string,			// also in SourceMediaData
 *	"type" : string
 * ];
 *
 */
YCPValue
PkgModuleFunctions::SourceGeneralData (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isInteger()))
    {
	return YCPError ("Bad args to Pkg::SourceData");
    }

    int source_slot = args->value(0)->asInteger()->value();
    if (source_slot < 0
	|| source_slot >= _sources.size()
	|| _sources[source_slot] == 0)
    {
	return YCPError ("Source not active");
    }

    YCPMap data;

    InstSrcManager::ISrcId source_id = _sources[source_slot];
    constInstSrcDescrPtr source_descr = source_id->descr();

    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("base_arch"), YCPString ((const std::string &)source_descr->base_arch()));
    data->add (YCPString ("default_activate"), YCPBoolean (source_descr->default_activate()));
    data->add (YCPString ("product_dir"), YCPString (source_descr->product_dir().asString()));
    data->add (YCPString ("type"), YCPString (InstSrc::toString (source_descr->type())));
    data->add (YCPString ("url"), YCPString (source_descr->url().asString ()));
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
    if ((args->size() != 1)
	|| !(args->value(0)->isInteger()))
    {
	return YCPError ("Bad args to Pkg::SourceData");
    }

    int source_slot = args->value(0)->asInteger()->value();

    if (source_slot < 0
	|| source_slot >= _sources.size()
	|| _sources[source_slot] == 0)
    {
	return YCPError ("Source not active");
    }

    YCPMap data;

    InstSrcManager::ISrcId source_id = _sources[source_slot];
    constInstSrcDescrPtr source_descr = source_id->descr();
    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("media_count"), YCPInteger (atoi (source_descr->media_count().c_str())));
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
    if ((args->size() != 1)
	|| !(args->value(0)->isInteger()))
    {
	return YCPError ("Bad args to Pkg::SourceData");
    }

    int source_slot = args->value(0)->asInteger()->value();

    if (source_slot < 0
	|| source_slot >= _sources.size()
	|| _sources[source_slot] == 0)
    {
	return YCPError ("Source not active");
    }

    YCPMap data;

    InstSrcManager::ISrcId source_id = _sources[source_slot];
    constInstSrcDescrPtr source_descr = source_id->descr();
    if (!source_descr)
	return YCPError ("No description for source", data);

    data->add (YCPString ("productname"), YCPString ((const std::string &)(source_descr->content_product().name)));
    data->add (YCPString ("productversion"), YCPString (PkgEdition::toString (source_descr->content_product().edition)));
    data->add (YCPString ("baseproductname"), YCPString ((const std::string &)(source_descr->content_baseproduct().name)));
    data->add (YCPString ("baseproductversion"), YCPString (PkgEdition::toString (source_descr->content_baseproduct().edition)));
    data->add (YCPString ("vendor"), YCPString (source_descr->content_vendor ()));
    data->add (YCPString ("defaultbase"), YCPString (source_descr->content_defaultbase ()));
    const std::string& base_arch = (const std::string &)(source_descr->base_arch());
    InstSrcDescr::ArchMap::const_iterator a_it = source_descr->content_archmap().find (base_arch);
    if (a_it != source_descr->content_archmap().end())
    {
	YCPList architectures;
	for (std::list<Pathname>::const_iterator p_it = a_it->second.begin();
	     p_it != a_it->second.end(); ++p_it)
	{
	    architectures->add (YCPString (p_it->asString()));
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

