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

   File:       YCPDoWhileBlock.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * Part of basic interpreter that evaluates blocks
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPBasicInterpreter.h"





// YCPDoWhileBlock

YCPDoWhileBlockRep::YCPDoWhileBlockRep(const YCPBlock& block, const YCPValue& condition, bool repeat_until)
    : YCPBlockRep(block->getStatements())
    , repeat_until(repeat_until)
    , condition(condition)
{
}


YCPBlockType YCPDoWhileBlockRep::blocktype() const
{
    return YB_DOWHILE;
}


YCPValue YCPDoWhileBlockRep::getCondition() const
{
    return condition;
}


bool YCPDoWhileBlockRep::isRepeatUntil() const
{
    return repeat_until;
}


bool YCPDoWhileBlockRep::handleBlockEnd(YCPBasicInterpreter *interpreter) const
{
    const YCPValue c = interpreter->evaluate(condition);
    if (!c->isBoolean())
    {
	y2error("Error in %s condition: %s does evaluate to "
	      "%s, not to a boolean value",
	      repeat_until ? "until" : "do ... while", 
	      condition->toString().c_str(), c->toString().c_str());
	return false;
    }
    bool r = c->asBoolean()->value();
    return (r != repeat_until);
}


void YCPDoWhileBlockRep::handleBreak(bool& do_break) const
{
    do_break = false;
}


bool YCPDoWhileBlockRep::handleContinue(bool& do_continue, int& program_counter) const
{
    program_counter = 0;
    do_continue = false;
    return true;
}


//compare DoWhileBlocks
YCPOrder YCPDoWhileBlockRep::compare(const YCPDoWhileBlock& b) const
{
    YCPOrder order = YO_EQUAL;

    if ( repeat_until == b->repeat_until )
    {
	order = condition->compare( b->condition );
    
	if ( order != YO_EQUAL ) return order;
	else                     return compare_statements( b );
    }
    else   // repeat_until flags are different
    {
	if ( repeat_until ) return YO_LESS;
	else                return YO_GREATER;
    }
}


// get the block as string
string YCPDoWhileBlockRep::toString() const
{
    if (repeat_until) {
	return "repeat " + YCPBlockRep::toString() 
	    + " until (" + condition->toString() + ");\n";
    }
    else {
	return "do " + YCPBlockRep::toString() 
	    + " while (" + condition->toString() + ");\n";
    }
}
