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

$Id$
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

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

#include "y2/Y2Component.h"
#include "y2/Y2ComponentBroker.h"

extern ExecutionEnvironment ee;

// ------------------------------------------------------------------
// statement (-> statement, next statement)

YStatement::YStatement (ykind kind, int line)
    : YCode (kind)
    , m_line (line)
{
}


YStatement::YStatement (ykind kind, std::istream & str)
    : YCode (kind)
{
    m_line = Bytecode::readInt32 (str);
    y2debug ("YStatement::YStatement([%d]:%d)", (int)kind, m_line);
}


string
YStatement::toString () const
{
    switch (kind())
    {
	case ysBreak:	 return "break;"; break;
	case ysContinue: return "continue;"; break;
	default:
	    break;
    }
    return "<<undefined statement>>";
}


YCPValue
YStatement::evaluate (bool cse)
{
    y2debug ("YStatement::evaluate(%s)\n", toString().c_str());
    switch (kind())
    {
	case ysBreak:	    return YCPBreak();
	case ysContinue:    return YCPVoid();
	default:	    break;
    }
    return YCPNull();
}


std::ostream &
YStatement::toStream (std::ostream & str) const
{
    y2debug ("YStatement::toStream ([%d]:%d)", (int)kind(), m_line);
    YCode::toStream (str);
    return Bytecode::writeInt32 (str, m_line);
}

// ------------------------------------------------------------------
// expression as statement

YSExpression::YSExpression (YCode *expr, int line)
    : YStatement (ysExpression, line)
    , m_expr (expr)
{
}


YSExpression::YSExpression (std::istream & str)
    : YStatement (ysExpression, str)
{
    m_expr = Bytecode::readCode (str);
}


YSExpression::~YSExpression ()
{
    delete m_expr;
}


string
YSExpression::toString() const
{
    string s = m_expr->toString();
    s += ";";
    return s;
}


std::ostream &
YSExpression::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return m_expr->toStream (str);
}


YCPValue
YSExpression::evaluate (bool cse)
{
    y2debug ("YSExpression::evaluate(%s:%d)\n", toString().c_str(), line());
    m_expr->evaluate (cse);
    return YCPNull();
}


// ------------------------------------------------------------------
// return

YSReturn::YSReturn (YCode *value, int line)
    : YStatement (ysReturn, line)
    , m_value (value)
{
}


YSReturn::YSReturn (std::istream & str)
    : YStatement (ysReturn, str)
    , m_value (0)
{
    if (Bytecode::readBool (str))
    {
	m_value = Bytecode::readCode (str);
    }
}


YSReturn::~YSReturn ()
{
    if (m_value)
	delete m_value;
}


YCode *
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
//y2debug ("YSReturn::toString value kind %d", m_value->kind());
	s += " ";
	s += m_value->toString();
    }
    s += ";";
//y2debug ("s %s", s.c_str());
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


YCPValue
YSReturn::evaluate (bool cse)
{
    if (cse) return YCPNull();
    y2debug ("YSReturn::evaluate (%s)\n", m_value ? m_value->toString().c_str() : "");
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

YSFunction::YSFunction (SymbolEntry *entry, int line)
    : YStatement (ysFunction, line)
    , m_entry (entry)
{
}


YSFunction::~YSFunction ()
{
    // don't delete m_entry here, it belongs to SymbolTable
}


SymbolEntry *
YSFunction::entry() const
{
    return m_entry;
}


YFunction *
YSFunction::function() const
{
    return (YFunction *)(m_entry->code());
}


string
YSFunction::toString() const
{
    return m_entry->toString() + "\n" + ((YFunction *)(m_entry->code()))->definition()->toString();
}


YCPValue
YSFunction::evaluate (bool cse)
{
    y2debug ("YSFunction::evaluate(%s)\n", toString().c_str());
    // there's nothing to evaluate for a function _definition_
    // its all in the function call.
    return YCPNull();
}


YSFunction::YSFunction (std::istream & str)
    : YStatement (ysFunction, str)
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


// ------------------------------------------------------------------
// typedef (-> type string)

YSTypedef::YSTypedef (const string &name, constTypePtr type, int line)
    : YStatement (ysTypedef, line)
    , m_name (name)
    , m_type (type)
{
}


YSTypedef::YSTypedef (std::istream & str)
    : YStatement (ysTypedef, str)
{
    Bytecode::readString (str, m_name);
    m_type = Bytecode::readType (str);
}


string
YSTypedef::toString() const
{
    string s = string ("typedef ") + m_name + " " + m_type->toString() + ";";
    return s;
}


std::ostream &
YSTypedef::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeString (str, m_name);
    return m_type->toStream (str);
}


