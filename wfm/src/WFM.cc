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

   File:	WFM.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "WFM.h"
#include "Y2WFMComponent.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;

static YCPInteger
WFMSCROpen (const YCPString& name, const YCPBoolean &check_version)
{
    return Y2WFMComponent::instance ()->SCROpen (name, check_version);
}

static YCPValue
WFMSCRClose (const YCPInteger& handle)
{
    Y2WFMComponent::instance ()->SCRClose (handle);

    return YCPVoid ();
}


static YCPString
WFMSCRGetName (const YCPInteger &handle)
{
    return Y2WFMComponent::instance ()->SCRGetName (handle);
}


static YCPValue
WFMSCRSetDefault (const YCPInteger &handle)
{
    Y2WFMComponent::instance ()->SCRSetDefault (handle);
    return YCPVoid ();
}


static YCPInteger
WFMSCRGetDefault ()
{
    return Y2WFMComponent::instance ()->SCRGetDefault ();
}


static YCPValue
WFMCallFunction (const YCPString& name)
{
    return Y2WFMComponent::instance ()->CallFunction (name);
}

static YCPBoolean
WFMClientExists (const YCPString& name)
{
    return Y2WFMComponent::instance ()->ClientExists (name);
}

static YCPValue
WFMCallFunction1 (const YCPString& name, const YCPList& args)
{
    return Y2WFMComponent::instance ()->CallFunction (name, args);
}

static YCPValue
WFMArgs ()
{
    return Y2WFMComponent::instance ()->Args ();
}

static YCPValue
WFMArgs2 (const YCPInteger& index)
{
    return Y2WFMComponent::instance ()->Args (index);
}

static YCPString
WFMGetLanguage ()
{
    return Y2WFMComponent::instance ()->GetLanguage ();
}


static YCPString
WFMGetEncoding ()
{
    return Y2WFMComponent::instance ()->GetEncoding ();
}


static YCPString
WFMGetEnvironmentEncoding ()
{
    return Y2WFMComponent::instance ()->GetEnvironmentEncoding ();
}


static YCPString
WFMSetLanguage (const YCPString& language)
{
    return Y2WFMComponent::instance ()->SetLanguage (language);
}

static YCPString
WFMSetLanguage2 (const YCPString& language, const YCPString& encoding)
{
    return Y2WFMComponent::instance ()->SetLanguage (language, encoding);
}


static YCPValue
WFMRead (const YCPPath& p, const YCPValue& arg)
{
    return Y2WFMComponent::instance ()->Read (p, arg);
}


static YCPValue
WFMWrite3 (const YCPPath& p, const YCPValue& arg1, const YCPValue& arg2 = YCPNull ())
{
    return Y2WFMComponent::instance ()->Write (p, arg1, arg2);
}

static YCPValue
WFMWrite2 (const YCPPath& p, const YCPValue& arg)
{
    return WFMWrite3 (p, arg);
}

static YCPValue
WFMExecute (const YCPPath& p, const YCPValue& arg)
{
    return Y2WFMComponent::instance ()->Execute (p, arg);
}


bool WFM::registered = false;

WFM::WFM ()
{
    if (registered)
    {
	return;
    }


    y2debug( "registering WFM builtins" );
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL
	{ "WFM",		"",				NULL, 		DECL_NAMESPACE, ETCf },
	{ "SCROpen",		"integer (string, boolean)",	(void*)WFMSCROpen,               ETC },
	{ "SCRClose",		"void (integer)",		(void*)WFMSCRClose,		 ETC },
	{ "SCRGetName",		"string (integer)",		(void*)WFMSCRGetName,		 ETC },
	{ "SCRSetDefault",	"void (integer)",		(void*)WFMSCRSetDefault,	 ETC },
	{ "SCRGetDefault",	"integer ()",			(void*)WFMSCRGetDefault,	 ETC },
	{ "CallFunction",	"any (string, list <any>)",	(void*)WFMCallFunction1,	 ETC },
	{ "CallFunction",	"any (string)",			(void*)WFMCallFunction,		 ETC },
	{ "call",		"any (string, list <any>)",	(void*)WFMCallFunction1,         ETC },
	{ "call",		"any (string)",			(void*)WFMCallFunction,		 ETC },
	{ "Args",		"list <any> ()",		(void*)WFMArgs,			 ETC },
	{ "Args",		"any (integer)",		(void*)WFMArgs2,		 ETC },
	{ "GetLanguage",	"string ()",			(void*)WFMGetLanguage,		 ETC },
	{ "GetEncoding",	"string ()",			(void*)WFMGetEncoding,		 ETC },
	{ "GetEnvironmentEncoding",	"string ()",		(void*)WFMGetEnvironmentEncoding,ETC },
	{ "SetLanguage",	"string (string)",		(void*)WFMSetLanguage,           ETC },
	{ "SetLanguage",	"string (string, string)",	(void*)WFMSetLanguage2,		 ETC },
	{ "Read",		"any (path,any)",		(void*)WFMRead,			 ETC },
	{ "Write",		"boolean (path, any, any)",	(void*)WFMWrite3,		 ETC },
	{ "Write",		"boolean (path, any)",		(void*)WFMWrite2,		 ETC },
	{ "Execute",		"any (path, any)",		(void*)WFMExecute,		 ETC },
	{ "ClientExists",	"boolean (string)",		(void*)WFMClientExists,		 ETC },
	{ NULL, NULL, NULL, ETC }
#undef ETC
#undef ETCf
    };

    static_declarations.registerDeclarations ("WFM", declarations);

    registered = true;
}

