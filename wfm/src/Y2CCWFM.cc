/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2CCWFM.cc

   Author:     Mathias Kettner <kettner@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Component Creator that executes YCP script via wfm
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#define KMTRACE 0

#include <config.h>
#include <unistd.h>
#include <regex.h>
#include <sys/stat.h>

#include <ycp/y2log.h>
#include <Y2CCWFM.h>
#include <Y2WFMComponent.h>
#include <y2/Y2ErrorComponent.h>


#include <ycp/Parser.h>
#include <ycp/pathsearch.h>
#include <ycp/Bytecode.h>
#include <ycp/YCPCode.h>

#if KMTRACE
#include "/opt/kde3/include/ktrace.h"
#endif

#include <scr/SCR.h>
#include "WFM.h"

Y2CCWFM::Y2CCWFM()
    : Y2ComponentCreator(Y2ComponentBroker::SCRIPT)
{    
}

static void initializeBuiltins ()
{
    // initialize YCP-related builtins WFM, SCR and UI
    // we can't do this in constructor, since it is called in static initialization
    // and we can't control the order of initialization - it can crash
    static SCR scr;
    static WFM wfm;
}

Y2Component *Y2CCWFM::createInLevel(const char *name, int level, int) const
{
    y2debug ("Trying to create %s", name );
    if (name == 0)
    {
	return 0;
    }
    
    if ( strcasecmp ( name, "wfm" ) == 0 )
    {
	y2debug ("Creating just the component WFM");
	return Y2WFMComponent::instance ();
    }
    
    string modulename = name;
    string filename = string(name) + ".ycp";
    FILE *file = 0;

    // try to find clients/<name>.ycp

    string fullname = Y2PathSearch::findy2 ("clients/" + filename, R_OK,
					    level);

    if (fullname.empty())
    {
	// not found "clients/<name>.ycp"
	// try plain name
	// only if the name contains a slash, #330965#c10
	if (!strchr (name, '/'))
	    return 0;

	// we have to keep completeFilename because it also does :: translation :(
	fullname = Y2PathSearch::completeFilename (string (name));
	if (fullname.empty())
	    return 0;

	file = fopen (fullname.c_str(), "r");
	if (!file) return 0; // Not found under the direct path either.

	filename = name;
	// 2nd try: examine the file: does the name end in .ycp

	bool try_it = false;

	if (strlen(name) > 4 && !strcmp(name + strlen(name) - 4, ".ycp"))
	    try_it = true;
	// The stat code that used to be here had a bug
	// in operator precedence rendering it useless. let's make it explicit.
	if (!try_it) return 0;

	modulename = string(name);
	string::size_type slashpos = modulename.rfind('/');
	if (slashpos != string::npos) modulename = modulename.substr(slashpos + 1);
	string::size_type dotpos = modulename.find('.');
	if (dotpos != string::npos) modulename = modulename.substr(0, dotpos);
    }
    else
    {
	file = fopen (fullname.c_str(), "r");
	if (!file)
	    return 0;	// shouldn't happen since findy2() already checked
    }

#if KMTRACE
    ktrace();
#endif

    // to be on the safe side
    initializeBuiltins ();
    
    // check, if there is a newer YBC client
    YCPCode script;
    
    string ybc_filename = YCPPathSearch::bytecodeForFile (fullname);
    if (ybc_filename.empty ())
    {
	// Parse Script
	Parser parser(file, fullname.c_str());
	parser.setBuffered();

	script = YCPCode( parser.parse() );
	fclose(file);
    
	y2milestone ("Parsing finished");
    }
    else
    {
	y2milestone ("Using bytecode file %s", ybc_filename.c_str ());
	script = YCPCode ( Bytecode::readFile (ybc_filename) );
	y2milestone ("Bytecode file loaded");
    }
    
#if KMTRACE
    kuntrace ();
#endif

    if (script->code () != 0 && !script->code ()->isError ())
    {
	Y2WFMComponent *s = Y2WFMComponent::instance ();
	s->setupComponent (modulename, fullname, script);
	return s;
    }

    // NULL would mean not found and the component broker would keep trying
    // which means it would ignore errors in y2update (#330656)
    return new Y2ErrorComponent ();
}

bool Y2CCWFM::isServerCreator() const
{
    return false;
}

Y2Component* Y2CCWFM::provideNamespace(const char* name)
{
    // first, check if we should provide System namespace
    if (strstr (name, "System::") == name)
    {
	return Y2WFMComponent::instance ();

    }
    
    // check the filename
    string filename = YCPPathSearch::findModule (name);
    if (filename.empty())
    {
        y2debug ("YCP module '%s' not found", name);
        return 0;
    }

// this is useless in itself, because then we will have registered builtins
// but no instance to handle them
    // But see bug 37338 - this helps for code paths which do not reach UI
    initializeBuiltins ();

    y2debug ("Component to provide the namespace: %p", Y2WFMComponent::instance ());    
    return Y2WFMComponent::instance ();
}

Y2Component *
Y2CCWFM::create (const char *name) const
{
    if (!strcasecmp(name, "wfm"))
    {
	return Y2WFMComponent::instance ();
    }
    else
	return 0;
}


Y2CCWFM g_y2ccWFM;
