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
    /**
     * @builtin SCROpen (string name, bool check_version) -> integer
     * Create a new scr instance. The name must be a valid y2component name
     * (e.g. "scr", "chroot=/mnt:scr"). The component is created immediately.
     * The parameter check_version determined whether the SuSE Version should
     * be checked. On error a negative value is returned.
     */
    return Y2WFMComponent::instance ()->SCROpen (name, check_version);
}

static YCPValue 
WFMSCRClose (const YCPInteger& handle)
{
    /**
     * @builtin SCRClose (integer handle) -> void
     * Close a scr instance.
     */
    Y2WFMComponent::instance ()->SCRClose (handle);
    
    return YCPVoid ();
}


static YCPString 
WFMSCRGetName (const YCPInteger &handle)
{
    /**
     * @builtin SCRGetName (integer handle) -> string
     * Get the name of a scr instance.
     */
    return Y2WFMComponent::instance ()->SCRGetName (handle);
}


static YCPValue 
WFMSCRSetDefault (const YCPInteger &handle)
{
    /**
     * @builtin SCRSetDefault (integer handle) -> void
     * Set's the default scr instance.
     */
    Y2WFMComponent::instance ()->SCRSetDefault (handle);
    return YCPVoid ();
}


static YCPInteger 
WFMSCRGetDefault ()
{
    /**
     * @builtin SCRGetDefault () -> integer
     * Get's the default scr instance.
     */
    return Y2WFMComponent::instance ()->SCRGetDefault ();
}


static YCPValue 
WFMCallFunction (const YCPString& name)
{
    return Y2WFMComponent::instance ()->CallFunction (name);
}

static YCPValue 
WFMCallFunction1 (const YCPString& name, const YCPList& args)
{
    /**
     * @builtin CallFunction(string name, list arguments) -> any
     * This is much like CallModule, but it differs in the way how the
     * module is called. The module call does not make use of the Y2 component
     * system. CallFunction simply looks for a YCP script residing in
     * YAST2HOME or one of the other directories in the Y2 search path.
     * You can't call arbitrary client components, just YCP scripts.
     * The YCP script then is not executed in an own workflow manager,
     * but is just called as a function.
     *
     * This implies, that the called YCP code has full access to
     * all symbol definitions of the YCP code is was called from!
     * A new scope is opened for each block of the YCP code. Since most
     * YCP scripts are implemented as blocks (starting with {) // keep emacs happy }
     * so any symbol declarations inside the called code are dropped
     * when the code is left.
     *
     * What CallFunction has in common with CallModule is that
     * the modulename is temporarily changed to the name of the
     * called function. This has an impact on the translator, that
     * always needs to know, which module as currently being executed.
     *
     * @example CallFunction ("inst_mouse", [true, false]) -> ....
     *
     * In the example, WFM looks for the file YAST2HOME/clients/inst_mouse.ycp
     * and executes it.
     */
    return Y2WFMComponent::instance ()->CallFunction (name, args);
}

static YCPValue 
WFMGetClientName(const YCPInteger& filedescriptor)
{
    /**
     * @builtin GetClientName(integer filedescriptor) -> string
     * This builtin read the clientname from a pipe
     */
    return Y2WFMComponent::instance ()->GetClientName (filedescriptor);
}

static YCPValue 
WFMArgs ()
{
    /**
     * @builtin Args() -> list
     * Returns the arguments with which the module was called.
     * It is a list whose
     * arguments are the module's arguments. If the module
     * was called with <tt>CallModule("my_mod",&nbsp;[17,&nbsp;true])</tt>,
     * &nbsp;<tt>Args()</tt> will return <tt>[ 17, true ]</tt>.
     */
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
    /**
     * @builtin GetLanguage() -> string
     * Returns the current language code (without modifiers !)
     */
    return Y2WFMComponent::instance ()->GetLanguage ();
}


static YCPString 
WFMGetEncoding ()
{
    /**
     * @builtin GetEncoding() -> string
     * Returns the current encoding code
     */
    return Y2WFMComponent::instance ()->GetEncoding ();
}


static YCPString 
WFMSetLanguage (const YCPString& language)
{
    /**
     * @builtin SetLanguage("de_DE" [, encoding]) -> "<proposed encoding>"
     * Selects the language for translate()
     * @example SetLanguage("de_DE@euro", "ISO-8859-1") -> ""
     * @example SetLanguage("de_DE@euro") -> "ISO-8859-15"
     * if the encoding isn't specified, it's set to "UTF-8"
     * The "<proposed encoding>" is the output of 'nl_langinfo (CODESET)'
     * and only given if SetLanguage() is called with a single argument.
     */
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



WFM::WFM ()
{
    y2debug( "registering WFM builtins" );
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "WFM",		"",				NULL, 		DECL_NAMESPACE },
	{ "SCROpen",		"integer (string, boolean)",	(void*)WFMSCROpen },
	{ "SCRClose",		"void (integer)",		(void*)WFMSCRClose },
	{ "SCRGetName",		"string (integer)",		(void*)WFMSCRGetName },
	{ "SCRSetDefault",	"void (integer)",		(void*)WFMSCRSetDefault },
	{ "SCRGetDefault",	"integer ()",			(void*)WFMSCRGetDefault },
	{ "CallFunction",	"any (string, list <any>)",	(void*)WFMCallFunction1 },
	{ "CallFunction",	"any (string)",			(void*)WFMCallFunction },
	{ "call",		"any (string, list <any>)",	(void*)WFMCallFunction1 },
	{ "call",		"any (string)",			(void*)WFMCallFunction },
	{ "GetClientName",	"string (integer)",		(void*)WFMGetClientName },
	{ "Args",		"list <any> ()",		(void*)WFMArgs },
	{ "Args",		"any (integer)",		(void*)WFMArgs2 },
	{ "GetLanguage",	"string ()",			(void*)WFMGetLanguage },
	{ "GetEncoding",	"string ()",			(void*)WFMGetEncoding },
	{ "SetLanguage",	"string (string)",		(void*)WFMSetLanguage },
	{ "SetLanguage",	"string (string, string)",	(void*)WFMSetLanguage2 },
	{ "Read",		"any (path,any)",		(void*)WFMRead },
	{ "Write",		"boolean (path, any, any)",	(void*)WFMWrite3},
	{ "Write",		"boolean (path, any)",		(void*)WFMWrite2},
	{ "Execute",		"any (path, any)",		(void*)WFMExecute},
	{ 0 }
    };

    static_declarations.registerDeclarations ("WFM", declarations);
}

