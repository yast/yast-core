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

   File:	YStatement.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#include <libintl.h>

#include "ycp/YStatement.h"
#include "ycp/YExpression.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPTerm.h"
#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

#include "y2/Y2Component.h"
#include "y2/Y2ComponentBroker.h"

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif


// ------------------------------------------------------------------

IMPL_DERIVED_POINTER(YStatement, YCode);
IMPL_DERIVED_POINTER(YSBreak, YCode);
IMPL_DERIVED_POINTER(YSContinue, YCode);
IMPL_DERIVED_POINTER(YSExpression, YCode);
IMPL_DERIVED_POINTER(YSBlock, YCode);
IMPL_DERIVED_POINTER(YSReturn, YCode);
IMPL_DERIVED_POINTER(YSTypedef, YCode);
IMPL_DERIVED_POINTER(YSFunction, YCode);
IMPL_DERIVED_POINTER(YSAssign, YCode);
IMPL_DERIVED_POINTER(YSVariable, YCode);
IMPL_DERIVED_POINTER(YSBracket, YCode);
IMPL_DERIVED_POINTER(YSIf, YCode);
IMPL_DERIVED_POINTER(YSWhile, YCode);
IMPL_DERIVED_POINTER(YSRepeat, YCode);
IMPL_DERIVED_POINTER(YSDo, YCode);
IMPL_DERIVED_POINTER(YSTextdomain, YCode);
IMPL_DERIVED_POINTER(YSInclude, YCode);
IMPL_DERIVED_POINTER(YSImport, YCode);
IMPL_DERIVED_POINTER(YSFilename, YCode);
IMPL_DERIVED_POINTER(YSSwitch, YCode);

// ------------------------------------------------------------------
// statement (-> statement, next statement)

YStatement::YStatement (int line)
    : YCode ()
    , m_line (line)
{
}


YStatement::YStatement (bytecodeistream & str)
    : YCode ()
{
    m_line = Bytecode::readInt32 (str);
#if DO_DEBUG
    y2debug ("YStatement::YStatement([%d]:%d)", (int)kind, m_line);
#endif
}


string
YStatement::toString () const
{
    return "<<undefined statement>>";
}


YCPValue
YStatement::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("YStatement::evaluate(%s)\n", toString().c_str());
#endif
    return YCPNull();
}


std::ostream &
YStatement::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("YStatement::toStream ([%d]:%d)", (int)kind(), m_line);
#endif
    YCode::toStream (str);
    return Bytecode::writeInt32 (str, m_line);
}

std::ostream &
YStatement::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << "<statement line=" << m_line ;
    commentToXml(str);
    return str << "/>";
}

// ------------------------------------------------------------------
// "break"

YSBreak::YSBreak (int line)
    : YStatement (line)
{
}


YSBreak::YSBreak (bytecodeistream & str)
    : YStatement (str)
{
}


string
YSBreak::toString () const
{
    return "break;";
}


YCPValue
YSBreak::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("YSBreak::evaluate(%s)\n", toString().c_str());
#endif
    return YCPBreak();
}


std::ostream &
YSBreak::toStream (std::ostream & str) const
{
    return YStatement::toStream (str);
}

std::ostream &
YSBreak::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<break";
    commentToXml(str);
    return str << "/>";
}

// ------------------------------------------------------------------
// "continue"

YSContinue::YSContinue (int line)
    : YStatement (line)
{
}


YSContinue::YSContinue (bytecodeistream & str)
    : YStatement (str)
{
}


string
YSContinue::toString () const
{
    return "continue;";
}


YCPValue
YSContinue::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("YSContinue::evaluate(%s)\n", toString().c_str());
#endif
    return YCPVoid();		// special meaning!
}


std::ostream &
YSContinue::toStream (std::ostream & str) const
{
    return YStatement::toStream (str);
}

std::ostream &
YSContinue::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<continue";
    commentToXml(str);
    return str << "/>";
}

// ------------------------------------------------------------------
// expression as statement

YSExpression::YSExpression (YCodePtr expr, int line)
    : YStatement (line)
    , m_expr (expr)
{
}


YSExpression::YSExpression (bytecodeistream & str)
    : YStatement (str)
{
    m_expr = Bytecode::readCode (str);
}


YSExpression::~YSExpression ()
{
}


string
YSExpression::toString() const
{
    string s = m_expr->toString();
    if (!m_expr->isBlock())
    {
	s += ";";
    }
    return s;
}


std::ostream &
YSExpression::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return m_expr->toStream (str);
}


std::ostream &
YSExpression::toXml( std::ostream & str, int indent ) const
{
    if (comment_before)
      m_expr->setCommentBefore(comment_before);
    if (comment_after)
      m_expr->setCommentAfter(comment_after);
    m_expr->toXml( str, indent );
    comment_after = comment_before = NULL;
    return str;
}


YCPValue
YSExpression::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("YSExpression::evaluate(%s:%d)\n", toString().c_str(), line());
#endif
    m_expr->evaluate (cse);
    return YCPNull();
}


// ------------------------------------------------------------------
// block as statement

YSBlock::YSBlock (YBlockPtr block, int line)
    : YStatement (line)
    , m_block (block)
{
    m_block->setKind (YBlock::b_statement);
}


YSBlock::YSBlock (bytecodeistream & str)
    : YStatement (str)
{
    m_block = Bytecode::readCode (str);
}


YSBlock::~YSBlock ()
{
}


