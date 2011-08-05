/*-----------------------------------------------------------*- c++ -*-\
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

   File:	YBreakpoint.h

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef YBreakpoint_h
#define YBreakpoint_h

#include <string>
using std::string;

#include "ycp/YCode.h"
#include "ycp/YCodePtr.h"

#include "ycp/YCPValue.h"
#include "ycp/YCPString.h"
#include "ycp/Type.h"
#include "ycp/YSymbolEntry.h"

/**
 * \brief YBreakpoint wrapper for YCP debugger
 *
 * A class representing a breakpoint in YCP code.
 */
class YBreakpoint : public YCode
{
    REP_BODY(YBreakpoint);
    
    YCodePtr m_code;
    bool m_enabled;
    string m_name;
public:

    /**
     * Creates a new YCode element
     */
    YBreakpoint (YCodePtr wrapped_ycp, string name);

    /**
     * Destructor
     */
    virtual ~YBreakpoint();

    /**
     * Kind of this \ref YCode.
     *
     * \return the YCode kind
     */
    virtual ykind kind() const { return yiBreakpoint; }
   
    /**
     * Return ASCII represtation of this YCP code. Actually no-op.
     * 
     * \return ASCII string representation
     */
    virtual string toString() const;

    /**
     * Write YCP code to a byte stream (bytecode implementation). No-op.
     */
    virtual std::ostream & toStream (std::ostream & str) const;

    /**
     * Write YCP code as XML representation. No-op.
     * \param str	string stream to store into
     * \param indend	indentation level for pretty print
     * \return 		string stream for chaining writing XML (str)
     */
    virtual std::ostream & toXml (std::ostream & str, int indent ) const;

    /**
     * Is this code constant?
     *
     * \return true if the \ref YCode represents a constant
     */
    virtual bool isConstant () const;

    /**
     * Is this a YCP statement (e.g. if, while, ...)
     *
     * \return true if the \ref YCode represents a statement
     */
    virtual bool isStatement () const;

    /**
     * Is this a YCP block?
     *
     * \return true if the \ref YCode represents a block
     */
    virtual bool isBlock () const;

    /**
     * Can this code be stored in a variable of a type reference?
     *
     * \return true if the \ref YCode represents something we can reference to
     */
    virtual bool isReferenceable () const;

    /**
     * Execute YCP code to get the resulting \ref YCPValue. Every inherited class of YCode should reimplement
     * this method.
     * 
     * \return \ref YCPValue after executing the code
     * \param cse  should the evaluation be done for parse time evaluation (i.e. constant subexpression elimination)
     */
    virtual YCPValue evaluate (bool cse = false);

    /**
    * Return type of this YCP code (interesting mostly for function calls).
    *
    * \return type of the value to be returned after calling \ref evaluate
    */
    virtual constTypePtr type () const;
  
    YCodePtr code () const;
    
    bool enabled () const;
    void setEnabled (bool enable);
};

#endif   // YBreakpoint_h