// FIXME: needed ?

YCPValue
YSTypedef::evaluate (bool cse)
{
    y2debug("evaluate(%s) = nil", toString().c_str());
    return YCPNull();
}

// ------------------------------------------------------------------
// assignment or definition
// -> the ykind tells

YSAssign::YSAssign (bool definition, SymbolEntry *entry, YCode *code, int line)
    : YStatement (definition ? ysVariable : ysAssign, line)
    , m_entry (entry)
    , m_code (code)
{
}


YSAssign::YSAssign (bool definition, std::istream & str)
    : YStatement (definition ? ysVariable : ysAssign, str)
{
    m_entry = Bytecode::readEntry (str);
    m_code = Bytecode::readCode (str);
    
    if (definition)
    {
	// setup default value
	m_entry->setCode (m_code);
    }
}


YSAssign::~YSAssign ()
{
    // don't delete m_entry here, it belongs to SymbolTable
    delete m_code;
}


string
YSAssign::toString () const
{
    return (((kind() == ysVariable) && (m_entry->nameSpace() == 0)) ? "global " : "")
	+ m_entry->toString (kind() == ysVariable)
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


YCPValue
YSAssign::evaluate (bool cse)
{
    if (cse) return YCPNull();
    y2debug ("YSAssign::evaluate(%s)\n", toString().c_str());
    YCPValue value = m_code->evaluate ();
    m_entry->setValue (value);
    y2debug ("YSAssign::evaluate (%s) = '%s'\n", m_code->toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());

    return YCPNull();
}


// ------------------------------------------------------------------
// bracket assignment


YSBracket::YSBracket (SymbolEntry *entry, YCode *arg, YCode *code, int line)
    : YStatement (ysBracket, line)
    , m_entry (entry)
    , m_arg (arg)
    , m_code (code)
{
}


YSBracket::~YSBracket ()
{
    // don't delete m_entry here, it belongs to SymbolTable
    delete m_arg;
    delete m_code;
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


YSBracket::YSBracket (std::istream & str)
    : YStatement (ysBracket, str)
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


// commit bracket assign recursively

YCPValue
YSBracket::commit (YCPValue current, int idx, YCPList arg, YCPValue value)
{
    if (idx > arg->size())
	return YCPNull();

    if (idx == arg->size())
	return value;
	
    if (current.isNull ())
    {
	y2error ("Non-existent bracket parameter");
	return YCPNull ();
    }
	
y2debug ("commit (%s, %d, %s, %s)", current->toString().c_str(), idx, arg->toString().c_str(), value.isNull () ? "nil" : value->toString().c_str());

    YCPValue argval = arg->value (idx);
    if (argval.isNull())
    {
	y2error ("Invalid bracket parameter");
	return YCPNull ();
    }

    if (current->isList())
    {
	if (!argval->isInteger())
	{
	    y2error ("Invalid bracket parameter for list");
	    return YCPNull ();
	}
	YCPList list = current->asList();
	int argint = argval->asInteger()->value();
	
	YCPValue val = value;
	
	//  not the end of the argument list, continue
	if (idx < arg->size ()-1)
	{
	    val = commit (list->value (argint), idx+1, arg, value);
	    if (val.isNull ())
		return val;
	}
	
	list->set (argint, val);
y2debug ("list[%d] = %s -> %s", argint, val->toString().c_str(), list->toString().c_str());
	return list;
    }
    else if (current->isMap())
    {
	YCPMap map = current->asMap();
	
	YCPValue val = value;
	
	if (idx < arg->size ()-1)
	{
	    val = commit (map->value (argval), idx+1, arg, value);
	    if (val.isNull ())
		return val;
	}
	map = map->functionalAdd (argval, val);
y2debug ("map[%s] = %s -> %s", argval->toString().c_str(), val->toString().c_str(), map->toString().c_str());
	return map;
    }
    else if (current->isTerm())
    {
	if (!argval->isInteger())
	{
	    y2error ("Invalid bracket parameter for term");
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
	term->set (argval->asInteger()->value(), val);
	return term;
    }
    y2error ("Bracket assignment not list, map, or term");
    return YCPNull ();
}


YCPValue
YSBracket::evaluate (bool cse)
{
    if (cse) return YCPNull();

    // check variable first
    YCPValue result = m_entry->value(); 
    if (result.isNull())
    {
	// initial assignment
	m_entry->setValue (m_entry->code() ? m_entry->code()->evaluate () : YCPNull());
	result = m_entry->value();
    }

    // evaluate other arguments _before_ error checking
    YCPValue arg_value = m_arg->evaluate ();
    YCPValue newvalue = m_code->evaluate ();

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

    result = commit (result, 0, arg_value->asList(), newvalue);

    if (!result.isNull())
    {
	m_entry->setValue (result);
y2debug ("%s = %s", m_entry->name(), result->toString().c_str());
	result = YCPNull();
    }

    return result;
}



// ------------------------------------------------------------------
// If-then-else statement (-> bool expr, true statement, false statement)

YSIf::YSIf (YCode *a_condition, YCode *a_true, YCode *a_false, int line)
    : YStatement (ysIf, line)
    , m_condition (a_condition)
    , m_true (a_true)
    , m_false (a_false)
{
}


YSIf::YSIf (std::istream & str)
    : YStatement (ysIf, str)
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
    delete m_condition;
    delete m_true;
    if (m_false) delete m_false;
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


YCPValue
YSIf::evaluate (bool cse)
{
    y2debug ("YSIf::evaluate(%s)\n", toString().c_str());
    YCPValue bval = m_condition->evaluate (cse);
    
    if (bval.isNull ())
    {
	ycp2error ("if condition is nil.");
	return YCPNull ();
    }
    
    if (!bval->isBoolean())
    {
	ycp2error ("'if (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
    }
    else if ((bval->asBoolean()->value() == true)
	     && (m_true != 0))
    {
	return m_true->evaluate (cse);
    }
    else if (m_false != 0)
    {
	return m_false->evaluate (cse);
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// while-do statement (-> bool condition, loop statement)

YSWhile::YSWhile (YCode *condition, YCode *loop, int line)
    : YStatement (ysWhile, line)
    , m_condition (condition)
    , m_loop (loop)
{
}


YSWhile::~YSWhile ()
{
    delete m_condition;
    delete m_loop;
}


string
YSWhile::toString () const
{
    string s = "while (" + m_condition->toString() + ")\n    ";
    if (m_loop != 0)
	s += m_loop->toString();
    else
	s += "{ /* EMPTY */ }";
    return s;
}


YSWhile::YSWhile (std::istream & str)
    : YStatement (ysWhile, str)
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


YCPValue
YSWhile::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YSWhile::evaluate(%s)\n", toString().c_str());

    bool first_iteration = true;

    for (;;)
    {
	YCPValue bval = m_condition->evaluate ();
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
	    lval = ((YBlock *)m_loop)->evaluate ();
	    if (first_iteration) first_iteration = false;
	}
	else
	{
	    lval = m_loop->evaluate ();
	}

	y2debug ("YSWhile::evaluate lval (%s)", lval.isNull()?"NULL":lval->toString().c_str());

	if (lval.isNull())
	{
	    continue;
	}
	else if (lval->isBreak())	// executed 'break'
	{
	    break;
	}
	else if (lval->isReturn())	// executed 'return;' - YCPReturn is also YCPVoid, keep the order of tests!
	{
	    return lval;
	}
	else if (lval->isVoid())	// normal block/statement or 'continue'
	{
	    continue;
	}
	else
	{
	    return lval;		// executed 'return <expr>;'
	}
    }
    return YCPNull ();
}


// ------------------------------------------------------------------
// repeat-until statement (-> loop statement, bool condition)

YSRepeat::YSRepeat (YCode *loop, YCode *condition, int line)
    : YStatement (ysRepeat, line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSRepeat::YSRepeat (std::istream & str)
    : YStatement (ysRepeat, str)
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
    delete m_loop;
    delete m_condition;
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

    y2debug ("YSRepeat::evaluate(%s)\n", toString().c_str());

    bool first_iteration = true;

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    if (m_loop->isBlock())
	    {
		lval = ((YBlock *)m_loop)->evaluate ();
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

YSDo::YSDo (YCode *loop, YCode *condition, int line)
    : YStatement (ysDo, line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSDo::YSDo (std::istream & str)
    : YStatement (ysDo, str)
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
    delete m_loop;
    delete m_condition;
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


YCPValue
YSDo::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YSDo::evaluate(%s)\n", toString().c_str());

    bool first_iteration = true;

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    if (m_loop->isBlock())
	    {
		lval = ((YBlock *)m_loop)->evaluate ();
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
    : YStatement (ysTextdomain, line)
    , m_domain (textdomain)
{
    bind ();
}


YSTextdomain::~YSTextdomain ()
{
}


string
YSTextdomain::toString() const
{
    string s = string ("textdomain \"") + m_domain + "\";";
    return s;
}


YSTextdomain::YSTextdomain (std::istream & str)
    : YStatement (ysTextdomain, str)
{
    Bytecode::readString (str, m_domain);
    
    bind ();
}


std::ostream &
YSTextdomain::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeString (str, m_domain);
}


YCPValue
YSTextdomain::evaluate (bool cse)
{
    return YCPNull();
}

void
YSTextdomain::bind ()
{
    y2debug ("going to bind a domain %s", m_domain.c_str() );
    bindtextdomain (m_domain.c_str (), LOCALEDIR);
    bind_textdomain_codeset (m_domain.c_str (), "UTF-8");
}

// ------------------------------------------------------------------
// include

YSInclude::YSInclude (const string &filename, int line, bool skipped)
    : YStatement (ysInclude, line)
    , m_filename (filename)
    , m_skipped (skipped)
{
}


YSInclude::~YSInclude ()
{
}


string
YSInclude::toString() const
{
    string s = string ("// include \"") + m_filename + "\";";
    return s;
}


YSInclude::YSInclude (std::istream & str)
    : YStatement (ysInclude, str)
{
    m_filename = Bytecode::readCharp (str);
    m_skipped = Bytecode::readBool (str);
}


std::ostream &
YSInclude::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeCharp (str, m_filename.c_str());
    return Bytecode::writeBool (str, m_skipped);
}


YCPValue
YSInclude::evaluate (bool cse)
{
    if (!cse && !m_skipped) ee.setFilename (m_filename);
    return YCPNull();
}

//-------------------------------------------------------------------
// import

// normal 'import' statement

YSImport::YSImport (const string &name, int line)
    : YStatement (ysImport, line)
    , Import (name)
{
}


// 'pre'tached 'import' statement for preloaded namespaces (see parser.yy)

YSImport::YSImport (const string &name, Y2Namespace *name_space)
    : YStatement (ysImport, 0)
    , Import (name, name_space)
{
}


YSImport::~YSImport ()
{
}


string
YSImport::name () const
{
    return m_name;
}


string
YSImport::toString() const
{
    string s = string ("import \"") + m_name + "\";";
    return s;
}


YSImport::YSImport (std::istream & str)
    : YStatement (ysImport, str)
    , Import (Bytecode::readCharp (str), 0, true)			// don't open references in Import()
{
    if (nameSpace() == 0)
    {
	fprintf (stderr, "Import '%s' failed\n", name().c_str());
#warning Pass error about failed import to top properly
	return;
    }

    Bytecode::pushNamespace (nameSpace ());				// see YBlock::YBlock(str) for popUptoNamespace()

    bool xref_debug = (getenv (XREFDEBUG) != 0);

    int xrefcount = Bytecode::readInt32 (str);

    if (xref_debug) y2milestone ("Resolving %d symbols from module %s\n", xrefcount, m_name.c_str());
    else y2debug ("Resolving %d symbols from module %s\n", xrefcount, m_name.c_str());

    if (xrefcount != 0)
    {
	SymbolTable *table = m_module->second.name_space->table();

	const char *sname;
	TypePtr stype;
	TableEntry *tentry;

	while (xrefcount-- > 0)						// build up xref vector in table
	{
	    sname = Bytecode::readCharp (str);
	    stype = Bytecode::readType (str);

	    if (xref_debug) y2milestone ("Xref -------- '%s' <%s>\n", sname, stype->toString().c_str());
	    else y2debug ("Xref -------- '%s' <%s>\n", sname, stype->toString().c_str());

	    tentry = table->xref (sname);				// look for match in table

	    if (tentry == 0)
	    {
		y2error ("Unresolved xref to %s::%s\n", m_name.c_str(), sname);
		fprintf (stderr, "Unresolved xref to %s::%s\n", m_name.c_str(), sname);
		m_name = "";
	    }
	    if (tentry->sentry()->type()->match (stype) != 0)
	    {
		y2error ("Symbol '%s::%s' <%s> does not match xref type <%s>\n", m_name.c_str(), sname, tentry->sentry()->type()->toString().c_str(), stype->toString().c_str());
		fprintf (stderr, "Symbol '%s::%s' <%s> does not match xref type <%s>\n", m_name.c_str(), sname, tentry->sentry()->type()->toString().c_str(), stype->toString().c_str());
		m_name = "";
	    }
	    delete [] sname;
	}
    }
}


std::ostream &
YSImport::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeCharp (str, m_name.c_str());

    SymbolTable *table = m_module->second.name_space->table();
    table->writeUsage (str);

    y2debug ("pushNamespace import '%s'", m_name.c_str());
    Bytecode::pushNamespace (nameSpace());				// see YBlock::toStream(str) for popUptoNamespace()

    return str;
}


YCPValue
YSImport::evaluate (bool cse)
{
    if (!cse
	&& (nameSpace () != NULL)
	&& !m_module->second.activated)
    {
	// init all definitions

	nameSpace()->evaluate (cse);

	// run constructor

	if (m_module->second.constructor != 0)
	{
	    y2debug ("Calling constructor '%s'", m_name.c_str());
	    YEFunction fun (m_module->second.constructor);
	    fun.finalize();
	    fun.evaluate();
	}

	m_module->second.activated = true;
    }

    return YCPNull();
}


//-------------------------------------------------------------------
/**
 * internal: Filename
 */

YSFilename::YSFilename (const string &filename, int line)
    : YStatement (ysFilename, line)
    , m_filename (filename)
{
}


YSFilename::~YSFilename ()
{
}


string
YSFilename::toString() const
{
    string s = string ("// force filename: \"") + m_filename + "\"";
    return s;
}


YSFilename::YSFilename (std::istream & str)
    : YStatement (ysFilename, str)
{
    m_filename = Bytecode::readCharp (str);
}


std::ostream &
YSFilename::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeCharp (str, m_filename.c_str());
}


YCPValue
YSFilename::evaluate (bool cse)
{
    y2debug( "YSFilename to set %s", m_filename.c_str ());
    if (! cse) ee.setFilename (m_filename);

    return YCPNull ();
}

// EOF