string
YSBlock::toString() const
{
    string s = m_block->toString();
    return s;
}


std::ostream &
YSBlock::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return m_block->toStream (str);
}


std::ostream &
YSBlock::toXml (std::ostream & str, int indent ) const
{
    return m_block->toXml( str, indent );
}


YCPValue
YSBlock::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("YSBlock::evaluate(%s:%d)\n", toString().c_str(), line());
#endif
    return m_block->evaluate (cse);
}


// ------------------------------------------------------------------
// return

YSReturn::YSReturn (YCodePtr value, int line)
    : YStatement (line)
    , m_value (value)
{
}


YSReturn::YSReturn (bytecodeistream & str)
    : YStatement (str)
    , m_value (0)
{
    if (Bytecode::readBool (str))
    {
	m_value = Bytecode::readCode (str);
    }
}


YSReturn::~YSReturn ()
{
}


YCodePtr 
YSReturn::value() const
{
    return m_value;
}


// clear value to prevent deletion of the YCode when
// this return statement is deleted.
// This is used if YBlock::justReturn triggers and the
// YBlock is converted to a YEReturn which re-uses the value
// and deletes the block.

void
YSReturn::clearValue()
{
    m_value = 0;
    return;
}



// propagate return value to new type
void
YSReturn::propagate (constTypePtr from, constTypePtr to)
{
    if (m_value != 0)
    {
	m_value = new YEPropagate (m_value, from, to);
    }
    return;
}


string
YSReturn::toString() const
{
    string s = "return";
    if (m_value != 0)
    {
	s += " ";
	s += m_value->toString();
    }
    s += ";";
    return s;
}


std::ostream &
YSReturn::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    if (m_value == 0)
    {
	return Bytecode::writeBool (str, false);
    }
    Bytecode::writeBool (str, true);
    return m_value->toStream (str);
}


std::ostream &
YSReturn::toXml (std::ostream & str, int indent ) const
{
    str << Xmlcode::spaces( indent ) << "<return";
    commentToXml(str);
    str << ">";
    if (m_value != 0)
        m_value->toXml( str, 0 );
    return str << "</return>";
}


YCPValue
YSReturn::evaluate (bool cse)
{
    if (cse) return YCPNull();
#if DO_DEBUG
    y2debug ("YSReturn::evaluate (%s)\n", m_value ? m_value->toString().c_str() : "");
#endif
    if (m_value != 0)
    {
	YCPValue val = m_value->evaluate (cse);

	if (!val.isNull()
	    && !val->isVoid())		// treat 'return nil;' as 'return;'
	{
	    return val;
	}
    }
    // don't return YCPVoid() here since YBlock() needs YCPReturn to distinguish
    // a normal statement (returning YCPVoid) from the 'return' statement which
    // ends the block
    return YCPReturn();
}


// ------------------------------------------------------------------
// function definition

YSFunction::YSFunction (YSymbolEntryPtr entry, int line)
    : YStatement (line)
    , m_entry (entry)
{
}


YSFunction::~YSFunction ()
{
    // don't delete m_entry here, it belongs to SymbolTable
}


SymbolEntryPtr
YSFunction::entry() const
{
    return m_entry;
}


YFunctionPtr
YSFunction::function() const
{
    return (YFunctionPtr)(m_entry->code());
}


string
YSFunction::toString() const
{
    return m_entry->toString() + "\n" + ((YFunctionPtr)(m_entry->code()))->definition()->toString();
}


YCPValue
YSFunction::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("YSFunction::evaluate(%s)\n", toString().c_str());
#endif
    // there's nothing to evaluate for a function _definition_
    // its all in the function call.
    return YCPNull();
}


YSFunction::YSFunction (bytecodeistream & str)
    : YStatement (str)
    , m_entry (Bytecode::readEntry (str))
{
    if (!m_entry->isGlobal())
    {
	m_entry->setCode (Bytecode::readCode (str));
    }
    function()->setDefinition (str);
}


std::ostream &
YSFunction::toStream (std::ostream & str) const
{
    YStatement::toStream (str);

    Bytecode::writeEntry (str, m_entry);
    if (!m_entry->isGlobal())
    {
	function()->toStream (str);	// write function parameters
    }
    return function()->toStreamDefinition (str);	// write function definition
}


std::ostream &
YSFunction::toXml( std::ostream & str, int indent ) const
{
    str << "<fun_def name=\"" << m_entry->name() << "\"";
    function()->commentToXml(str);
    str << ">\n";

    function()->toXml( str, indent+2 );

    return str << Xmlcode::spaces( indent ) << "</fun_def>";
}


// ------------------------------------------------------------------
// typedef (-> type string)

YSTypedef::YSTypedef (const string &name, constTypePtr type, int line)
    : YStatement (line)
    , m_name (Ustring (*SymbolEntry::_nameHash, name))
    , m_type (type)
{
}


YSTypedef::YSTypedef (bytecodeistream & str)
    : YStatement (str)
    , m_name (Bytecode::readUstring (str))
{
    m_type = Bytecode::readType (str);
}


string
YSTypedef::toString() const
{
    string s = string ("typedef ") + m_type->toString() + " " + m_name.asString() + ";";
    return s;
}


std::ostream &
YSTypedef::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeUstring (str, m_name);
    return m_type->toStream (str);
}


std::ostream &
YSTypedef::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << "<typedef name=\"" << m_name << "\"";
    commentToXml(str);
    m_type->toXml( str, 0 );
    return str << "/>";
}


