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

   File:       YCPBasicInterpreter.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

/*
 * Basic interpreter for YCP
 *
 */

#ifndef YCPBasicInterpreter_h
#define YCPBasicInterpreter_h

#include <map>
#include "YCPIdentifier.h"
#include "YCPScope.h"
#include "ycp/y2log.h"

class YCPDebugger;

/**
 * @short Base class of all YCP interpreters
 * The class YCPBasicInterpreter is the core interpreter of YCP.
 * It handles the evaluation of blocks
 * (control structures), macro definitions and substitutions,
 * variable definitions and assignments. Common operators
 * such as the addition of numbers and many other builtings
 * are implemented in the
 * subclass @ref YCPInterpreter, since these are not part
 * of the YCP core language.
 */

class YCPBasicInterpreter : public YCPScope
{
private:
    /**
     * name of the currently evaluated module (block).
     * normally empty.
     */
    string moduleName;

public:
    /**
     * YCP define being evaluated.
     */
    string current_func;

    /**
     * Backtrace information
     */
    struct CallFrame
    {
	string func;
	int line;
	string file;
	CallFrame (string fu, int li, string fi):
	    func (fu), line (li), file (fi) {}
/**/}; /* Trick ydoc to continue parsing */

    /**
     * Backtrace information
     */
    vector<CallFrame> backtrace;

    /**
     * Creates a new YCPBasicInterpreter.
     */
    YCPBasicInterpreter();

    /**
     * Base class must have virtual destructor.
     */
    virtual ~YCPBasicInterpreter();

    /**
     * Evaluates a YCP value.
     * @param value value to be evaluated
     * @return the evaluation
     */
    YCPValue evaluate(const YCPValue& value);

    /**
     * Evaluates a YCP value in UI context
     * @param value value to be evaluated
     * @return the evaluation
     */
    virtual YCPValue evaluateUI(const YCPValue& value);

    /**
     * Evaluates a YCP value in SCR context
     * @param value value to be evaluated
     * @return the evaluation
     */
    virtual YCPValue evaluateSCR(const YCPValue& value);

    /**
     * Evaluates a YCP value in WFM context
     * @param value value to be evaluated
     * @return the evaluation
     */
    virtual YCPValue evaluateWFM(const YCPValue& value);

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
     * Set the current module name.
     * all declarations and definitions go to this scope.
     * !! set only during evaluatation of a module block !!
     */
    YCPValue setModuleName (const string& modulename);

protected:
   /**
     * Override this method to implement term evaluation. When the
     * interpreter executes a @ref YCPEvaluationStatementRep with
     * a term to evaluate, it first evaluates the terms arguments
     * and then call this method.
     */
    virtual YCPValue evaluateInstantiatedTerm(const YCPTerm& term);

    /**
     * Evaluates a locale. Evaluate _("string") to "string".
     * Usually overlaid in WFM for translations.
     */
    virtual YCPValue evaluateLocale(const YCPLocale&);

    /**
     * This method is overridden by @ref YCPInterpreter in order to
     * define many builtin functions such as number and string arithmetic.
     */
    virtual YCPValue evaluateBuiltinBuiltin (builtin_t code, const YCPList& args);

    /**
     * This method is overridden by @ref YCPInterpreter in order to
     * define many builtin functions.
     */
    virtual YCPValue evaluateBuiltinTerm (const YCPTerm& term);

    /**
     * This method might be overridden by any subclassed interpreter
     */
    virtual YCPValue includeFile (const string& filename);
    virtual YCPValue importModule (const string& modulename);

    /**
     * This method might be overridden by any subclassed interpreter
     */
    virtual YCPValue setTextdomain (const string& domainname);
    virtual string getTextdomain (void);

private:

    /**
     * Evaluates a builtin.
     * These are ``, &&, ||, =, (declaration), _debug,
     * _dump_scope, and _dump_meminfo
     */
    YCPValue evaluateBuiltin (const YCPBuiltin& builtin);

    /**
     * Evaluates a term. Quoted terms
     * like `Id(17+2) are evaluated to themselves with evaluated
     * arguments -> `Id(19). Other terms are evaluated first
     * by calling @ref #evaluateInstantiatedTerm , then by
     * calling @ref #evaluateBuiltinTerm .
     */
    YCPValue evaluateTerm(const YCPTerm& term);

