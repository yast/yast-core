/*-----------------------------------------------------------*- c++ -*-\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                    (C) SUSE LINUX AG |  
\----------------------------------------------------------------------/ 

   File:       MemUsage.h
   Author:     Martin Vidner <mvidner@suse.cz>

$Id$

/-*/

#ifndef MemUsage_h
#define MemUsage_h

#include <set>


/**
 * This is the master switch for MemUsage
 */
#undef D_MEMUSAGE

/**
 * Counts instances of classes
 * If you want to count a class, derive it form this one.
 * Then call MuDump in gdb.
 * Suggestions are welcome.
 */
class MemUsage
{
protected:
    typedef std::set <MemUsage *> data;
    static data* m_mu_instances;

    MemUsage () {
	if ( ! m_mu_instances ) 
	{
	    m_mu_instances = new data;
	}
	m_mu_instances->insert (this);
    }

    virtual ~MemUsage () {
	m_mu_instances->erase (this);
    }
public:
    //! dump all classes and nuber of their instances
    static void MuDump ();
    //! for a given class, dump its instances' addresses,
    //  ready to be printed in gdb
    static void MuDumpVal (const char *name);

    virtual size_t mem_size () const { return sizeof (*this); }
};

// this makes it easier for gdb.
void MuDump ();
void MuDumpVal (const char *name);

#endif