// FIXME: needed ?

YCPValue
YSTypedef::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug("evaluate(%s) = nil", toString().c_str());
#endif
    return YCPNull();
}

// ------------------------------------------------------------------
// assignment

YSAssign::YSAssign (SymbolEntryPtr entry, YCodePtr code, int line)
    : YStatement (line)
    , m_entry (entry)
    , m_code (code)
{
}


YSAssign::YSAssign (bytecodeistream & str)
    : YStatement (str)
{
    m_entry = Bytecode::readEntry (str);
    m_code = Bytecode::readCode (str);
}


YSAssign::~YSAssign ()
{
    // don't delete m_entry here, it belongs to SymbolTable
}


string
YSAssign::toString () const
{
    return m_entry->toString (false /*definition*/)
    	+ " = "
	+ m_code->toString()
	+ ";";
}


std::ostream &
YSAssign::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    return m_code->toStream (str);
}


std::ostream &
YSAssign::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << "<assign name=\"" << m_entry->name() << "\"";
    commentToXml(str);
    string ns = m_entry->nameSpace()->name();
    if (!ns.empty())
      str << " ns=\"" << ns << "\"";
    str << ">";
    m_code->toXml( str, 0 );
    return str << "</assign>";
}


YCPValue
YSAssign::evaluate (bool cse)
{
    if (cse) return YCPNull();
#if DO_DEBUG
    y2debug ("YSAssign::evaluate(%s)\n", toString().c_str());
#endif
    YCPValue value = m_code->evaluate ();
    m_entry->setValue (value.isNull() ? YCPVoid() : value);
#if DO_DEBUG
    y2debug ("YSAssign::evaluate (%s) = '%s'\n", m_code->toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif

    return YCPNull();
}


// ------------------------------------------------------------------
// variable definition

YSVariable::YSVariable (SymbolEntryPtr entry, YCodePtr code, int line)
    : YSAssign (entry, code, line)
{
}


YSVariable::YSVariable (bytecodeistream & str)
    : YSAssign (str)
{
    // setup default value
    ((YSymbolEntryPtr)m_entry)->setCode (m_code);
}


YSVariable::~YSVariable ()
{
}


string
YSVariable::toString () const
{
    return ( (m_entry->nameSpace() == 0) ? "global " : "")
	+ m_entry->toString (true /*definition*/)
    	+ " = "
	+ m_code->toString()
	+ ";";
}


// ------------------------------------------------------------------
// bracket assignment
// <entry>[<arg>] = <code>

YSBracket::YSBracket (SymbolEntryPtr entry, YCodePtr arg, YCodePtr code, int line)
    : YStatement (line)
    , m_entry (entry)
    , m_arg (arg)
    , m_code (code)
{
}


YSBracket::~YSBracket ()
{
    // don't delete m_entry here, it belongs to SymbolTable
}


string
YSBracket::toString () const
{
    return m_entry->toString (false)
	+ m_arg->toString()
    	+ " = "
	+ m_code->toString()
	+ ";";
}


YSBracket::YSBracket (bytecodeistream & str)
    : YStatement (str)
{
    m_entry = Bytecode::readEntry (str);
    m_arg = Bytecode::readCode (str);
    m_code = Bytecode::readCode (str);
}


std::ostream &
YSBracket::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    m_arg->toStream (str);
    return m_code->toStream (str);
}


std::ostream &
YSBracket::toXml( std::ostream & str, int indent ) const
{
    str << "<bracket";
    commentToXml(str);
    str << ">";
    str << "<lhs>";
    Xmlcode::writeEntry( str, m_entry );
    str << "<arg>"; m_arg->toXml( str, 0 ); str << "</arg>";
    str << "</lhs><rhs>";
    m_code->toXml( str, 0 );
    str << "</rhs>";
    return str << "</bracket>";
}


// commit bracket assign recursively

YCPValue
YSBracket::commit (YCPValue current, int idx, YCPList arg, YCPValue value)
{
    if (arg.isNull()
	|| (idx > arg->size()))
    {
	return YCPNull();
    }

    if (idx == arg->size())
    {
	return value;
    }

    if (current.isNull ())
    {
	YCPList correct_until;
	
	for (int i = 0 ; i < idx ; i++)
	{
	    correct_until->add (arg->value (i));
	}

	ycp2error ("Intermediate structure with index %s does not exist", correct_until->toString ().c_str ());
	return YCPNull ();
    }
#if DO_DEBUG	
    y2debug ("commit (%s, %d, %s, %s)", current->toString().c_str(), idx, arg->toString().c_str(), value.isNull () ? "nil" : value->toString().c_str());
#endif
    YCPValue argval = arg->value (idx);
    if (argval.isNull())
    {
	ycp2error ("Invalid bracket parameter 'nil'");
	return YCPNull ();
    }

    if (current->isList())
    {
	if (!argval->isInteger())
	{
	    ycp2error ("Invalid bracket parameter for list, expected integer, seen '%s'", argval->toString().c_str());
	    return YCPNull ();
	}

	YCPList list = current->asList();
	int argint = argval->asInteger()->value();
	
	YCPValue val = value;

	//  not the end of the argument list, continue
	if (idx < arg->size ()-1)
	{
	    val = commit (list->value (argint), idx+1, arg, value);		// recurse
	    if (val.isNull ())
	    {
		return val;
	    }
	}
	
	list->set (argint, val.isNull() ? YCPVoid() : val);
#if DO_DEBUG	
    y2debug ("list[%d] = %s -> %s", argint, val->toString().c_str(), list->toString().c_str());
#endif
	return list;
    }
    else if (current->isMap())
    {
	YCPMap map = current->asMap();
	
	YCPValue val = value;
	
	if (idx < arg->size ()-1)
	{
	    val = commit (map->value (argval), idx+1, arg, value);		// recurse
	    if (val.isNull ())
	    {
		return val;
	    }
	}
	map = map->functionalAdd (argval, val.isNull() ? YCPVoid() : val);
#if DO_DEBUG	
    y2debug ("map[%s] = %s -> %s", argval->toString().c_str(), val->toString().c_str(), map->toString().c_str());
#endif
	return map;
    }
    else if (current->isTerm())
    {
	if (!argval->isInteger())
	{
	    ycp2error ("Invalid bracket parameter for term, expected integer, seen '%s'", argval->toString().c_str());
	    return YCPNull ();
	}
	YCPTerm term = current->asTerm();
	int argint = argval->asInteger()->value();
	
	YCPValue val = value;
	
	// not the end of the argument list, continue
	if (idx < arg->size ()-1)
	{
	    val = commit (term->value (argint), idx+1, arg, value);
	    if (val.isNull ())
		return val;
	}
	term->set (argval->asInteger()->value(), val.isNull() ? YCPVoid() : val);
	return term;
    }
    ycp2error ("Bracket assignment '%s'['%s'] = '%s', not to list, map, or term", current->toString().c_str(), argval->toString().c_str(), value->toString().c_str());
    return YCPNull ();
}


