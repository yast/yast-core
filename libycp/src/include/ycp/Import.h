/*---------------------------------------------------------*- c++ -*---\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                          Copyright (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	Import.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef Import_h
#define Import_h

#include <string>
using std::string;

#include "ycp/YCode.h"
#include "ycp/SymbolTable.h"

class SymbolEntry;
class Y2Namespace;

class Import {
public:
    // module block pointer, pointer to constructor
    typedef struct {
	Y2Namespace *name_space;	// name_space defining the module, non-const since it might get evaluated
	const SymbolEntry *constructor;	// pointer to constructor (NULL if no constructor)
	bool activated;			// true if block already evaluated
    } module_entry;

    // map of name : module_entry
    typedef std::map<std::string, module_entry> module_map;


protected:
    static module_map m_active_modules;

    string m_name;

    // iterator to share the module state on multiple imports
    module_map::iterator m_module;

public:
    // load module by name. If block != 0, it's already loaded
    //   name_space is non-const since it might get evaluated
    Import (const string &name, Y2Namespace *name_space = 0, bool from_stream = false);
    ~Import ();
    string name () const;
    Y2Namespace *nameSpace () const;		// return NULL on failure
};

#endif   // Import_h
