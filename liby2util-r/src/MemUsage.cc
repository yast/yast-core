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

   File:       MemUsage.cc
   Author:     Martin Vidner <mvidner@suse.cz>

$Id$

/-*/

#include "y2util/MemUsage.h"
#include <stdlib.h>
#include <map>
#include <string>
#include <cstdio>
#include <typeinfo>
#include <cxxabi.h>

// top init priority, because all objects derived from MemUsage use it
MemUsage::data* MemUsage::m_mu_instances = 0;

// http://www.codesourcery.com/cxx-abi/abi.html#demangler
static std::string demangle (const char *sym)
{
    std::string ret;
    int status;
    char *dsym = abi::__cxa_demangle (sym, NULL, NULL, &status);
    ret = status == 0? dsym: sym;
    free (dsym);
    return ret;
}

void MemUsage::MuDump ()
{
    fprintf (stderr, "MemUsage dump:\n");
    std::map <std::string, int> m_mu_count;
    std::map <std::string, int> m_mu_size;
    // determine the type of each instance. now it is possible
    // because we are not in the constructor
    data::iterator
	ii = m_mu_instances->begin (),
	ie = m_mu_instances->end ();
    for (; ii != ie; ++ii)
    {
	const char * name = typeid (**ii).name ();
	std::string dename = demangle (name);
	if (m_mu_size.find (dename) == m_mu_size.end())
	{
	    m_mu_size[dename] = (**ii).mem_size();
	}
	++ m_mu_count[dename];
    }

    std::map <std::string, int>::iterator
	i = m_mu_count.begin (),
	e = m_mu_count.end ();
    unsigned long sum = 0;
    for (; i != e; ++i)
    {
	int size = m_mu_size[i->first];
	unsigned long mem = i->second * size;
	sum += mem;
	fprintf (stderr, "%9d <%5d> [%9lu] %s\n", i->second, size, mem, i->first.c_str ());
    }
    fprintf (stderr, "%9lu Total bytes\n", sum);

}

// for gdb copy and paste convenience
void MemUsage::MuDumpVal (const char *aname)
{
    data::iterator
	ii = m_mu_instances->begin (),
	ie = m_mu_instances->end ();
    for (; ii != ie; ++ii)
    {
	std::string dname = demangle (typeid (**ii).name ());
	if (dname == aname)
	{
	    fprintf (stderr, "p *(%s *)%p\n", aname, *ii);
	}
    }
}

void MuDump ()
{
    MemUsage::MuDump ();
}

void MuDumpVal (const char *name)
{
    MemUsage::MuDumpVal (name);
}

