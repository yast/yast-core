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

   File:       YCPCode.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPCode_h
#define YCPCode_h


#include "ycp/YCPValue.h"
#include "ycp/YCode.h"
#include "ycp/SymbolEntry.h"

/**
 * @short Wrapper for YCode as YCPValue
 */
class YCPCodeRep : public YCPValueRep
{
    YCode *m_code;

protected:
    friend class YCPCode;

    /**
     * Constructor
     */
    YCPCodeRep();
    YCPCodeRep (YCode *c);

    /**
     * Cleans up.
     */
    ~YCPCodeRep() {}

public:
    YCode *code() const;

    /**
     * Compares two YCodes for equality, greaterness or smallerness.
     * 
     */
    YCPOrder compare (const YCPCode &v) const;

    /**
     * Returns an ASCII representation of the YCode.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_CODE. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;

    /**
     * Evaluates the code.
     */
    YCPValue evaluate (bool cse = false) const;
};


/**
 * @short Wrapper for YCPCodeRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPCodeRep
 * with the arrow operator. See @ref YCPCodeRep.
 */
class YCPCode : public YCPValue
{
    DEF_COMMON(Code, Value);
public:
    YCPCode() : YCPValue (new YCPCodeRep()) {}
    YCPCode (YCode *code) : YCPValue (new YCPCodeRep (code)) {}
    YCPCode (std::istream & str);
};


//-------------------------------------------------------------
/**
 * @short Wrapper for Break as YCPValue
 *
 * YCPBreak is used as a result value in YCode->evaluate
 * in order to distinguish a normal statement/block (returning
 * a value (return <expr>) or YCPVoid  (return)) from a 'break'
 *
 */
class YCPBreakRep : public YCPValueRep
{
protected:
    friend class YCPBreak;

    /**
     * Constructor
     */
    YCPBreakRep () {};

    /**
     * Cleans up.
     */
    ~YCPBreakRep() {}

public:
    /**
     * Compares two YBreaks for equality, greaterness or smallerness.
     * 
     */
    YCPOrder compare (const YCPBreak &v) const { return YO_EQUAL; }

    /**
     * Returns an ASCII representation of the YBreak.
     *   Executing 'break;' always results in YCPNull() 
     */
    string toString() const { return "nil"; }

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const { return str; };

    /**
     * Returns YT_BREAK. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const { return YT_BREAK; }

};

/**
 * @short Wrapper for YCPBreakRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBreakRep
 * with the arrow operator. See @ref YCPBreakRep.
 */
class YCPBreak : public YCPValue
{
    DEF_COMMON(Break, Value);
public:
    YCPBreak() : YCPValue (new YCPBreakRep()) {}
    YCPBreak (std::istream & str) : YCPValue (new YCPBreakRep()) {}
};


//-------------------------------------------------------------
/**
 * @short Wrapper for Return as YCPValue
 *
 * YCPReturn is used as a result value in YCode->evaluate
 * in order to distinguish a normal statement/block (resulting
 * in YCPVoid) from a 'return;' (resulting in YCPReturn)
 * A return with value 'return <expr>;' statement results
 * in a YCPValue (!= YCPVoid() since 'return nil;' is not allowed)
 *
 */
class YCPReturnRep : public YCPValueRep
{
protected:
    friend class YCPReturn;

    /**
     * Constructor
     */
    YCPReturnRep() {}

    /**
     * Cleans up.
     */
    ~YCPReturnRep() {}

public:
    /**
     * Compares two YReturns for equality, greaterness or smallerness.
     * 
     */
    YCPOrder compare (const YCPReturn &v) const { return YO_EQUAL; }

    /**
     * Returns an ASCII representation of the YReturn.
     */
    string toString() const { return ""; }

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const { return str; };

    /**
     * Returns YT_BREAK. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const { return YT_RETURN; }

};

/**
 * @short Wrapper for YCPReturnRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPReturnRep
 * with the arrow operator. See @ref YCPReturnRep.
 */
class YCPReturn : public YCPValue
{
    DEF_COMMON(Return, Value);
public:
    YCPReturn() : YCPValue(new YCPReturnRep()) {}
    YCPReturn(std::istream & str) : YCPValue(new YCPReturnRep()) {}
};

/**
 * @short Wrapper for YEntry as YCPValue
 */
class YCPEntryRep : public YCPValueRep
{
    SymbolEntry *m_entry;

protected:
    friend class YCPEntry;

    /**
     * Constructor
     */
    YCPEntryRep ();
    YCPEntryRep (SymbolEntry *entry);

    /**
     * Cleans up.
     */
    ~YCPEntryRep() {}

public:
    SymbolEntry *entry() const;

    /**
     * Compares two YCPEntrys for equality, greaterness or smallerness.
     * 
     */
    YCPOrder compare (const YCPEntry &v) const;

    /**
     * Returns an ASCII representation of the YCPEntry.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_ENTRY. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;

    /**
     * Evaluates the code.
     */
    YCPValue evaluate (bool cse = false) const;
};


/**
 * @short Wrapper for YCPEntryRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPEntryRep
 * with the arrow operator. See @ref YCPEntryRep.
 */
class YCPEntry : public YCPValue
{
    DEF_COMMON(Entry, Value);
public:
    YCPEntry (SymbolEntry *entry) : YCPValue(new YCPEntryRep(entry)) {}
    YCPEntry (std::istream & str);
};

#endif   // YCPCode_h
