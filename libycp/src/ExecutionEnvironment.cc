/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 execution environment, i.e. processing context.
 *   Contains reference to the current statement, 
 *   the current file name and backtrace.
 *   This information can be used for logging, debugger etc.
 *
 * Authors:
 *   Stanislav Visnovsky <visnov@suse.cz>
 *
 */

#include "ycp/ExecutionEnvironment.h"

#include "ycp/YStatement.h"

#include "ycp/y2log.h"

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
    return m_filename;
}


void
ExecutionEnvironment::setFilename (const string & filename)
{
    m_filename = filename;
    m_forced_filename = true;
    return;
}


YStatementPtr
ExecutionEnvironment::statement () const
{
    return m_statement;
}


void
ExecutionEnvironment::setStatement (YStatementPtr s)
{
    m_statement = s;
    
    if (s != NULL)
    {
	m_linenumber = s->line ();
    }
    
    return;
}


void
ExecutionEnvironment::pushframe (string called_function)
{
    y2debug ("Push frame %s", called_function.c_str ());
    CallFrame* frame = new CallFrame (filename(), linenumber (), called_function);
    m_backtrace.push_back (frame);
}


void
ExecutionEnvironment::popframe ()
{
    y2debug ("Pop frame %p", m_backtrace.back ());
    CallFrame* frame = m_backtrace.back ();
    m_backtrace.pop_back ();
    delete frame;
}


void
ExecutionEnvironment::backtrace (loglevel_t level, uint omit)
{
    if (m_backtrace.empty ())
	return;
	
    // FIXME: omit
    vector<CallFrame*>::iterator it = m_backtrace.end ();
    
    while (it != m_backtrace.begin ())
    {
	it--;
	ycp2log (level, (*it)->filename.c_str (), (*it)->linenumber
	    , "", "%s", (*it)->called_function.c_str ());
    };

}

/* EOF */
