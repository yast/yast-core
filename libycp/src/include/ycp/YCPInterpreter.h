/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPInterpreter.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

/*
 *  interpreter for YCP
 *
 */

#ifndef YCPInterpreter_h
#define YCPInterpreter_h

#include <map>
#include "YCP.h"
#include "ycp/y2log.h"

class YCPDebugger;

/**
 * @short Base class of all YCP interpreters
 * The class YCPInterpreter is the core interpreter of YCP.
 * It handles the evaluation of blocks
 * (control structures), macro definitions and substitutions,
 * variable definitions and assignments. Common operators
 * such as the addition of numbers and many other builtings
 * are implemented in the
 * subclass @ref YCPInterpreter, since these are not part
 * of the YCP core language.
 */

class YCPInterpreter
{
private:
    /**
     * name of the currently evaluated module (block).
     * normally empty.
     */
    const char *m_filename;
 
    /**
     * number of the currently evaluated line
     * normally 0.
     */
    int m_line;

public:
    /**
     * Creates a new YCPInterpreter.
     */
    YCPInterpreter();

    /**
     * Base class must have virtual destructor.
     */
    virtual ~YCPInterpreter();

    /**
     * Evalutes a YCP value.
     * @param value value to be evaluated
     * @return the evaluation
     */
    YCPValue evaluate (const YCPValue& value);
    virtual YCPValue evaluateUI (const YCPValue& value);
    virtual YCPValue evaluateWFM (const YCPValue& value);
    virtual YCPValue evaluateSCR (const YCPValue& value);

    /**
     * Callback evaluate.
     * @param value value to be evaluated
     * @return the evaluation
     */
    virtual YCPValue callback(const YCPValue& value);

    /**
     * A interpreter has a name. Used for debugging, so you can
     * say: "I want to debug the `WFM' interpreter." Default
     * returns "YCP".
     */
    virtual string interpreter_name () const;

    /**
     * report evaluation error
     *
     * The numbers in format are shifted be one since the first argument
     * to a member function is the reference to the object. The argument
     * numbers in the compiler warnings are also shifted.
     */
    void reportError (enum loglevel_t severity, const char *message, ...) const __attribute__ ((format (printf, 3, 4)));

    /**
     * Get the current line number.
     */
    int lineNumber () const;

    /**
     * Set the current line number.
     */
    void setLineNumber (int line);

    /**
     * Get the current file name.
     */
    const char *filename () const;

    /**
     * Set the current file name for error outputs.
     */
    void setFilename (const char *filename);

protected:
   /**
     * Override this method to implement term evaluation. When the
     * interpreter executes a @ref YCPEvaluationStatementRep with
     * a term to evaluate, it first evaluates the terms arguments
     * and then call this method.
     */
    virtual YCPValue evaluateInstantiatedTerm(const YCPTerm& term);

    /**
     * This method is overridden by @ref YCPInterpreter in order to
     * define many builtin functions.
     */
    virtual YCPValue evaluateBuiltinTerm (const YCPTerm& term);

private:

    /**
     * Evaluates a YCode.
     */
    YCPValue evaluateCode(const YCPCode&);

    /**
     * The method implements the builtin function _dump_meminfo,
     * which writes some memory informations to y2log.
     */
    YCPValue dumpMeminfo(const YCPList& args) const;

    /**
     * Give the debugger access to everything.
     */
    friend class YCPDebugger;

    /**
     * Debugger for all interpreters.
     */
    static YCPDebugger *debugger;

};


#endif // YCPInterpreter_h
