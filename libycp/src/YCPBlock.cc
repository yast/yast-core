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

   File:       YCPBlock.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * Part of basic interpreter that evaluates blocks
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#define INNER_DEBUG 0

#include "y2log.h"
#include "YCPScope.h"
#include "YCPBasicInterpreter.h"
#include "YCPDebugger.h"


YCPBlockRep::YCPBlockRep()
{
}

YCPBlockRep::YCPBlockRep(const string name)
    : module_name(name)
{
}

YCPBlockRep::YCPBlockRep(const vector<YCPStatement> &statements)
    : statements(statements)
{
}


YCPBlockRep::YCPBlockRep(const string name, const vector<YCPStatement> &statements)
    : module_name(name)
    , statements(statements)
{
}


// see comment of equal() and compare()
YCPBlockType YCPBlockRep::blocktype() const
{
    return YB_PLAIN;
}


// block type checking

bool YCPBlockRep::isWhileBlock()   const { return blocktype() == YB_WHILE; }
bool YCPBlockRep::isDoWhileBlock() const { return blocktype() == YB_DOWHILE; }
bool YCPBlockRep::isLoopBlock()    const { return (isWhileBlock() || isDoWhileBlock ()); }


// block type conversions

YCPWhileBlock YCPBlockRep::asWhileBlock() const
{
    if ( ! isWhileBlock() )
    {
	y2error ("Invalid cast of YCP block %s! Should be but is not WhileBlock!\n",
	      toString().c_str());
	abort();
    }
    return YCPWhileBlock(static_cast<const YCPWhileBlockRep *>(this)); 
}


YCPDoWhileBlock YCPBlockRep::asDoWhileBlock() const
{
    if ( ! isDoWhileBlock() )
    {
	y2error ("Invalid cast of YCP block %s! Should be but is not DoWhileBlock!\n",
	      toString().c_str());
	abort();
    }
    return YCPDoWhileBlock(static_cast<const YCPDoWhileBlockRep *>(this)); 
}


void YCPBlockRep::add(const YCPStatement& s)
{
    statements.push_back(s);
}


YCPStatement YCPBlockRep::statement(int i) const
{
    return statements[i];
}


int YCPBlockRep::size() const
{
    return statements.size();
}


// comparison functions

YCPOrder YCPBlockRep::compare(const YCPBlock& b) const
{
    if (blocktype() == b->blocktype())
    {
	switch ( blocktype() )
	{
	case YB_PLAIN:
	    /*
	     * This is not consistent with the handling of subtypes in
	     * YCPdeclarations or YCPStatements. In this case YCPBlock
	     * is _NOT_ only a base class and therefore is instantiated
	     * for itself. This way we can not cast further to call
	     * methods of the subtype but have to do all work here
	     * in place.
	     * ==> maybe there should be a YCPPlainBlock class.
	     */
	    return this->compare_statements( b );
	case YB_WHILE:
	    return this->asWhileBlock()->compare(b->asWhileBlock());
	case YB_DOWHILE:
	    return this->asDoWhileBlock()->compare(b->asDoWhileBlock());
	default:
	    y2error ("Sorry, comparison of %s with %s not yet implemented",
		  toString().c_str(), b->toString().c_str());
	    return YO_EQUAL;
	}
    }
    else return blocktype() < b->blocktype() ? YO_LESS : YO_GREATER;
}


YCPOrder YCPBlockRep::compare_statements(const YCPBlock& b) const
{
    int size_this  = size();
    int size_b     = b->size();
    YCPOrder order = YO_EQUAL;

    if ( size_this != 0 || size_b != 0 )
    {
	// any one is not empty ( maybe both ) ==> shorter is less
	if ( size_this < size_b )      return YO_LESS;
	else if ( size_this > size_b ) return YO_GREATER;
	else
	{
	    // equal length ==> pairwise comparison
	    for( int i = 0; i < size_this; i++ )
	    {
		// compare statements
		order = statement(i)->compare( b->statement(i) );
		if ( order == YO_LESS || order == YO_GREATER ) return order;
	    }

	    // no difference found in equal lengths ==> equal
	    return YO_EQUAL;
	}
    }
    else return YO_EQUAL;   // both are empty 
}


string YCPBlockRep::toString() const
{
    string ret = "{\n";
    if (!module_name.empty())
    {
	ret += "  module \"" + module_name + "\";\n";
    }
    for (unsigned i=0; i<statements.size(); i++)
    {
	ret += "  " + statements[i]->toString() + "\n";
    }
    return ret + "}\n";
}


YCPValueType YCPBlockRep::valuetype() const
{
    return YT_BLOCK;
}


bool YCPBlockRep::handleBlockInit(YCPBasicInterpreter *) const
{
    return true;
}