YCPValue
YSBracket::evaluate (bool cse)
{
    if (cse) return YCPNull();

    // evaluate other arguments _before_ error checking

    // the bracket arguments
    YCPValue arg_value = m_arg->evaluate ();

    // the rhs of the assignment
    YCPValue newvalue = m_code->evaluate ();

    // now check the variable

    YCPValue result = m_entry->value(); 
    if (result.isNull())
    {
	// initial assignment
	y2internal ("Initial assignment reached, not working ATM");
	result = YCPVoid ();
//FIXME:	m_entry->setValue (m_entry->code() ? m_entry->code()->evaluate () : YCPNull());
//FIXME:	result = m_entry->value();
    }

    // bad variable ?
    if (result.isNull())
    {
	ycp2error ("Assignment not possible: variable %s is not declared", m_entry->toString ().c_str ());
	return YCPNull ();
    }

    // bad bracket argument
    if (arg_value.isNull()
	|| !arg_value->isList())
    {
	ycp2error ("Assignment not possible: bracket %s does not evaluate to list", m_arg->toString ().c_str ());
	return YCPNull ();
    }

    result = commit (result, 0, arg_value->asList(), newvalue.isNull() ? YCPVoid() : newvalue);

    if (!result.isNull())
    {
	m_entry->setValue (result);
#if DO_DEBUG
y2debug ("%s = %s", m_entry->name(), result->toString().c_str());
#endif
	result = YCPNull();
    }

    return result;
}



// ------------------------------------------------------------------
// If-then-else statement (-> bool expr, true statement, false statement)

YSIf::YSIf (YCodePtr a_condition, YCodePtr a_true, YCodePtr a_false, int line)
    : YStatement (line)
    , m_condition (a_condition)
    , m_true (a_true)
    , m_false (a_false)
{
}


YSIf::YSIf (bytecodeistream & str)
    : YStatement (str)
    , m_true (0)
    , m_false (0)
{
    m_condition = Bytecode::readCode (str);
    if (Bytecode::readBool (str))
	m_true = Bytecode::readCode (str);
    if (Bytecode::readBool (str))
	m_false = Bytecode::readCode (str);
}


YSIf::~YSIf ()
{
}


string
YSIf::toString () const
{
    string s = "if (" + m_condition->toString() + ")\n";
    if (m_true != 0)
    {
	s = s + "    " + m_true->toString();
    }
    else
    {
	s += "{ /*EMPTY*/ }";
    }
    if (m_false != 0)
    {
	s += "\nelse\n    ";
	s += m_false->toString();
    }
    return s;
}


std::ostream &
YSIf::toStream (std::ostream & str) const
{
    YStatement::toStream (str);

    m_condition->toStream (str);

    if (m_true)
    {
	Bytecode::writeBool (str, true);
	m_true->toStream (str);
    }
    else
    {
	Bytecode::writeBool (str, false);
    }
    if (m_false)
    {
	Bytecode::writeBool (str, true);
	m_false->toStream (str);
    }
    else
    {
	Bytecode::writeBool (str, false);
    }
    return str;
}


std::ostream &
YSIf::toXml (std::ostream & str, int indent ) const
{
    str << "<if";
    commentToXml(str);
    str << ">";

    m_condition->toXml( str, 0 );

    if (m_true)
    {
	str << endl << Xmlcode::spaces( indent+2 ) << "<then>\n";
	m_true->toXml( str, indent+4 );
	str << endl << Xmlcode::spaces( indent+2 ) << "</then>\n";
	str << Xmlcode::spaces( indent );
    }
    if (m_false)
    {
	str << endl << Xmlcode::spaces( indent+2 ) << "<else>\n";
	m_false->toXml( str, indent+4 );
	str << endl << Xmlcode::spaces( indent+2 ) << "</else>\n";
	str << Xmlcode::spaces( indent );
    }
    return str << "</if>";
}


