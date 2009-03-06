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

// the number of call frames to show warning at
#define WARN_RECURSION 1001
static const char * Y2RECURSIONLIMIT = "Y2RECURSIONLIMIT";


ExecutionEnvironment::ExecutionEnvironment ()
    : m_filename ("")
    , m_forced_filename (false)
    , m_statement(NULL)
{
    m_backtrace.clear ();

    m_recursion_limit = 0;
    char * s = getenv (Y2RECURSIONLIMIT);
    if (s != NULL)
	m_recursion_limit = atoi (s);
    if (m_recursion_limit == 0)
	m_recursion_limit = WARN_RECURSION;
}

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

bool
ExecutionEnvironment::endlessRecursion ()
{
    if (m_backtrace.size () == m_recursion_limit)
    {
	y2error ("Recursion limit of %zd call frames reached. Set the environment variable %s to change this", m_recursion_limit, Y2RECURSIONLIMIT);
	return true;
    }
    return false;
}

void
ExecutionEnvironment::pushframe (string called_function)
{
    y2debug ("Push frame %s", called_function.c_str ());
    CallFrame* frame = new CallFrame (filename(), linenumber (), called_function);
    m_backtrace.push_back (frame);
    // backtrace( LOG_MILESTONE, 0 );
}


void
ExecutionEnvironment::popframe ()
{
    y2debug ("Pop frame %p", m_backtrace.back ());
    const CallFrame* frame = m_backtrace.back ();
    m_backtrace.pop_back ();
    // backtrace( LOG_MILESTONE, 0 );
    delete frame;
}


void
ExecutionEnvironment::backtrace (loglevel_t level, uint omit) const
{
    if (m_backtrace.empty ())
	return;
	
    // FIXME: omit
    CallStack::const_reverse_iterator it = m_backtrace.rbegin();

    y2logger(level, "------------- Backtrace begin -------------");
    
    while (it != m_backtrace.rend())
    {
	ycp2log (level, (*it)->filename.c_str (), (*it)->linenumber
		 , "", "%s", (*it)->called_function.c_str ());
	++it;
    };

    y2logger(level, "------------- Backtrace end ---------------");
}


ExecutionEnvironment::CallStack ExecutionEnvironment::callstack() const
{
    // backtrace( LOG_MILESTONE, 0 );
    return m_backtrace;
}

/* EOF */
