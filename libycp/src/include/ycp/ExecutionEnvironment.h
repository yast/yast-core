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
 * $Id$
 */

#ifndef _execution_environment_h
#define _execution_environment_h

#include <stack>
#include <string>

using namespace std;

class YBlock;
class YStatement;

struct CallFrame;

/**
 * Class to track current execution environment. Typically used for logging
 * and debugging.
 * 
 * m_forced_filename is a way to enforce a given filename until another
 * block is entered (or the current one is left). Used for include statements,
 * where top level block does not exist.
 */
class ExecutionEnvironment {
private:
    int m_linenumber;
    string m_filename;
    bool m_forced_filename;
    YStatement* m_statement;
    stack<CallFrame*> m_backtrace;
    stack<YBlock*> m_blocks;

public:
    ExecutionEnvironment () : m_forced_filename (false), m_statement(NULL) {};
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
    YStatement* statement () const;

    /**
     * Set the currently evaluated statement.
     */
    void setStatement (YStatement* s);

    /**
     * Return the currently evaluated block. 
     */
    YBlock* block () const;

    /**
     * Set the currently evaluated block.
     */
    void pushBlock (YBlock* b);
    
    /**
     * Remove a block from stack after its evaluation.
     */
    void popBlock ();

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
     * Return the string containing the current backtrace.
     *
     * @param skip	number of the top call frames to be omitted 
     *			from the backtrace
     */
    string backtrace (uint skip = 0);
};

#endif /* _execution_environment_h */
