/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | |( _| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							( C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Xmlcode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   Cloned from Bytecode.cc

   This file contains code to output YCode as XML.

   It primary use is to have an easily parseable representation
   of the abstract syntax tree coming from the YCP parser.
   This XML representation could then be used to convert it
   to C, Ruby, Python, Java, C#, whatever, code.

   See also http://idea.opensuse.org/content/ideas/ycp-to-ruby-translator

   The functions to read XML (and construct YCode from it) are
   not needed and therefore disabled. If the need arises in
   the future to parse XML, define XMLCODE_INPUT_SUPPORTED to 1
   and fix the missing pieces ;-)
/-*/

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

// No, input is not supported
#define XMLCODE_INPUT_SUPPORTED 0

// MAJOR and MINOR number must the the same in header, RELEASE is assumed to
// provide a backward compatibility
#define YaST_BYTECODE_HEADER "YaST xmlcode "
#define YaST_BYTECODE_MAJOR "1"
#define YaST_BYTECODE_MINOR "4"
#define YaST_BYTECODE_RELEASE "0"

#include "ycp/Xmlcode.h"
#include "YCP.h"
#include "ycp/YCode.h"
#include "ycp/YExpression.h"
#include "ycp/YStatement.h"
#include "ycp/YBlock.h"

#include "y2/Y2Namespace.h"

#include "ycp/y2log.h"
#include "ycp/pathsearch.h"

#include <fstream>
#include <errno.h>
#include <string.h>
#include <ctype.h>


static int
readInt( xmlcodeistream & str)
{
    int i = 0;

    char c;

    for( ;;)
    {
	str.get( c);
	if( !isdigit( c))
	    break;
	i *= 10;
	i +=( c - '0');
    }

    return i;
}


xmlcodeistream::xmlcodeistream( string filename)
    : std::ifstream( filename.c_str( ))
    , m_major( -1)
    , m_minor( -1)
    , m_release( -1)
{
    if( !is_open( ))
    {
	y2error( "Failed to open '%s': %s", filename.c_str(), strerror( errno));
	return;
    }
    // read YaST_BYTECODE_HEADER

    char header[sizeof(YaST_BYTECODE_HEADER)+1];
    int headerlen = strlen( YaST_BYTECODE_HEADER);
    read( header, headerlen);
    header[headerlen] = 0;
    if( strcmp( header, YaST_BYTECODE_HEADER) != 0)
    {
	y2error( "Not a xmlcode file '%s'[%s]", filename.c_str(), header);
	return;
    }

    m_major = readInt( *this);
    m_minor = readInt( *this);
    m_release = readInt( *this);
}

bool xmlcodeistream::isVersion( int major, int minor, int release)
{
    return( major == m_major) 
	&&( minor == m_minor) 
	&&( release == m_release);
}

bool xmlcodeistream::isVersionAtMost( int major, int minor, int release)
{
    if( m_major > major)
    {
	return false;
    }
    
    if( m_major == major)
    {
	if( m_minor > minor)
	{
	    return false;
	}
	
	if( ( m_minor == minor) &&( m_release > release))
	{
	    return false;
	}
    }

    return true;
}

int Xmlcode::m_namespace_nesting_level = -1;
int Xmlcode::m_namespace_nesting_array_size = 0;
int Xmlcode::m_namespace_tare_level = 0;
Xmlcode::namespaceentry_t *Xmlcode::m_namespace_nesting_array = 0;

void
Xmlcode::namespaceInit( )
{
#if DO_DEBUG
    y2debug( "Reinitialize namespaces");
#endif
    if( Xmlcode::m_namespace_nesting_array)
    {
	free( Xmlcode::m_namespace_nesting_array);
    }
    
    Xmlcode::m_namespace_nesting_array = 0;
    Xmlcode::m_namespace_nesting_level = -1;
    Xmlcode::m_namespace_nesting_array_size = 0;
    Xmlcode::m_namespace_tare_level = 0;
}

// ------------------------------------------------------------------
// xmlcode I/O

static int
to_hexc( unsigned char v )
{
    if( v < 10 ) return '0' + v;
    if( v < 16 ) return 'A' + v - 10;
    return -1;
}

std::ostream &
Xmlcode::writeBytep( std::ostream & str, const unsigned char * bytep, unsigned int len)
{
    unsigned int i = 0;
    str << "<bytes>";
    while( i < len ) {
	str << to_hexc(( *bytep & 0xf0) >> 4 ) << to_hexc( *bytep & 0x0f );
	bytep++;
    }
    return str << "</bytes>";
}


unsigned char *
Xmlcode::readBytep( xmlcodeistream & str)
{
    return 0;
}


// ------------------------------------------------------------------
// Type I/O

std::ostream &
Xmlcode::writeType( std::ostream & str, constTypePtr type)
{
    return type->toXml( str, 0 );
}


TypePtr
Xmlcode::readType( xmlcodeistream & str)
{
#if DO_DEBUG
y2debug( "Xmlcode::readType(%d)", kind);
#endif
#if XMLCODE_INPUT_SUPPORTED
    int kind = readInt32( str);
    switch( (Type::tkind)kind)
    {
	case Type::UnspecT:
	case Type::ErrorT:
	case Type::AnyT:
	case Type::BooleanT:
	case Type::ByteblockT:
	case Type::FloatT:
	case Type::IntegerT:
	case Type::LocaleT:
	case Type::PathT:
	case Type::StringT:
	case Type::SymbolT:
	case Type::TermT:
	case Type::VoidT:
	case Type::WildcardT:
	case Type::FlexT:
	case Type::NilT:
	    return TypePtr(  new Type( (Type::tkind)kind, str) );
	break;

	case Type::NFlexT:	return TypePtr(  new NFlexType( str) ); break;
	case Type::VariableT:	return TypePtr(  new VariableType( str) ); break;
	case Type::BlockT:	return TypePtr(  new BlockType( str) ); break;
	case Type::ListT:	return TypePtr(  new ListType( str) ); break;
	case Type::MapT:	return TypePtr(  new MapType( str) ); break;
	case Type::TupleT:	return TypePtr(  new TupleType( str) ); break;
	case Type::FunctionT:	return TypePtr(  new FunctionType( str) ); break;
    }
    y2error( "Unhandled type kind %d", kind);
#endif
    return Type::Error->clone();
}

// ------------------------------------------------------------------
// ycodelist_t * I/O

std::ostream &
Xmlcode::writeYCodelist( std::ostream & str, const ycodelist_t *codelist )
{
    const ycodelist_t *codep = codelist;

    while( codep)
    {
	str << "<element>";
	if( !codep->code->toXml( str, 0 ) )
	{
	    y2error( "Error writing codelist");
	    break;
	}
	codep = codep->next;
	str << "</element>";
    }

    return str;
}


bool
Xmlcode::readYCodelist( xmlcodeistream & str, ycodelist_t **anchor)
{

    return str.good();
}


// ------------------------------------------------------------------
// namespace stack handling

// find Id matching namespace
int
Xmlcode::namespaceId( const Y2Namespace *name_space)
{
    for( int i = m_namespace_tare_level; i <= m_namespace_nesting_level; i++)
    {
	if( m_namespace_nesting_array[i].name_space == name_space)
	{
	    return i - m_namespace_tare_level;
	}
    }
    y2error( "No ID for %p, level %d", name_space, m_namespace_nesting_level);
    return -1;
}


// retrieve namespace for ID
const Y2Namespace *
Xmlcode::namespacePtr( int namespace_id)
{
    // for entries without a name_space( foreach)
    if( namespace_id < 0) return 0;

    namespace_id += m_namespace_tare_level;
    if( namespace_id <= m_namespace_nesting_level)	// local namespace
    {
	return m_namespace_nesting_array[namespace_id].name_space;
    }
    y2error( "Block id %d > nesting_level %d", namespace_id - m_namespace_tare_level, m_namespace_nesting_level - m_namespace_tare_level);
    return 0;
}


// push namespace to stack
//  the stack resembles the nesting of namespaces
int
Xmlcode::pushNamespace( const Y2Namespace *name_space, bool with_xrefs)
{
    if( name_space == 0)
    {
	y2error( "Xmlcode::pushNamespace( %p) NULL", name_space);
	return -1;
    }

    m_namespace_nesting_level++;
    if( m_namespace_nesting_array_size <= m_namespace_nesting_level)
    {
	m_namespace_nesting_array_size += 16;
	m_namespace_nesting_array =( namespaceentry_t *)realloc( m_namespace_nesting_array, sizeof( namespaceentry_t) * m_namespace_nesting_array_size);
    }
#if DO_DEBUG
    y2debug( "Xmlcode::pushNamespace( %p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    m_namespace_nesting_array[m_namespace_nesting_level].name_space = name_space;
    m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs = with_xrefs;
    if( with_xrefs)
    {
	name_space->table()->openXRefs();
    }

    return m_namespace_nesting_level-m_namespace_tare_level;
}


// pop namespace from stack
//  the stack resembles the nesting of namespaces
int
Xmlcode::popNamespace( const Y2Namespace *name_space)
{
#if DO_DEBUG
    y2debug( "Xmlcode::popNamespace( %p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    if( name_space == 0)
    {
	y2error( "Xmlcode::popNamespace( %p) NULL", name_space);
	return -1;
    }

    if( m_namespace_nesting_level < m_namespace_tare_level)
    {
	y2error( "Xmlcode::popNamespace( %p) empty stack", name_space);
    }
    else if( m_namespace_nesting_array[m_namespace_nesting_level].name_space != name_space)
    {
	y2error( "Xmlcode::popNamespace( %p) not top of stack [%d]%p", name_space, m_namespace_nesting_level, m_namespace_nesting_array[m_namespace_nesting_level].name_space);
    }
    else
    {
	if( m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs)
	{
	    name_space->table()->closeXRefs();
	}
	m_namespace_nesting_level--;
    }
    return 0;
}


// pop all from id stack until given namespace is reached and popped too
void
Xmlcode::popUptoNamespace( const Y2Namespace *name_space)
{
#if DO_DEBUG
    y2debug( "Xmlcode::popUptoNamespace( %p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    if( name_space == 0)
    {
	y2error( "Xmlcode::popUptoNamespace( %p) NULL", name_space);
	return;
    }

    while( m_namespace_nesting_level >= m_namespace_tare_level)
    {
	const Y2Namespace *top_space = m_namespace_nesting_array[m_namespace_nesting_level].name_space;
	if( m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs)
	{
	    top_space->table()->closeXRefs();
	}
	m_namespace_nesting_level--;
	if( top_space == name_space)
	{
	    return;
	}
    }
    y2error( "Xmlcode::popUptoNamespace( %p) empty stack", name_space);
    return;
}


// reset current namespace stack to 'empty' for module loading
//   returns a tare id needed later
int
Xmlcode::tareStack( )
{
    int tare = m_namespace_nesting_level - m_namespace_tare_level + 1;
#if DO_DEBUG
//    y2debug( "Xmlcode::tareStack() level %d, size %d, current tare %d, tare_id %d", m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level, tare);
#endif
    m_namespace_tare_level = m_namespace_nesting_level + 1;
    return tare;
}


void
Xmlcode::untareStack( int tare_id)
{
#if DO_DEBUG
//    y2debug( "Xmlcode::untareStack() level %d, size %d, current tare %d, tare_id %d", m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level, tare_id);
#endif
    m_namespace_tare_level -= tare_id;
    return;
}

// ------------------------------------------------------------------
// SymbolEntry pointer( !) handling
//   the SymbolEntries itself are 'owned' by YBlock and referenced via pointers
//   to SymbolEntry. These functions handle Xml I/O for SymbolEntry pointers.
//
//  position is the index in namespace's m_senvironment[] for _local_ symbols
//  position is the index in module table's m_xrefs[] for _external_ symbols, see YSImport
//
std::ostream &
Xmlcode::writeEntry( std::ostream & str, const SymbolEntryPtr sentry)
{
    const Y2Namespace *name_space = sentry->nameSpace();
    string ns = name_space->name();
    str << "<entry";
    if (!ns.empty())
	str << " ns=\"" << ns << "\"";
    str << " pos=\"" << sentry->position();
    str << "\" name=\"" << sentry->name() << "\"/>";
    return str;
}


SymbolEntryPtr 
Xmlcode::readEntry( xmlcodeistream & str)
{
    return 0;
}

// ------------------------------------------------------------------
// YCode read


// read code from stream
YCodePtr
Xmlcode::readCode( xmlcodeistream & str)
{
    char code;
    if( !str.get( code))
    {
	y2error( "Can't read from stream");
	return 0;
    }
#if DO_DEBUG
//    y2debug( "Xmlcode::readCode( %d:%s)", code, YCode::toString( (YCode::ykind)code).c_str());
#endif
    YCodePtr res = 0;

#if XMLCODE_INPUT_SUPPORTED
    if( code < YCode::ycConstant)
    {
	return new YConst( (YCode::ykind)code, str);
    }
    
    // compatibility with 9.1/SLES
    if( str.isVersion( 1,3,2) && code > YCode::yeExpression)
    {
	// yeFunctionPointer did not exist then
	code++;
    }
    
    try
    {

    switch( code)
    {
	case YCode::ycConstant:
	{
	    // this constant is a placeholder, typically used by
	    // language bindings that cannot provide type information
	    y2error( "Unable to read constant, check the compilation of the module");
	    return 0;
	}
	case YCode::ycLocale:
	{
	    res = new YLocale( str);
	}
	break;
	case YCode::ycFunction:
	{
	    res = new YFunction( str);
	}
	break;
	case YCode::yePropagate:
	{
	    res = new YEPropagate( str);
	}
	break;
	case YCode::yeUnary:
	{
	    res = new YEUnary( str);
	}
	break;
	case YCode::yeBinary:
	{
	    res = new YEBinary( str);
	}
	break;
	case YCode::yeTriple:
	{
	    res = new YETriple( str);
	}
	break;
	case YCode::yeCompare:
	{
	    res = new YECompare( str);
	}
	break;
	case YCode::yeLocale:
	{
	    res = new YELocale( str);
	}
	break;
	case YCode::yeList:
	{
	    res = new YEList( str);
	}
	break;
	case YCode::yeMap:
	{
	    res = new YEMap( str);
	}
	break;
	case YCode::yeTerm:
	{
	    res = new YETerm( str);
	}
	break;
	case YCode::yeIs:
	{
	    res = new YEIs( str);
	}
	break;
	case YCode::yeBracket:
	{
	    res = new YEBracket( str);
	}
	break;
	case YCode::yeBlock:
	{
	    res = new YBlock( str);
	}
	break;
	case YCode::yeReturn:
	{
	    res = new YEReturn( str);
	}
	break;
	case YCode::yeVariable:
	{
	    res = new YEVariable( str);
	}
	break;
	case YCode::yeReference:
	{
	    res = new YEReference( str);
	}
	break;
	case YCode::yeBuiltin:
	{
	    res = new YEBuiltin( str);
	}
	break;
	case YCode::yeFunction:
	{
	    res = YECall::readCall( str);
	}
	break;
	case YCode::yeFunctionPointer:
	{
	    res = new YEFunctionPointer( str);
	}
	break;
	case YCode::ysTypedef:
	{
	    res = new YSTypedef( str);
	}
	break;
	case YCode::ysVariable:
	{
	    res = new YSVariable( str);
	}
	break;
	case YCode::ysFunction:
	{
	    res = new YSFunction( str);
	}
	break;
	case YCode::ysAssign:
	{
	    res = new YSAssign( str);
	}
	break;
	case YCode::ysBracket:
	{
	    res = new YSBracket( str);
	}
	break;
	case YCode::ysIf:
	{
	    res = new YSIf( str);
	}
	break;
	case YCode::ysWhile:
	{
	    res = new YSWhile( str);
	}
	break;
	case YCode::ysDo:
	{
	    res = new YSDo( str);
	}
	break;
	case YCode::ysRepeat:
	{
	    res = new YSRepeat( str);
	}
	break;
	case YCode::ysExpression:
	{
	    res = new YSExpression( str);
	}
	break;
	case YCode::ysReturn:
	{
	    res = new YSReturn( str);
	}
	break;
	case YCode::ysBreak:
	{
	    res = new YSBreak( str);
	}
	break;
	case YCode::ysContinue:
	{
	    res = new YSContinue( str);
	}
	break;
	case YCode::ysTextdomain:
	{
	    res = new YSTextdomain( str);
	}
	break;
	case YCode::ysInclude:
	{
	    res = new YSInclude( str);
	}
	break;
	case YCode::ysFilename:
	{
	    res = new YSFilename( str);
	}
	break;
	case YCode::ysImport:
	{
	    res = new YSImport( str);
	}
	break;
	case YCode::ysBlock:
	{
	    res = new YSBlock( str);
	}
	break;
	case YCode::ysSwitch:
	{
	    res = new YSSwitch( str);
	}
	break;
	default:
	{
	    y2error( "Unknown code %d", code);
	}
	break;
    }

    }
    catch( const Xmlcode::Invalid&)
    {
	// there are memory leaks all over the place now
	y2error( "Caught invalid xmlcode");
    }
#endif
    return res;
}


// ------------------------------------------------------------------
// File I/O

// static member
map <string, YBlockPtr>* Xmlcode::m_xmlcodeCache = NULL;

// read file from module path

YBlockPtr 
Xmlcode::readModule( const string & mname)
{
#if DO_DEBUG
//    y2debug( "Xmlcode::readModule( %s) ", mname.c_str( ));
#endif

    // TODO better error reporting?
    // like: could not find foo.ycp in /modules, /a/modules.
    // It will return an empty string on failure

    string filename = YCPPathSearch::findModule( mname);
    if( filename.empty())
    {
	ycperror( "Module '%s' not found", mname.c_str());
	return 0;
    }
    
    if( ! m_xmlcodeCache)
    {
	m_xmlcodeCache = new map <string, YBlockPtr>;
    }

    // check the cache
    if( m_xmlcodeCache->find( mname) != m_xmlcodeCache->end( ))
    {
#if DO_DEBUG
//	y2debug( "Xmlcode cache hit: %s", mname.c_str( ));
#endif

	return m_xmlcodeCache->find( mname)->second;
    }
    
    int tare_id = Xmlcode::tareStack( );			// current nesting level is 0 for this module
    YBlockPtr block =( YBlockPtr)Xmlcode::readFile( filename);

    if( block == NULL)
    {
	return NULL;
    }

    Xmlcode::untareStack( tare_id);

    if( !block->isModule())
    {
	y2error( "'%s' is no module", filename.c_str());
	return NULL;
    }

    m_xmlcodeCache->insert( std::make_pair( mname, block));

    return block;
}


// read YCode from file, return YCode( 0 in case of failure)
YCodePtr
Xmlcode::readFile( const string & filename)
{
#if DO_DEBUG
//    y2debug( "Xmlcode::readFile( %s)", filename.c_str());
#endif
    xmlcodeistream instream( filename);
    if( !instream.is_open( ))
    {
	y2error( "Failed to open '%s': %s", filename.c_str(), strerror( errno));
	return 0;
    }
    // check YaST_BYTECODE_HEADER
    if(  
	instream.isVersion( 
	    atoi( YaST_BYTECODE_MAJOR)
	    , atoi( YaST_BYTECODE_MINOR)
	    , atoi( YaST_BYTECODE_RELEASE))
	||
	instream.isVersion( 1,3,2) )	// 9.1/SLES9
    {
#if DO_DEBUG
//	y2debug( "Header accepted");
#endif
	
	return readCode( instream);
    }

    y2error( "Unsupported version %d.%d.%d"
	, instream.major( )
	, instream.minor( )
	, instream.release( ));
    return 0;
}


// write YCode to file, return false on error( i.e. file not existing - see errno)
bool
Xmlcode::writeFile( const YCodePtr code, const string & filename)
{
    // clear errno first
    errno = 0;

    std::ofstream outstream( filename.c_str());
    if( !outstream.is_open( ))
    {
	y2error( "Failed to write '%s': %s", filename.c_str(), strerror( errno));
	return false;
    }
    // write XML without any localization
    outstream.imbue(std::locale("C"));

    outstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    outstream << "<ycp version=\"2.15.8.39280\">\n";
    code->toXml( outstream, 2 );
    outstream << "</ycp>\n";
    return ! outstream.fail( );
}


// indentation

string
Xmlcode::spaces( int count )
{
    count >>= 1;
    if (count <= 0) return "";

    //                      2     4       6         8     10      12        14          16      18        20
    static string s[10] = { "  ", "    ", "      ", "\t", "\t  ", "\t    ", "\t      ", "\t\t", "\t\t  ", "\t\t    " };

    if( count > 10 ) {
	return s[9] + spaces( count*2 - 20 );
    }
    return s[count - 1];
}

string
Xmlcode::xmlify( const string & s )
{
    string result;

    const char *cptr = s.c_str();
    const char *next;
    while( (next = strpbrk( cptr, "&<>'\"\n\t" )) ) {
	result += string( cptr, next - cptr );
	switch (*next) {
	  case '&': result += "&amp;"; break;
	  case '<': result += "&lt;"; break;
	  case '>': result += "&gt;"; break;
	  case '"': result += "&quot;"; break;
	  case '\'': result += "&apos;"; break;
	  case '\n': result += "&#xA;"; break;
	  case '\t': result += "&#x9;"; break;
	}
	cptr = next + 1;
    }
    result += string( cptr );
    return result;

}