YCPValue
YSIf::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("YSIf::evaluate(%s)\n", toString().c_str());
#endif
    YCPValue bval = m_condition->evaluate (cse);
    
    if (bval.isNull ())
    {
	ycp2error ("if condition is nil.");
	return YCPNull ();
    }
    
    if (!bval->isBoolean())
    {
	ycp2warning(YaST::ee.filename().c_str(), YaST::ee.linenumber(), "'if (%s)' evaluates to non-boolean '%s' (%s), using 'false' instead.", m_condition->toString().c_str(), bval->toString().c_str(), bval->valuetype_str());
	bval	= YCPBoolean (false);
    }
    if (bval->asBoolean()->value() == true)
    {
	if (m_true != 0)
	{
	    return m_true->evaluate (cse);
	}
    }
    else if (m_false != 0)
    {
	return m_false->evaluate (cse);
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// while-do statement (-> bool condition, loop statement)

YSWhile::YSWhile (YCodePtr condition, YCodePtr loop, int line)
    : YStatement (line)
    , m_condition (condition)
    , m_loop (loop)
{
    if (loop
	&& loop->isBlock())
    {
	YBlockPtr block = loop;
	if (block->isStatement())
	{
	    y2milestone ("Converting statement-block to YSBlock");
	    loop = new YSBlock (loop, line);
	}
    }
}


YSWhile::~YSWhile ()
{
}


string
YSWhile::toString () const
{
    string s = "while (" + m_condition->toString() + ")\n    ";
    if (m_loop != 0)
    {
	s += m_loop->toString();
    }
    else
    {
	s += "{ /* EMPTY */ }";
    }
    return s;
}


YSWhile::YSWhile (bytecodeistream & str)
    : YStatement (str)
    , m_loop (0)
{
    m_condition = Bytecode::readCode (str);
    if (Bytecode::readBool (str))
    {
	m_loop = Bytecode::readCode (str);
    }
}


std::ostream &
YSWhile::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    m_condition->toStream (str);
    Bytecode::writeBool (str, m_loop != 0);
    if (m_loop != 0)
    {
	m_loop->toStream (str);
    }
    return str;
}


std::ostream &
YSWhile::toXml (std::ostream & str, int indent ) const
{
    str << "<while";
    commentToXml(str);
    str << ">\n";
    str << Xmlcode::spaces( indent+2 ) << "<cond>"; m_condition->toXml(str, 0 ); str << "</cond>\n";
    if (m_loop) {
	str << Xmlcode::spaces( indent+2 ) << "<do>";
	m_loop->toXml(str, indent );
	str << Xmlcode::spaces( indent+2 ) << "</do>\n";
    }
    return str << Xmlcode::spaces( indent ) << "</while>";
}


