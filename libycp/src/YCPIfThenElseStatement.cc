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

   File:       YCPIfThenElseStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPIfThenElseStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPIfThenElseStatement.h"




// YCPIfThenElseStatementRep

YCPIfThenElseStatementRep::YCPIfThenElseStatementRep(int lineno,
		const YCPValue& cond, const YCPBlock& then_block,    
		const YCPBlock& else_block)
    : YCPStatementRep (lineno)
    , cond(cond)
    , then_block(then_block)
    , else_block(else_block)
{
}


YCPStatementType YCPIfThenElseStatementRep::statementtype() const
{
    return YS_IFTHENELSE;
}


YCPValue YCPIfThenElseStatementRep::condition() const
{
    return cond;
}


YCPBlock YCPIfThenElseStatementRep::thenBlock() const
{
    return then_block;
}


YCPBlock YCPIfThenElseStatementRep::elseBlock() const
{
    return else_block;
}


//compare IfThenElseStatements
YCPOrder YCPIfThenElseStatementRep::compare(const YCPIfThenElseStatement& s) const
{
    YCPOrder order = YO_EQUAL;

    order = cond->compare( s->cond );
    if ( order == YO_EQUAL ) order = then_block->compare( s->then_block );
    if ( order == YO_EQUAL ) order = else_block->compare( s->else_block );

    return order;
}


// get the statement as string
string YCPIfThenElseStatementRep::toString() const
{
    return "if (" + cond->toString() + ") " + then_block->toString()
	   + ((else_block.isNull()) ? "" : (" else " + else_block->toString()));
}


