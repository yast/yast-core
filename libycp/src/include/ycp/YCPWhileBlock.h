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

   File:       YCPWhileBlock.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPWhileBlock_h
#define YCPWhileBlock_h


#include "YCPBlock.h"




/**
 * @short YCPBlockRep representing a while (cond) { ... } block.
 */
class YCPWhileBlockRep : public YCPBlockRep
{
    /**
     * An YCP expression describing the while condition.
     */
    const YCPValue condition;

protected:
    friend class YCPWhileBlock;

    /**
     * Converts a block into a while block
     */
    YCPWhileBlockRep(const YCPBlock& block, const YCPValue& condition);

    /**
     * Cleans up
     */ 
    ~YCPWhileBlockRep() {}

public:
    /**
     * Returns YB_WHILE. See @ref YCPBlockRep#blocktype.
     */
    YCPBlockType blocktype() const;

    /**
     * Returns, which condition this block has
     */
    YCPValue getCondition() const;

    /**
     * Checks the condition. Returns false, if it is false, since
     * the block must be left then
     */
    bool handleBlockHead(YCPBasicInterpreter * interpreter) const;

    /**
     * Sets the program counter to the beginning of the block
     * and calls handleBlockHead() and returns that return value.
     */
    bool handleBlockEnd(YCPBasicInterpreter * interpreter) const;

    /**
     * Returns false, but leaves do_break false, because the
     * break has been handled already.
     */
    void handleBreak(bool& do_break) const;

    /**
     * Does the same as @ref handleBlockEnd.
     */
    bool handleContinue(bool& do_continue, int& program_counter) const;

    /**
     * Compares two YCPWhileBlocks for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the conditions
     *   If conditions are equal ompare the statements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPWhileBlock &v) const;

    /**
     * Returns a string representation of this block in YCP syntax.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPWhileBlockRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPWhileBlockRep
 * with the arrow operator. See @ref YCPWhileBlockRep.
 */
class YCPWhileBlock : public YCPBlock
{
    DEF_COMMON(WhileBlock, Block);
public:
    YCPWhileBlock(const YCPBlock& block, const YCPValue& condition)
	: YCPBlock(new YCPWhileBlockRep(block, condition)) {}
};

#endif   // YCPWhileBlock_h