YCPValue
YSWhile::evaluate (bool cse)
{
    if (cse) return YCPNull();

#if DO_DEBUG
    y2debug ("YSWhile::evaluate(%s)\n", toString().c_str());
#endif

    bool first_iteration = true;

    for (;;)
    {
	YCPValue bval = m_condition->evaluate ();
	if (bval.isNull ())
	{
	    ycp2error ("while condition is nil.");
	    return YCPNull ();
	}
    
	if (!bval->isBoolean())
	{
	    ycp2error ("'while (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
	    break;
	}
	else if (bval->asBoolean()->value() == false)
	{
	    break;
	}

	if (m_loop == 0)
	{
	    continue;
	}
	YCPValue lval = YCPNull();

	if (m_loop->isBlock())
	{
	    lval = ((YBlockPtr)m_loop)->evaluate ();
	    if (first_iteration) first_iteration = false;
	}
	else
	{
	    lval = m_loop->evaluate ();
	}

#if DO_DEBUG
	y2debug ("YSWhile::evaluate lval (%s)", lval.isNull() ? "NULL" : lval->toString().c_str());
#endif

	if (lval.isNull())
	{
#if DO_DEBUG
	    y2debug ("isNull");
#endif
	    continue;
	}
	else if (lval->isBreak())	// executed 'break'
	{
#if DO_DEBUG
	    y2debug ("isBreak");
#endif
	    break;
	}
	else if (lval->isReturn())	// executed 'return;' - YCPReturn is also YCPVoid, keep the order of tests!
	{
#if DO_DEBUG
	    y2debug ("isReturn");
#endif
	    return lval;
	}
	else if (lval->isVoid())	// normal block/statement or 'continue'
	{
#if DO_DEBUG
	    y2debug ("isVoid");
#endif
	    continue;
	}
	else
	{
#if DO_DEBUG
	    y2debug ("return <expr>;");
#endif
	    return lval;		// executed 'return <expr>;'
	}
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// repeat-until statement (-> loop statement, bool condition)

YSRepeat::YSRepeat (YCodePtr loop, YCodePtr condition, int line)
    : YStatement (line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSRepeat::YSRepeat (bytecodeistream & str)
    : YStatement (str)
    , m_loop (0)
{
    if (Bytecode::readBool (str))
    {
	m_loop = Bytecode::readCode (str);
    }
    m_condition = Bytecode::readCode (str);
}


YSRepeat::~YSRepeat ()
{
}


std::ostream &
YSRepeat::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeBool (str, m_loop != 0);
    if (m_loop != 0)
    {
	m_loop->toStream (str);
    }
    return m_condition->toStream (str);
}


std::ostream &
YSRepeat::toXml (std::ostream & str, int indent ) const
{
    str << "<repeat";
    commentToXml(str);
    str << ">\n";
    if (m_loop != 0)
    {
        str << Xmlcode::spaces( indent+2 ) << "<do>\n";
	m_loop->toXml( str, indent+4 );
        str << Xmlcode::spaces( indent+2 ) << "</do>\n";
    }
    str << Xmlcode::spaces( indent+2 ) << "<until>";
    m_condition->toXml( str, 0 );
    str << "</until>\n";
    return str << Xmlcode::spaces( indent ) << "</repeat>";
}


string
YSRepeat::toString () const
{
    string s = "repeat\n";
    if (m_loop != 0)
    {
	s += m_loop->toString();
    }
    else
    {
	s += "{ /* EMPTY */ }";
    }

    s = s + "until (" + m_condition->toString() + ");";
    return s;
}

YCPValue
YSRepeat::evaluate (bool cse)
{
    if (cse) return YCPNull();

#if DO_DEBUG
    y2debug ("YSRepeat::evaluate(%s)\n", toString().c_str());
#endif

    bool first_iteration = true;

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    if (m_loop->isBlock())
	    {
		lval = ((YBlockPtr)m_loop)->evaluate ();
		if (first_iteration) first_iteration = false;
	    }
	    else
	    {
		lval = m_loop->evaluate ();
	    }
	}

	if (lval.isNull()
	    || lval->isVoid())		// normal block/statement or 'continue'
	{
	    YCPValue bval = m_condition->evaluate ();
	    if (bval.isNull ())
	    {
		ycp2error ("until condition is nil.");
		return YCPNull ();
	    }
    
	    if (!bval->isBoolean())
	    {
		ycp2error ( "'repeat ... until (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
		break;
	    }
	    else if (bval->asBoolean()->value() == true)
	    {
		break;
	    }
	}
	else if (lval->isBreak())	// executed 'break'
	{
	    break;
	}
	else if (lval->isReturn())	// executed 'return;'
	{
	    return lval;
	}
	else
	{
	    return lval;		// executed 'return <expr>;'
	}
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// do-while statement (-> loop statement, bool condition)

YSDo::YSDo (YCodePtr loop, YCodePtr condition, int line)
    : YStatement (line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSDo::YSDo (bytecodeistream & str)
    : YStatement (str)
    , m_loop (0)
{
    if (Bytecode::readBool (str))
    {
	m_loop = Bytecode::readCode (str);
    }
    m_condition = Bytecode::readCode (str);
}


YSDo::~YSDo ()
{
}


string
YSDo::toString () const
{
    string s = "do\n";
    if (m_loop != 0)
    {
	s += m_loop->toString();
    }
    else
    {
	s += "{ /* EMPTY */ }";
    }

    s = s + "while (" + m_condition->toString() + ");";
    return s;
}


std::ostream &
YSDo::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeBool (str, m_loop != 0);
    if (m_loop != 0)
    {
	m_loop->toStream (str);
    }
    return m_condition->toStream (str);
}


std::ostream &
YSDo::toXml( std::ostream & str, int indent ) const
{
    str << "<do";
    commentToXml(str);
    str << ">";
    if (m_loop != 0)
    {
	m_loop->toXml( str, indent );
    }
    str << "<while>";
    m_condition->toXml( str, indent );
    str << "</while>";
    return str << "</do>";
}


YCPValue
YSDo::evaluate (bool cse)
{
    if (cse) return YCPNull();

#if DO_DEBUG
    y2debug ("YSDo::evaluate(%s)\n", toString().c_str());
#endif

    bool first_iteration = true;

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    if (m_loop->isBlock())
	    {
		lval = ((YBlockPtr)m_loop)->evaluate ();
		if (first_iteration) first_iteration = false;
	    }
	    else
	    {
		lval = m_loop->evaluate ();
	    }
	}
	if (lval.isNull()
	    || lval->isVoid())		// normal block/statement or 'continue'
	{
	    YCPValue bval = m_condition->evaluate ();
	    if (bval.isNull ())
	    {
		ycp2error ("while condition is nil.");
		return YCPNull ();
	    }
    
	    if (!bval->isBoolean())
	    {
		ycp2error ("'do (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
		break;
	    }
	    else if (bval->asBoolean()->value() == false)
	    {
		break;
	    }
	}
	else if (lval->isBreak())	// executed 'break'
	{
	    break;
	}
	else if (lval->isReturn())	// executed 'return;'
	{
	    return lval;
	}
	else
	{
	    return lval;		// executed 'return <expr>;'
	}
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// textdomain

YSTextdomain::YSTextdomain (const string &textdomain, int line)
    : YStatement (line)
    , m_domain (Ustring (*SymbolEntry::_nameHash, textdomain))
{
    bind ();
}


YSTextdomain::~YSTextdomain ()
{
}


string
YSTextdomain::toString() const
{
    string s = string ("textdomain \"") + m_domain.asString() + "\";";
    return s;
}


YSTextdomain::YSTextdomain (bytecodeistream & str)
    : YStatement (str)
    , m_domain (Bytecode::readUstring (str))
{
    bind ();
}


std::ostream &
YSTextdomain::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeUstring (str, m_domain);
}


std::ostream &
YSTextdomain::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<textdomain name=\"" << m_domain.asString() << "\"";
    commentToXml(str);
    str << "/>";
    return str;
}


YCPValue
YSTextdomain::evaluate (bool /*cse*/)
{
    return YCPNull();
}

void
YSTextdomain::bind ()
{
#if DO_DEBUG
    y2debug ("going to bind a domain %s", m_domain->c_str() );
#endif
    YLocale::ensureBindDomain (m_domain);
}

// ------------------------------------------------------------------
// include

YSInclude::YSInclude (const string &filename, int line, bool skipped)
    : YStatement (line)
    , m_filename (Ustring (*SymbolEntry::_nameHash, filename))
    , m_skipped (skipped)
{
}


YSInclude::~YSInclude ()
{
}


string
YSInclude::toString() const
{
    string s = string ("// include \"") + m_filename.asString() + "\";";
    return s;
}


YSInclude::YSInclude (bytecodeistream & str)
    : YStatement (str)
    , m_filename (Bytecode::readUstring (str))
{
    m_skipped = Bytecode::readBool (str);
}


std::ostream &
YSInclude::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeUstring (str, m_filename);
    return Bytecode::writeBool (str, m_skipped);
}


std::ostream &
YSInclude::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << "<include";
    commentToXml(str);
    if (m_skipped) str << " skipped=\"1\"";
    return str << " name=\"" << m_filename.asString() << "\"/>";
}


YCPValue
YSInclude::evaluate (bool cse)
{
    if (!cse && !m_skipped) YaST::ee.setFilename(m_filename);
    return YCPNull();
}

//-------------------------------------------------------------------
// import

// normal 'import' statement during .ycp parsing

YSImport::YSImport (const string &name, int line)
    : YStatement (line)
{
    Import::disableTracking();				// don't track variable usage _inside_ the imported module

    if (import (name) != 0)
    {
#if DO_DEBUG
	y2debug ("import '%s' failed", name.c_str());   // debug only, import() already logged the error
#endif
	// the caller checks this->name().empty() as an error indicator
    }

    Import::enableTracking();                          // continue tracking in .ycp
}


// 'pre'tached 'import' statement for preloaded namespaces (see parser.yy)

YSImport::YSImport (const string &name, Y2Namespace *name_space)
    : YStatement (0)
    , Import (name, name_space)
{
}


YSImport::~YSImport ()
{
}


string
YSImport::name () const
{
    return m_name.asString();
}


string
YSImport::toString() const
{
    string s = string ("import \"") + m_name.asString() + "\";";
    return s;
}


// Import module from bytecode
//   -> we're in bytecode reading and just encountered an 'import' statement in bytecode
//
// If an error occurs, m_name will be set to empty
//
YSImport::YSImport (bytecodeistream & str)
    : YStatement (str)
{
    char *mname = Bytecode::readCharp (str);
    import (mname);				// <=== this does the importing
    delete [] mname;

    if (nameSpace() == 0)
    {
	ycp2error ("Import '%s' failed...\n", name().c_str());
	ycp2error ("Could not create its namespace\n");
	throw Bytecode::Invalid();
    }

    // now load symbols we need from the just imported namespace
    // (this import statement is here because symbols are needed from the namespace)

    Bytecode::pushNamespace (nameSpace (), true);			// see YBlock::YBlock(str) for popUptoNamespace()

    bool xref_debug = (getenv (XREFDEBUG) != 0);

    int xrefcount = Bytecode::readInt32 (str);

    if (xref_debug) y2milestone ("Resolving %d symbols from module %s\n", xrefcount, m_name->c_str());
#if DO_DEBUG
    else y2debug ("Resolving %d symbols from module %s\n", xrefcount, m_name->c_str());
#endif

    if (xrefcount != 0)
    {
	SymbolTable *table = m_module->second->table();

	const char *sname;
	TypePtr stype;
	TableEntry *tentry;

	while (xrefcount-- > 0)						// build up xref vector in table
	{
	    sname = Bytecode::readCharp (str);
	    stype = Bytecode::readType (str);

	    if (xref_debug) y2milestone ("Xref -------------------------------- '%s' <%s>\n", sname, stype->toString().c_str());
#if DO_DEBUG
	    else y2debug ("Xref -------------------------------- '%s' <%s>\n", sname, stype->toString().c_str());
#endif

	    tentry = table->xref (sname);				// look for match in table

	    if (tentry == 0)
	    {
		ycp2error ("Import '%s' failed\n", m_name->c_str());
		ycp2error ("Symbol '%s::%s' does not exist.\n", m_name->c_str(), sname);
		throw Bytecode::Invalid();
	    }
	    else if (tentry->sentry()->type()->match (stype) != 0)
	    {
		ycp2error ("Import '%s' failed\n", m_name->c_str());
		ycp2error ("A reference to '%s::%s' expects type <%s> but module provides type <%s>\n",
			m_name->c_str(), sname,
			stype->toString().c_str(), tentry->sentry()->type()->toString().c_str());
		throw Bytecode::Invalid();
	    }
	    delete [] sname;
	}
    }
}


std::ostream &
YSImport::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeUstring (str, m_name);

    SymbolTable *table = m_module->second->table();
    table->writeUsage (str);

#if DO_DEBUG
    y2debug ("pushNamespace import '%s'", m_name->c_str());
#endif
    Bytecode::pushNamespace (nameSpace());				// see YBlock::toStream(str) for popUptoNamespace()

    return str;
}


