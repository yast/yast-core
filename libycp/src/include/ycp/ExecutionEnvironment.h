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
 *
 */

#ifndef _execution_environment_h
#define _execution_environment_h

#include <stack>
#include <string>

#include "y2log.h"
#include "ycp/YStatement.h"

using namespace std;

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

/**
 * Class to track current execution environment. Typically used for logging
 * and debugging.
 * 
 * m_forced_filename is a way to enforce a given filename until another
 * block is entered (or the current one is left). Used for include statements,
 * where top level block does not exist.
 */
class ExecutionEnvironment {

public:
    typedef vector<const CallFrame*> CallStack;
    
private:
    int m_linenumber;
    string m_filename;
    bool m_forced_filename;
    YStatementPtr m_statement;
    CallStack m_backtrace;

public:
    ExecutionEnvironment () : m_filename (""), m_forced_filename (false), m_statement(NULL) 
	{ m_backtrace.clear (); };
    ~ExecutionEnvironment() {};

    /**
     * Get the current line number.
     */
    int linenumber () const;

    /**
     * Set the current line number.
     */
    void setLinenumber (int line);

    /**
     * Get the current file name.
     */
    const string filename () const;

    /**
     * Set the current file name for error outputs.
     */
    void setFilename (const string & filename);

    /**
     * Return the currently evaluated statement.
     */
    YStatementPtr statement () const;

    /**
     * Set the currently evaluated statement.
     */
    void setStatement (YStatementPtr s);

    /**
     * Push another call frame to the backtrace stack according to the
     * current information.
     *
     * @param called_function	name of the function to be called at this point
     */
    void pushframe (string called_function);

    /**
     * Pop the top call frame from the backtrace stack.
     */
    void popframe ();
    
    /**
     * Report the current backtrace to log.
     *
     * @param skip	number of the top call frames to be omitted 
     *			from the backtrace
     */
    void backtrace (loglevel_t level, uint skip = 0) const;

    /**
     * Returns a copy of the call stack for debugging etc.
     *
     * The stack itself may safely be modified. The pointers it contains,
     * however, may not.
     **/
    CallStack callstack() const;
};

#endif /* _execution_environment_h */
