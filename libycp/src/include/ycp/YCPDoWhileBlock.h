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

   File:       YCPDoWhileBlock.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDoWhileBlock_h
#define YCPDoWhileBlock_h


#include "YCPBlock.h"




/**
 * @short YCPBlockRep representing a do/while or repeat/until block.
 * This class can represent either a do ... while block or an
 * repeat ... until block.
 */
class YCPDoWhileBlockRep : public YCPBlockRep
{
    /**
     * true, if this is a repeat until block. It interprets the
     * condition negated.
     */
    bool repeat_until;

    /**
     * An YCP expression describing the condition.
     */
    const YCPValue condition;

protected:
    friend class YCPDoWhileBlock;

    /**
     * Converts a block into a while block
     */
    YCPDoWhileBlockRep(const YCPBlock& block, const YCPValue& condition, bool repeat_until);

    /**
     * Cleans up
     */
    ~YCPDoWhileBlockRep() {}

public:
    /**
     * Returns YB_DOWHILE. See @ref YCPBlockRep#blocktype.
     */
    YCPBlockType blocktype() const;

    /**
     * Returns, which condition this block has
     */
    YCPValue getCondition() const;

    /**
     * Determines, whether this block is a repeat..until block (true), or
     * a do..while block (false)
     */
    bool isRepeatUntil() const;

    /**
     * Checks to condition. If it is true (or false in a
     * repeat...until block), sets the program counter to the beginning of the block.
     * Otherwise return false.
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
     * Compares two YCPDoWhileBlocks for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the repeat_until flag (repeat_until is less).
     *   If flags are equal compare the conditions.
     *   If conditions are equal ompare the statements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     *
     */
    YCPOrder compare(const YCPDoWhileBlock &v) const;

    /**
     * Returns a string representation of this block in YCP syntax.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPDoWhileBlockRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDoWhileBlockRep
 * with the arrow operator. See @ref YCPDoWhileBlockRep.
 */
class YCPDoWhileBlock : public YCPBlock
{
    DEF_COMMON(DoWhileBlock, Block);
public:
    YCPDoWhileBlock(const YCPBlock& block, const YCPValue& condition, bool repeat_until)
	: YCPBlock(new YCPDoWhileBlockRep(block, condition, repeat_until)) {}
};

#endif   // YCPDoWhileBlock_h