    /**
     * evaluates a symbol, that is the symbol is considered
     * to be a variable, whose values is to be returned.
     */
    YCPValue evaluateSymbol(const YCPSymbol&);

    /**
     * evaluates an identifier (as a variable). Lookup the identifier
     * in the given (named) scope and return its value.
     */
    YCPValue evaluateIdentifier(const YCPIdentifier&) const;

    /**
     * Evaluates a list. The evaluation of a list is the list of
     * the evaluations of its members.
     */
    YCPValue evaluateList(const YCPList&);

    /**
     * Evaluates a map. The evaluation of a map is the map containing
     * of the evaluations of the values.
     */
    YCPValue evaluateMap(const YCPMap&);

    /**
     * Evaluates a declaration. Only the complex declarations
     * DeclStruct and DeclTerm must be evaluated, because they
     * contain subexpression.
     */
    YCPValue evaluateDeclaration(const YCPDeclaration&);

    /**
     * Evaluates a YCPDeclStructRep. It must be
     * evaluated, because the declarations it contains may
     * be given in form of a term
     */
    YCPValue evaluateDeclStruct(const YCPDeclStruct&);

    /**
     * Evaluates a YCPDeclTermRep.  It must be
     * evaluated, because the declarations it contains may
     * be given in form of a term, for example float|integer
     * is a term %or(float, integer) that evaluates into
     * a YCPDeclAndOrRep.
     */
    YCPValue evaluateDeclTerm(const YCPDeclTerm&);

    /**
     * Evaluates a block. Handles gotos and labels.
     */
    YCPValue evaluateBlock(const YCPBlock&);

    /**
     * Executes an assignment.
     */
    YCPValue evaluateAssign(const YCPList& args);

    /**
     * Executes a bracket assignment.
     */
    YCPValue evaluateBracketAssign(const YCPBuiltin& builtin);

    /**
     * Executes a bracket expression.
     */
    YCPValue evaluateBracket(const YCPBuiltin& builtin);

    /**
     * Defines a macro substitution.
     * @param decl A definition term
     */
    YCPValue evaluateDefine (bool global, const YCPList& args);

    /**
     * Looks through all macro definitions and tries to find one that
     * matches the given term and evaluates if, if existent. Otherwise
     * returns 0.
     * @param term Term to be evaluated
     */
    YCPValue evaluateDefinition(const YCPTerm& term);

    /**
     * Evaluates the logical && and || operators. The reason,
     * why this is handled here, is that we want a non-strict operator
     * semantic: The second argument may only be evaluated, if the result
     * of the term is not clear from the first one. true || x is always true,
     * regardless of x. x must not be evaluated in this case.
     * @param f1 first boolean value
     * @param f2 second boolean value
     * @param op_is_and true, if and operation should be performed, false if or.
     */
    YCPValue evaluateLogicalAndOr(const YCPValue& f1, const YCPValue& f2, bool op_is_and);

    /**
     * The method implements the builtin function _dump_meminfo,
     * which writes some memory informations to y2log.
     */
    YCPValue dumpMeminfo(const YCPList& args) const;

    /**
     * Executes a declaration statement.
     * Assigns a value to a variable and optionally declares the variable
     * before this. A variable _must_ be declared at its first assignment.
     * A run time error is triggered, if an undeclared variable is assigned
     * or referenced or if a variable is redeclared.
     * @param symbol Name of the variable
     * @param value Value that should be assigned
     * @param decl Restriction of type and range of the value this variable
     * can hold.
     */
    YCPValue evaluateDeclare(bool global, const YCPList& args);

    /**
     * Give @ref YCPBlockRep access to evaluateDeclare
     */
    friend class YCPBlockRep;

    /**
     * Give the debugger access to everything.
     */
    friend class YCPDebugger;

    /**
     * Debugger for all interpreters.
     */
    static YCPDebugger *debugger;

    /**
     * Activate the debugger
     * @param signum signal number
     */
    static void debuggerSignalHandler (int signum);
};


#endif // YCPBasicInterpreter_h
