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

   File:       Y2CCScript.cc

   Author:     Mathias Kettner <kettner@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Thomas Roelz <tom@suse.de>

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
#include "Y2CCScript.h"
#include "Y2ScriptComponent.h"

#include <ycp/Parser.h>
#include <ycp/pathsearch.h>
#include <ycp/Bytecode.h>

#if KMTRACE
#include "/opt/kde3/include/ktrace.h"
#endif

#include <scr/SCR.h>
#include <UI.h>
#include "WFM.h"

Y2CCScript::Y2CCScript()
    : Y2ComponentCreator(Y2ComponentBroker::SCRIPT)
{
    // I don't bother freeing these regular expressions

    regcomp(&rx1, "^#![[:space:]]*/bin/y2gf[[:space:]]\\+$", 0);
    
//    y2debug ("registering %p", this);
    
}

static void initializeBuiltins ()
{
    // initialize YCP-related builtins WFM, SCR and UI
    // we can't do this in constructor, since it is called in static initialization
    // and we can't control the order of initialization - it can crash
    static SCR scr;
    static UI ui;
    static WFM wfm;
}

Y2Component *Y2CCScript::createInLevel(const char *name, int level, int) const
{
    y2debug ("Trying to create %s", name );
    if (name == 0)
    {
	return 0;
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

	fullname = Y2PathSearch::completeFilename (string (name));
	if (fullname.empty())
	    return 0;

	file = fopen (fullname.c_str(), "r");
	if (!file) return 0; // Not found under the direct path either.

	filename = name;
	// 2nd try: examine the file: Is it not executable or does
	// the name end in .ycp or does the file begin with #!/bin/y2wfm

	bool try_it = false;

	if (strlen(name) > 4 && !strcmp(name + strlen(name) - 4, ".ycp"))
	    try_it = true;
	else {
	    struct stat buf;
	    if (0 == stat(name, &buf))
	    {
		// Try it, if it is not executable
		if (S_ISREG(buf.st_mode) && buf.st_mode & S_IXOTH != S_IXOTH)
		    try_it = true;
	    }
	    else {
		char line[512];
		if (line == fgets(line, 512, file)) {
		    if (0 == regexec(&rx1, line, 0, 0, 0))
			try_it = true;
		}
	    }
	}
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

    if (script->code () != 0)
    {
	Y2ScriptComponent *s = Y2ScriptComponent::instance ();
	s->setupComponent (modulename, fullname, script);

	return s;
    }

    return NULL;
}

bool Y2CCScript::isServerCreator() const
{
    return false;
}

Y2Component* Y2CCScript::provideNamespace(const char* name)
{
    // check the filename
    string filename = YCPPathSearch::findModule (name);
    if (filename.empty())
    {
        y2debug ("YCP module '%s' not found", name);
        return 0;
    }

// this is useless in itself, because then we will have registered builtins
// but no instance to handle them
//    initializeBuiltins ();

    // return the component

    y2debug ("Component to provide the namespace: %p", Y2ScriptComponent::instance ());    
    return Y2ScriptComponent::instance ();
}

// Create global variable to register this component creator

Y2CCScript g_y2ccscript;
