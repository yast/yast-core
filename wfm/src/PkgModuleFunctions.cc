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
		Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	PkgModuleFunctions constructor, destructor and call handling
/-*/


#include <ycp/y2log.h>
#include <ycp/YExpression.h>
#include <ycp/YBlock.h>
#include "PkgModuleFunctions.h"
#include "PkgModuleCallbacks.h"

#include <y2pm/InstSrcDescr.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPString.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPVoid.h>

class Y2PkgFunction: public Y2Function 
{
    unsigned int m_position;
    PkgModuleFunctions* m_instance;
    YCPValue m_param1;
    YCPValue m_param2;
    YCPValue m_param3;
    YCPValue m_param4;
public:

    Y2PkgFunction (PkgModuleFunctions* instance, unsigned int pos);    
    bool attachParameter (const YCPValue& arg, const int position);
    constTypePtr wantedParameterType () const;
    bool appendParameter (const YCPValue& arg);
    bool finishParameters ();
    YCPValue evaluateCall ();
    bool reset ();
};


    Y2PkgFunction::Y2PkgFunction (PkgModuleFunctions* instance, unsigned int pos) :
	m_position (pos)
	, m_instance (instance)
	, m_param1 ( YCPNull () )
	, m_param2 ( YCPNull () )
	, m_param3 ( YCPNull () )
	, m_param4 ( YCPNull () )
    {
    };
    
    bool Y2PkgFunction::attachParameter (const YCPValue& arg, const int position)
    {
	switch (position)
	{
	    case 0: m_param1 = arg; break; 
	    case 1: m_param2 = arg; break; 
	    case 2: m_param3 = arg; break; 
	    case 3: m_param4 = arg; break; 
	    default: return false;
	}

	return true;
    }
    
    constTypePtr Y2PkgFunction::wantedParameterType () const
    {
	y2internal ("wantedParameterType not implemented");
	return Type::Unspec;
    }
    
    bool Y2PkgFunction::appendParameter (const YCPValue& arg)
    {
	if (m_param1.isNull ())
	{
	    m_param1 = arg;
	    return true;
	} else if (m_param2.isNull ())
	{
	    m_param2 = arg;
	    return true;
	} else if (m_param3.isNull ())
	{
	    m_param3 = arg;
	    return true;
	} else if (m_param4.isNull ())
	{
	    m_param4 = arg;
	    return true;
	}
	y2internal ("appendParameter > 3 not implemented");
	return false;
    }
    
    bool Y2PkgFunction::finishParameters ()
    {
	y2internal ("finishParameters not implemented");
	return true;
    }
    
    YCPValue Y2PkgFunction::evaluateCall ()
    {
	switch (m_position) {
#include "PkgBuiltinCalls.h"
	}
	
	return YCPNull ();
    }
    
    bool Y2PkgFunction::reset ()
    {
	m_param1 = YCPNull ();
	m_param2 = YCPNull ();
	m_param3 = YCPNull ();
	m_param4 = YCPNull ();

	return true;
    }

/**
 * Constructor.
 */
PkgModuleFunctions::PkgModuleFunctions ()
    : _callbackHandler( *new CallbackHandler( ) )
{
  // it's cheap to launch them
  _y2pm.packageManager();
  _y2pm.selectionManager();

  registerFunctions ();
}

/**
 * Destructor.
 */
PkgModuleFunctions::~PkgModuleFunctions ()
{
    SourceFinishAll ();
    TargetFinish ();
    delete &_callbackHandler;
}

Y2Function* PkgModuleFunctions::createFunctionCall (const string name, constFunctionTypePtr type)
{
    vector<string>::iterator it = find (_registered_functions.begin ()
	, _registered_functions.end (), name);
    if (it == _registered_functions.end ())
    {
	y2error ("No such function %s", name.c_str ());
	return NULL;
    }
    
    return new Y2PkgFunction (this, it - _registered_functions.begin ());
}

void PkgModuleFunctions::registerFunctions()
{
#include "PkgBuiltinTable.h"
}

///////////////////////////////////////////////////////////////////
//
//
//     METHOD NAME : PkgModuleFunctions::pkgError
//     METHOD TYPE : YCPValue
//
YCPValue PkgModuleFunctions::pkgError( PMError err_r, const YCPValue & ret_r )
{
  _last_error = err_r;
  return ret_r;
}


// ------------------------------------------------------------------
// general

/**
 * @builtin Pkg::InstSysMode () -> void
 *
 * Set packagemanager to "inst-sys" mode
 * - dont use local caches (ramdisk!)
 *
 * !!!!!!!!!! CAUTION !!!!!!!!!!!
 * Can only be called ONCE
 * MUST be called before any other function
 * !!!!!!!!!! CAUTION !!!!!!!!!!!
 */
YCPValue
PkgModuleFunctions::InstSysMode ()
{
    _y2pm.setNotRunningFromSystem();
    return YCPVoid();
}


/**
 * @builtin Pkg::SetLocale (string locale) -> void
 *
 * set the given locale as the "preferred" locale
 */
