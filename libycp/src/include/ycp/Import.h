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
#include <stack>
using std::string;

#include "ycp/YCode.h"
#include "ycp/SymbolTable.h"
#include "ycp/YSymbolEntry.h"

class SymbolEntry;
class Y2Namespace;

/// instantiate to import a module
class Import {
    // track tables of nested imports, no need to track their usage

    static int m_disable_tracking;
    static std::stack <std::pair <string, SymbolTable *> > m_table_stack;
public:
    static void disableTracking ();
    static void enableTracking ();

public:
    // module block pointer, pointer to constructor
    // map of name : module_entry
    typedef std::map<std::string, Y2Namespace *> module_map;


protected:
    static module_map m_active_modules;

    Ustring m_name;

    // iterator to share the module state on multiple imports
    module_map::iterator m_module;

public:
    Import ();

    /**
     * loads module by name.
     * @note if import failed it set {Import::name()} to {SymbolEntry::emptyUstring}
     * @see Import::import for parameters and details
     */
    Import (const string &name, Y2Namespace *name_space = 0);
    ~Import ();

    /**
     * imports namespace into component system. Used when default
     * constructor is used, so delayed import is done.
     * @param[in] name of namespace to import
     * @param[in] preloaded_namespace if namespace is already preloaded,
     * so it only register it and next call of import of this namespace will
     * just return this preloaded_namespace. It take ownership of this namespace
     * @return 0 if done (successfully or not), -1 if we should retry
     */
    int import (const string &name, Y2Namespace *preloaded_namespace = 0);

    string name () const;
    Y2Namespace *nameSpace () const;		// return NULL on failure
};

#endif   // Import_h