bool YCPBlockRep::handleBlockHead(YCPBasicInterpreter *) const
{
    return true;
}

bool YCPBlockRep::handleBlockEnd(YCPBasicInterpreter *) const
{
    return false;
}

void YCPBlockRep::handleBreak(bool& do_break) const
{
    do_break = true;
}

bool YCPBlockRep::handleContinue(bool& do_continue, int&) const
{
    do_continue = true;
    return false;
}


// The most interesting function: evaluation of a block

YCPValue YCPBlockRep::evaluate(YCPBasicInterpreter *interpreter, 
			    bool& do_break, bool& do_continue, bool& do_return) const
{
    static int nested_loop = 0;

    do_break = do_continue = do_return = false;
    int program_counter = 0;
    YCPValue return_value = YCPNull();

    if (isLoopBlock())
    {
	nested_loop++;
    }

    // Open a new (named ?) scope before evaluating the block.

    string name_cache = interpreter->current_file;
    if (!module_name.empty())
    {
	interpreter->setModuleName (module_name);
	if (!interpreter->createInstance (module_name))
	{
	    // module already loaded
	    // silently ignore circular dependency
	    return YCPVoid();
	}
	// the following openScope() is *inside* the new Instance !
    }

    interpreter->openScope();

    if (!file_name.empty())
    {
        interpreter->current_file = file_name;
    }

    string old_textdomain;

    if (handleBlockInit(interpreter)) while (return_value.isNull())
    {
	if ((program_counter == 0) && (!handleBlockHead(interpreter)))
	{
	    break;
	}

	if (program_counter >= size())
	{
	    if (!handleBlockEnd(interpreter))
	    {
		break;
	    }
	    program_counter = 0;
	    continue;
	}

	const YCPStatement s(statement(program_counter));
	
#if INNER_DEBUG
	y2debug ("evaluateBlock %d (%s)", interpreter->scopeLevel, s->toString().c_str());
#endif
	
	interpreter->current_line = s->linenumber();
//	interpreter->reportError(LOG_WARNING, "Trace: %s", s->toString().c_str());
	
	if (interpreter->debugger)
	{
	    interpreter->debugger->debug (interpreter, YCPDebugger::Block, s);
	}
	
	if ( s->isBreakStatement() )
	{
	    handleBreak(do_break);
	    if (nested_loop <= 0)
	    {
		interpreter->reportError (LOG_ERROR, "BL break only allowed in loop blocks");
	    }
	    break;
	}
	else if ( s->isContinueStatement() )
	{
	    if (nested_loop <= 0)
	    {
		interpreter->reportError (LOG_ERROR, "BL continue only allowed in loop blocks");
	    }
	    if (!handleContinue(do_continue, program_counter))
	    {
		break;
	    }
	    else
	    {
		continue;
	    }
	}
	else if ( s->isReturnStatement() )
	{
	    do_return = true;
	    YCPReturnStatement r = s->asReturnStatement();

	    return_value = YCPVoid();	// default value

	    if (!(r->value().isNull()))
	    {
		return_value = interpreter->evaluate(r->value());
	    }

	    if (return_value.isNull())
	    {
		interpreter->reportError (LOG_ERROR, "bad return expr\n");
	    }
#if INNER_DEBUG
	    else
	    {
y2debug ("block returns %s\n", return_value->toString().c_str());
	    }
#endif
	    break;
	}
	else
	{
	    // check for localdomain builtin at start of block
	    // (switches textdomain temporarely)

	    if ((program_counter == 0)
		 && (s->isBuiltinStatement())
		 && (s->asBuiltinStatement()->code() == YCPB_LOCALDOMAIN))
	    {
		string new_textdomain = s->asBuiltinStatement()->value()->asString()->value();
		string now_textdomain = interpreter->getTextdomain ();
		if (new_textdomain != now_textdomain)
		{
		    old_textdomain = now_textdomain;
		    interpreter->setTextdomain (new_textdomain);
		}
		program_counter++;
		continue;
	    }

	    // evaluate current statement

	    return_value = evaluateStatement(s, interpreter, do_break, do_continue, do_return);
	    if (do_return)
	    {
#if INNER_DEBUG
y2debug ("statement do_return: %s\n", return_value.isNull()?"NULL":return_value->toString().c_str());
#endif
		break;
	    }
	    if (!return_value.isNull() && return_value->isError())
	    {
		return_value = interpreter->evaluate (return_value);
	    }

	    return_value = YCPNull();

	    // check for (nested) break/continue statements

	    if (do_break)
	    {
		handleBreak(do_break);
		break;
	    }
	    else if (do_continue)
	    {
		if (!handleContinue(do_continue, program_counter))
		{
		    break;
		}
		continue;
	    }
	}

	program_counter++;
    } // while()


    // close the block/module scope

    if (module_name.empty())
    {
	// only if *not* module
	interpreter->closeScope();
    }
    else
    {
	YCPValue constructor = interpreter->lookupValue(module_name, module_name);
	if (!constructor.isNull())
	{
	    interpreter->evaluate (constructor);
	}

	// keep the local scope open, it's part of the instance.

	// close the instance, don't delete it.
	interpreter->closeInstance (module_name);
	interpreter->setModuleName ("");
    }

    interpreter->current_file = name_cache;

    // back to old textdomain if localdomain statement switched it

    if (!old_textdomain.empty())
    {
	interpreter->setTextdomain (old_textdomain);
    }    

    if (isLoopBlock())
    {
	nested_loop--;
    }
    
#if INNER_DEBUG
    y2debug ("block result %d (%s)", interpreter->scopeLevel, return_value.isNull() ? "nil" :
	     return_value->toString().c_str());
#endif
    return return_value.isNull() ? YCPVoid() : return_value;
}


