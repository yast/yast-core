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

/// Function and source location, for backtraces
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
    /**
     * There is a limit of 1001 call frames (overridable by
     * Y2RECURSIONLIMIT in the environment). After that, a call is
     * skipped and nil is returned instead.
     */
    size_t m_recursion_limit;

public:
    ExecutionEnvironment ();
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
     * Report error if there are too many stack frames
     */
    bool endlessRecursion ();

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