YCPValue
PkgModuleFunctions::SetLocale (const YCPString &locale)
{
    LangCode langcode(locale->value());
    _y2pm.setPreferredLocale (langcode);
    return YCPVoid();
}

/**
 * @builtin Pkg::GetLocale () -> string locale
 *
 * get the currently preferred locale
 */
YCPValue
PkgModuleFunctions::GetLocale ()
{
    return YCPString ((const std::string &)(_y2pm.getPreferredLocale()));
}


/**
 * @builtin Pkg::SetAdditionalLocales (list <string>) -> void
 *
 * set list of
 */
YCPValue
PkgModuleFunctions::SetAdditionalLocales (YCPList langycplist)
{
    Y2PM::LocaleSet langcodelist;
    int i = 0;
    while (i < langycplist->size())
    {
	if (langycplist->value (i)->isString())
	{
	    langcodelist.insert (LangCode (langycplist->value (i)->asString()->value()));
	}
	else
	{
	    y2error ("Pkg::SetAdditionalLocales ([...,%s,...]) not string", langycplist->value (i)->toString().c_str());
	}
	i++;
    }
    _y2pm.setRequestedLocales (langcodelist);
    return YCPVoid();
}

/**
 * @builtin Pkg::GetAdditionalLocales -> list <string>
 *
 * return list of additional locales
 */
YCPValue
PkgModuleFunctions::GetAdditionalLocales ()
{
    YCPList langycplist;
    const Y2PM::LocaleSet & langcodelist = _y2pm.getRequestedLocales();
    for (Y2PM::LocaleSet::const_iterator it = langcodelist.begin();
	 it != langcodelist.end(); ++it)
    {
	langycplist->add (YCPString ((const std::string &)(*it)));
    }
    return langycplist;
}


/**
 * @builtin Pkg::LastError
 *
 * get current error as string
 */
YCPValue
PkgModuleFunctions::LastError ()
{
    return YCPString (_last_error.errstr());
}

/**
 * @builtin Pkg::LastErrorDetails
 *
 * get current error details as string
 */
YCPValue
PkgModuleFunctions::LastErrorDetails ()
{
    return YCPString (_last_error.details());
}

/**
 * @builtin Pkg::LastErrorId
 *
 * get current error as id string
 */
YCPValue
PkgModuleFunctions::LastErrorId ()
{
    int errorId = _last_error;
    switch ( errorId ) {
        case PMError::E_ok:
            return YCPString( "ok" );
        case InstSrcError::E_isrc_cache_duplicate:
            return YCPString( "instsrc_duplicate" );
        default:
            return YCPString( "error" );
    }
}


/** ------------------------
 * Convert InstSrcDescr to product info YCPMap:
 * <TABLE>
 * <TR><TD>$[<TD>"product"             <TD>: YCPString (name' 'version)
 * <TR><TD>,<TD>"vendor"               <TD>: YCPString
 * <TR><TD>,<TD>"requires"             <TD>: YCPString
 * <TR><TD>,<TD>"name"                 <TD>: YCPString
 * <TR><TD>,<TD>"version"              <TD>: YCPString
 * <TR><TD>,<TD>"flags"                        <TD>: YCPString
 * <TR><TD>,<TD>"relnotesurl"          <TD>: YCPString
 * <TR><TD>,<TD>"distproduct"          <TD>: YCPString
 * <TR><TD>,<TD>"distversion"          <TD>: YCPString
 * <TR><TD>,<TD>"baseproduct"          <TD>: YCPString
 * <TR><TD>,<TD>"baseversion"          <TD>: YCPString
 * <TR><TD>];
 * </TABLE>
 */

YCPMap
PkgModuleFunctions::Descr2Map (constInstSrcDescrPtr descr)
{
    YCPMap map;

    map->add (YCPString ("product"), YCPString ((const std::string &)(descr->content_product().asPkgNameEd().name) + " " + descr->content_product().asPkgNameEd().edition.version()));
    map->add (YCPString ("vendor"), YCPString (descr->content_vendor()));
    map->add (YCPString ("requires"), YCPString (descr->content_requires().asString()));

    // for installation/modules/Product.ycp
    map->add (YCPString ("name"), YCPString ((const std::string &)(descr->content_product().asPkgNameEd().name)));
    map->add (YCPString ("version"), YCPString (descr->content_product().asPkgNameEd().edition.version()));
    map->add (YCPString ("flags"), YCPString (descr->content_flags()));
    map->add (YCPString ("relnotesurl"), YCPString (descr->content_relnotesurl()));

    // vendor already in map

    map->add (YCPString ("distproduct"), YCPString ((const std::string &)(descr->content_distproduct().name)));
    map->add (YCPString ("distversion"), YCPString ((const std::string &)(descr->content_distproduct().edition.version())));

    map->add (YCPString ("baseproduct"), YCPString ((const std::string &)(descr->content_baseproduct().asPkgNameEd().name)));
    map->add (YCPString ("baseversion"), YCPString ((const std::string &)(descr->content_baseproduct().asPkgNameEd().edition.version())));

    map->add (YCPString ("defaultbase"), YCPString (descr->content_defaultbase()));

    return map;
}


