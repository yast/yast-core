/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 execution environment, i.e. processing context.
 *   Contains reference to the current block, the current statement, 
 *   the current file name and backtrace.
 *   This information can be used for logging, debugger etc.
 *
 * Authors:
 *   Stanislav Visnovsky <visnov@suse.cz>
 * $Id$
 */

#include "ycp/ExecutionEnvironment.h"

#include "ycp/YStatement.h"
#include "ycp/YBlock.h"

#include "ycp/y2log.h"

struct CallFrame {
    string called_function;
    string filename;
    int linenumber;
    
    CallFrame (string f, int l, string func): 
	called_function (func),
	filename (f),
	linenumber (l)
    {}
};


int
ExecutionEnvironment::linenumber () const
{
    return m_linenumber;
}


void
ExecutionEnvironment::setLinenumber (int line)
{
    m_linenumber = line;
}


const string 
ExecutionEnvironment::filename () const
{
    return (m_forced_filename || ! block()) ? m_filename : block()->filename();
}


void
ExecutionEnvironment::setFilename (const string & filename)
{
    m_filename = filename;
    m_forced_filename = true;
    return;
}


YStatement* 
ExecutionEnvironment::statement () const
{
    return m_statement;
}


void
ExecutionEnvironment::setStatement (YStatement* s)
{
    m_statement = s;
    
    if (s != NULL)
    {
	m_linenumber = s->line ();
    }
    
    return;
}


YBlock* 
ExecutionEnvironment::block () const
{
    if (m_blocks.empty())
    {
	return NULL;
    }
    return m_blocks.top ();
}


void
ExecutionEnvironment::pushBlock (YBlock* b)
{
    m_blocks.push (b);
    if (b->isFile()
	|| b->isModule())
    {
	setFilename (b->filename());
    }
    else
    {
	m_forced_filename = false;
    }
    return;
}


void
ExecutionEnvironment::popBlock ()
{
    if (!m_blocks.empty())
    {
	y2debug ("ExecutionEnvironment::popBlock ()");
	m_blocks.pop ();
    }
    m_forced_filename = false;
    return;
}


void
ExecutionEnvironment::pushframe (string called_function)
{
    y2debug ("Push frame %s", called_function.c_str ());
    m_backtrace.push (new CallFrame (filename(), linenumber (), called_function));
}


void
ExecutionEnvironment::popframe ()
{
    y2debug ("Pop frame %p", m_backtrace.top ());
    delete m_backtrace.top ();
    m_backtrace.pop ();
}


string
ExecutionEnvironment::backtrace (uint omit)
{
    return "not implemented";
}

/* EOF */
