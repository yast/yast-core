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

#include "ycp/YStatement.h"
#include "ycp/YExpression.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPError.h"
#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"

extern ExecutionEnvironment ee;

// ------------------------------------------------------------------
// statement (-> statement, next statement)

YStatement::YStatement (ycode code, int line)
    : YCode (code)
    , m_line (line)
{
}


YStatement::YStatement (ycode code, std::istream & str)
    : YCode (code)
{
    m_line = Bytecode::readInt32 (str);
    y2debug ("YStatement::YStatement([%d]:%d)", (int)code, m_line);
}


string
YStatement::toString () const
{
    switch (code())
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
    switch (code())
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
    y2debug ("YStatement::toStream ([%d]:%d)", (int)code(), m_line);
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
YSReturn::propagate (TypeCode & from, TypeCode & to)
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


YCPValue
YSReturn::evaluate (bool cse)
{
    if (cse) return YCPNull();
    y2debug ("YSReturn::evaluate (%s)\n", m_value ? m_value->toString().c_str() : "");
    if (m_value != 0)
    {
	return m_value->evaluate (cse);
    }
    return YCPReturn();
}


// ------------------------------------------------------------------
// variable definition

YSVariable::YSVariable (SymbolEntry *entry, YCode *code, int line)
    : YStatement (ysVariable, line)
    , m_entry (entry)
{
    m_entry->setCode (code);
}


YSVariable::YSVariable (std::istream & str)
    : YStatement (ysVariable, str)
{
    m_entry = Bytecode::readEntry (str);
}


YSVariable::~YSVariable ()
{
}


string
YSVariable::toString() const
{
    return ((m_entry->block() == 0)?"global ":"")
	+ m_entry->toString (true)
    	+ " = "
	+ m_entry->code()->toString()
	+ ";";
}


std::ostream &
YSVariable::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeEntry (str, m_entry);
}


YCPValue
YSVariable::evaluate (bool cse)
{
    y2debug ("YSVariable::evaluate(%s)\n", toString().c_str());

    // nothing to do here since the variable initialization is done
    //   during block initialization.
    // YSVariable just keeps track of a variables definition inside
    //  the statement list of a block. And just for debug/output
    //  purposes. The correct usage and visibility of variables is
    //  ensured during parse time.
    return YCPNull();
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
    return m_entry->toString() + m_entry->code()->toString();
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
}


std::ostream &
YSFunction::toStream (std::ostream & str) const
{
    YStatement::toStream (str);

    return Bytecode::writeEntry (str, m_entry);
}


// ------------------------------------------------------------------
// typedef (-> type string)

YSTypedef::YSTypedef (const string &name, const TypeCode & type, int line)
    : YStatement (ysTypedef, line)
    , m_name (name)
    , m_type (type)
{
}


YSTypedef::YSTypedef (std::istream & str)
    : YStatement (ysTypedef, str)
{
    Bytecode::readString (str, m_name);
    m_type = TypeCode (str);
}


string
YSTypedef::toString() const
{
    string s = string ("typedef ") + m_name + " " + m_type.toString() + ";";
    return s;
}


std::ostream &
YSTypedef::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    Bytecode::writeString (str, m_name);
    return m_type.toStream (str);
}


// FIXME: needed ?

YCPValue
YSTypedef::evaluate (bool cse)
{
    y2debug("evaluate(%s) = nil", toString().c_str());
    return YCPNull();
}

// ------------------------------------------------------------------
// assignment

YSAssign::YSAssign (SymbolEntry *entry, YCode *code, int line)
    : YStatement (ysAssign, line)
    , m_entry (entry)
    , m_code (code)
{
}


YSAssign::YSAssign (std::istream & str)
    : YStatement (ysAssign, str)
{
    m_entry = Bytecode::readEntry (str);
    m_code = Bytecode::readCode (str);
}


YSAssign::~YSAssign ()
{
    // don't delete m_entry here, it belongs to SymbolTable
    delete m_code;
}


