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

   File:       YCPWhileBlock.cc

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




// YCPWhileBlock


YCPWhileBlockRep::YCPWhileBlockRep(const YCPBlock& block, const YCPValue& condition)
    : YCPBlockRep(block->getStatements())
    , condition(condition)
{
    
}


YCPBlockType YCPWhileBlockRep::blocktype() const
{
    return YB_WHILE;
}


YCPValue YCPWhileBlockRep::getCondition() const
{
    return condition;
}


bool YCPWhileBlockRep::handleBlockHead(YCPBasicInterpreter *interpreter) const
{
    const YCPValue c = interpreter->evaluate(condition);
    if (!c->isBoolean())
    {
	y2error("Error in while condition: %s does evaluate to "
	      "%s, not to a boolean value",
	      condition->toString().c_str(), c->toString().c_str());
	return false;
    }
    bool r = c->asBoolean()->value();
    return r;
}


bool YCPWhileBlockRep::handleBlockEnd(YCPBasicInterpreter *) const
{
    return true;
}


void YCPWhileBlockRep::handleBreak(bool& do_break) const
{
    do_break = false;
}


bool YCPWhileBlockRep::handleContinue(bool& do_continue, int& program_counter) const
{
    program_counter = 0;
    do_continue = false;
    return true;
}


//compare WhileBlocks
YCPOrder YCPWhileBlockRep::compare(const YCPWhileBlock& b) const
{
    YCPOrder order = YO_EQUAL;

    order = condition->compare( b->condition );
    
    if ( order != YO_EQUAL ) return order;
    else                     return compare_statements( b );
}


// get the block as string
string YCPWhileBlockRep::toString() const
{
    return "while (" + condition->toString() + ") " + YCPBlockRep::toString();
}