std::ostream &
YSImport::toXml( std::ostream & str, int /*indent*/ ) const
{
    Xmlcode::pushNamespace (nameSpace());				// see YBlock::toXml(str) for popUptoNamespace()
    str << "<import name=\"" << m_name.asString() << "\"";
    commentToXml(str);
    return str << "/>";
}


YCPValue
YSImport::evaluate (bool cse)
{
    if (!cse && (nameSpace () != NULL))
    {
#if DO_DEBUG
	y2debug ("Evaluating namespace '%s'", m_name->c_str());
#endif
	
	nameSpace()->initialize ();
    }

    return YCPNull();
}


//-------------------------------------------------------------------
/**
 * internal: Filename
 */

YSFilename::YSFilename (const string &filename, int line)
    : YStatement (line)
    , m_filename (Ustring (*SymbolEntry::_nameHash, filename))
{
}


YSFilename::~YSFilename ()
{
}


string
YSFilename::toString() const
{
    string s = string ("// force filename: \"") + m_filename.asString() + "\"";
    return s;
}


YSFilename::YSFilename (bytecodeistream & str)
    : YStatement (str)
    , m_filename (Bytecode::readUstring (str))
{
}


std::ostream &
YSFilename::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeUstring (str, m_filename);
}


std::ostream &
YSFilename::toXml(std::ostream & str, int /*indent*/ ) const
{
    return str << "<filename name=\"" << m_filename.asString() << "\"/>";
}