string
YSAssign::toString () const
{
    return m_entry->toString (false)
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
    y2debug ("YSAssign::evaluate (%s) = '%s'\n", m_code->toString().c_str(), value.isNull()?"NULL":value->toString().c_str());

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
y2debug ("commit (%s, %d, %s, %s)", current->toString().c_str(), idx, arg->toString().c_str(), value->toString().c_str());
    if (idx > arg->size())
	return YCPNull();

    if (idx == arg->size())
	return value;

    YCPValue argval = arg->value (idx);
    if (argval.isNull())
    {
	return YCPError ("Invalid bracket parameter");
    }

    if (current->isList())
    {
	if (!argval->isInteger())
	{
	    return YCPError ("Invalid bracket parameter for list");
	}
	YCPList list = current->asList();
	int argint = argval->asInteger()->value();
	list = list->shallowCopy();
	YCPValue val = commit (list->value (argint), idx+1, arg, value);
	if (val->isError ())
	    return val;
	list->set (argint, val);
y2debug ("list[%d] = %s -> %s", argint, val->toString().c_str(), list->toString().c_str());
	return list;
    }
    else if (current->isMap())
    {
	YCPMap map = current->asMap();
	YCPValue val = commit (map->value (argval), idx+1, arg, value);
	if (val->isError ())
	    return val;
	map = map->functionalAdd (argval, val);
y2debug ("map[%s] = %s -> %s", argval->toString().c_str(), val->toString().c_str(), map->toString().c_str());
	return map;
    }
    else if (current->isTerm())
    {
	YCPTerm term = current->asTerm();
	YCPValue val = commit (term->value (idx), idx+1, arg, value);
	if (val->isError ())
	    return val;
	term->set (argval->asInteger()->value(), val);
	return term;
    }
    return YCPError ("Bracket assignment not list, map, or term");
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
	return YCPError (string ("Assignment not possible: variable ")
			   + m_entry->toString()
			   + " is not declared");
    }

    // bad bracket argument
    if (arg_value.isNull()
	|| !arg_value->isList())
    {
	return YCPError (string ("Assignment not possible: bracket ")
			   + m_arg->toString()
			   + " does not evaluate to list");
    }

    result = commit (result, 0, arg_value->asList(), newvalue);

    if (!result.isNull()
	&& !result->isError())
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
    if (!bval->isBoolean())
    {
	ycp2error ("", 0, "'if (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
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
{
    m_condition = Bytecode::readCode (str);
    m_loop = Bytecode::readCode (str);
}


std::ostream &
YSWhile::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    m_condition->toStream (str);
    return m_loop->toStream (str);
}


YCPValue
YSWhile::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YSWhile::evaluate(%s)\n", toString().c_str());

    for (;;)
    {
	YCPValue bval = m_condition->evaluate ();
	if (!bval->isBoolean())
	{
	    ycp2error ("", 0, "'while (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
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

	lval = m_loop->evaluate ();

	y2debug ("YSWhile::evaluate lval (%s)", lval.isNull()?"NULL":lval->toString().c_str());

	if (lval.isNull()
	    || lval->isVoid())		// normal block/statement or 'continue'
	{
	    continue;
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
// repeat-until statement (-> loop statement, bool condition)

YSRepeat::YSRepeat (YBlock *loop, YCode *condition, int line)
    : YStatement (ysRepeat, line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSRepeat::YSRepeat (std::istream & str)
    : YStatement (ysRepeat, str)
{
    m_loop = (YBlock *)Bytecode::readCode (str);
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
    m_loop->toStream (str);
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

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    lval = m_loop->evaluate ();
	}

	if (lval.isNull()
	    || lval->isVoid())	// normal block/statement or 'continue'
	{
	    YCPValue bval = m_condition->evaluate ();
	    if (!bval->isBoolean())
	    {
		ycp2error ("", 0, "'repeat ... until (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
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

YSDo::YSDo (YBlock *loop, YCode *condition, int line)
    : YStatement (ysDo, line)
    , m_loop (loop)
    , m_condition (condition)
{
}


YSDo::YSDo (std::istream & str)
    : YStatement (ysDo, str)
{
    m_loop = (YBlock *)Bytecode::readCode (str);
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
    m_loop->toStream (str);
    return m_condition->toStream (str);
}


YCPValue
YSDo::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YSDo::evaluate(%s)\n", toString().c_str());

    for (;;)
    {
	YCPValue lval = YCPNull();
	if (m_loop != 0)
	{
	    lval = m_loop->evaluate ();
	}
	if (lval.isNull()
	    || lval->isVoid())	// normal block/statement or 'continue'
	{
	    YCPValue bval = m_condition->evaluate ();
	    if (!bval->isBoolean())
	    {
		ycp2error ("", 0, "'do (%s)' evaluates to non-boolean '(%s)'.", m_condition->toString().c_str(), bval->toString().c_str());
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

// ------------------------------------------------------------------
// include

YSInclude::YSInclude (const string &filename, int line)
    : YStatement (ysInclude, line)
    , m_filename (filename)
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
}


std::ostream &
YSInclude::toStream (std::ostream & str) const
{
    YStatement::toStream (str);
    return Bytecode::writeCharp (str, m_filename.c_str());
}


YCPValue
YSInclude::evaluate (bool cse)
{
    ee.setFilename (m_filename);
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
    string s = string ("// filename: \"") + m_filename + "\"";
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
    ee.setFilename (m_filename);
    return YCPNull();
}

// EOF