YCPValue YCPBlockRep::evaluateStatement(const YCPStatement& st, 
				     YCPBasicInterpreter *interpreter, 
				     bool& do_break, bool& do_continue, bool& do_return) const
{
    switch (st->statementtype())
    {
    case YS_IFTHENELSE:
    {
	const YCPIfThenElseStatement ite = st->asIfThenElseStatement();
	const YCPValue should_i = interpreter->evaluate(ite->condition());
	bool take_then_branch;
	if (should_i.isNull())
	{
	    return should_i;
	}
	if (!should_i->isBoolean())
	{
	    if (should_i->isError())
	    {
		interpreter->evaluate (should_i);
	    }
	    else
	    {
		interpreter->reportError (LOG_ERROR, "condition %s in if statement evaluates to %s,"
		  " but only true and false are allowed", 
		  ite->condition()->toString().c_str(), should_i->toString().c_str());
	    }
	    take_then_branch = false;
	}
	else
	{
	    take_then_branch = should_i->asBoolean()->value();
	}

	const YCPBlock block = take_then_branch ? ite->thenBlock() : ite->elseBlock();
	if (!block.isNull())
	{
	    return block->evaluate(interpreter, do_break, do_continue, do_return);
	}
	else
	{
	    return YCPNull();
	}
    }
    break;
    case YS_EVALUATION:
    {
	// Handle blocks in a different way in order to 
	// handle break, continue and return correctly
	const YCPEvaluationStatement es = st->asEvaluationStatement();
	if (es->isSubBlock())
	{
	    const YCPBlock block = es->value()->asBlock();
	    return block->evaluate(interpreter, do_break, do_continue, do_return);
	}
	else
	{
	    const YCPValue v = es->value();
	    return interpreter->evaluate(v);
	}
    }
    break;
    case YS_NESTED:
    {
	// Handle sub-blocks (to have a nested scope)
	const YCPBlock block = st->asNestedStatement()->value();
	return block->evaluate(interpreter, do_break, do_continue, do_return);
    }
    break;
    case YS_BUILTIN:
    {
	// Handle builtins
	const builtin_t code = st->asBuiltinStatement()->code();
	const YCPValue value = st->asBuiltinStatement()->value();
	switch (code)
	{
	    case YCPB_INCLUDE:
	    {
		// restore current textdomain if changed inside include
		string old_textdomain = interpreter->getTextdomain ();
		YCPValue v = interpreter->includeFile (value->asString()->value());
		interpreter->setTextdomain (old_textdomain);
		return v;
	    }
	    break;
	    case YCPB_IMPORT:
	    {
		// restore current textdomain if changed inside include
		string old_textdomain = interpreter->getTextdomain ();
		YCPValue v = interpreter->importModule (value->asString()->value());
		interpreter->setTextdomain (old_textdomain);
		return v;
	    }
	    break;
	    case YCPB_TEXTDOMAIN:
	    {
		return interpreter->setTextdomain (value->asString()->value());
	    }
	    break;
	    case YCPB_LOCALDOMAIN:
	    {
	    }
	    break;
	    case YCPB_UNDEFINE:
	    {
		YCPList symbollist = value->asList();
		for (int i = 0; i < symbollist->size(); i++)
		{
		    interpreter->removeSymbol (symbollist->value(i)->asSymbol()->symbol());
		}
		return YCPVoid();
	    }
	    break;
     	    case YCPB_FULLNAME:
	    {
		interpreter->current_file = value->asString()->value();
		return YCPVoid();
	    }
	    break;
	    default:
	    {
		return YCPError (string ("Builtin '")
				 + st->asBuiltinStatement()->toString()
				 + "' not implemented");
	    }
	    break;
	}
    }
    break;
    default:
	return YCPError (string ("Unknown statement [") + st->toString() + "]");
	break;
    }
    return YCPNull();
}