YCPValue
YSFilename::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug( "YSFilename to set %s", m_filename->c_str ());
#endif
    if (! cse) YaST::ee.setFilename(m_filename.asString());

    return YCPNull ();
}


//-------------------------------------------------------------------
/**
 * Switch
 */

YSSwitch::YSSwitch (YCodePtr condition)
    : YStatement ()
    , m_condition (condition)
    , m_block (0)
    , m_defaultcase (-1)
{
}


YSSwitch::~YSSwitch ()
{
}


string
YSSwitch::toString() const
{
    string s = string ("switch (") + m_condition->toString () + ") \n";
    s += m_block->toStringSwitch (m_cases, m_defaultcase);
    return s;
}


YSSwitch::YSSwitch (bytecodeistream & str)
    : YStatement (str)
    , m_condition (0)
    , m_block (0)
    , m_defaultcase (-1)
{
    m_condition = Bytecode::readCode (str);
    if (! m_condition)
    {
	throw Bytecode::Invalid();
    }

    int size = Bytecode::readInt32 (str);
    for (int i = 0; i < size ; i++)
    {
	YCPValue cv = Bytecode::readValue (str);
	if (cv.isNull ())
	{
	    throw Bytecode::Invalid();
	}
	int index = Bytecode::readInt32 (str);
	m_cases[cv] = index;
    }
    
    m_defaultcase = Bytecode::readInt32 (str);
    
    m_block = (YBlockPtr)Bytecode::readCode (str);
    if (! m_block)
    {
	throw Bytecode::Invalid();
    }
}


std::ostream &
YSSwitch::toStream (std::ostream & str) const
{
    YStatement::toStream (str);

    m_condition->toStream (str);

    Bytecode::writeInt32 (str, m_cases.size ());
    
    for (map<YCPValue, int, ycp_less>::const_iterator it = m_cases.begin ()
	; it != m_cases.end () ; it++)
    {
	Bytecode::writeValue (str, it->first);
	Bytecode::writeInt32 (str, it->second);
    }
    
    Bytecode::writeInt32 (str, m_defaultcase);
    
    return m_block->toStream (str);
}


std::ostream &
YSSwitch::toXml( std::ostream & str, int indent ) const
{
    str << "<switch";
    commentToXml(str);
    str << ">";

    str << "<cond>";
    m_condition->toXml( str, 0 );
    str << "</cond>\n";

    m_block->toXmlSwitch( m_cases, m_defaultcase, str, indent+2 );
    return str << endl << Xmlcode::spaces( indent ) << "</switch>";
}


YCPValue
YSSwitch::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug( "YSSwitch");
#endif
    if (cse)
	return YCPNull ();
	
    YCPValue condition = m_condition->evaluate ();
    if (condition.isNull ())
    {
	ycperror ("switch condition evaluates to 'nil'");
	return YCPNull ();
    }
    
    // if there is a case for this value, execute
    if (m_cases.find (condition) != m_cases.end ())
    {
#if DO_DEBUG
	y2debug ("Evaluating from statement '%d'",m_cases[condition]);
#endif
	YCPValue res = m_block->evaluateFrom (m_cases[condition]);

	// break should not be propagated
	if (!res.isNull () && res->isBreak ())
	    res = YCPNull ();
	    
	return res;
    }
    // no case, try default if defined
    else if (m_defaultcase != -1)
    {
#if DO_DEBUG
	y2debug ("Evaluating from default statement '%d'",m_defaultcase);
#endif
	YCPValue res = m_block->evaluateFrom (m_defaultcase);

	// break should not be propagated
	if (!res.isNull () && res->isBreak ())
	    res = YCPNull ();
	    
	return res;
    }

#if DO_DEBUG
    y2debug ("Switch done");
#endif

    return YCPNull ();
}


bool
YSSwitch::setCase (YCPValue value)
{
    // verify duplicate
    if (m_cases.find (value) != m_cases.end ())
	return false;
    
    int index = m_block->statementCount ();
    
    m_cases[value]=index;
    
    return true;
}


bool
YSSwitch::setDefaultCase ()
{
    // fail, if there is default case already
    if (m_defaultcase != -1)
	return false;

    m_defaultcase = m_block->statementCount ();
    
    return true;
}


void
YSSwitch::setBlock (YBlockPtr block)
{
    m_block = block;
}

// EOF
