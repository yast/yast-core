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

  File:       PkgModuleCallbacks.YCP.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Implementation of PkgModuleFunctions::CallbackHandler::YCPCallbacks
           (not intended to be distributed)

/-*/
#ifndef PkgModuleCallbacksYCP_h
#define PkgModuleCallbacksYCP_h

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>
#include <y2util/Date.h>
#include <y2util/FSize.h>
#include <y2util/Pathname.h>

#include <ycp/YCPInterpreter.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPError.h>

#include <PkgModuleCallbacks.h>

using namespace std;

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
inline ostream & operator<<( ostream & str, const YCPValueType & obj )
{
  switch ( obj ) {
#define ENUMOUT(V) case V: return str << #V; break
    ENUMOUT( YT_UNDEFINED );
    ENUMOUT( YT_VOID );
    ENUMOUT( YT_BOOLEAN );
    ENUMOUT( YT_INTEGER );
    ENUMOUT( YT_FLOAT );
    ENUMOUT( YT_STRING );
    ENUMOUT( YT_BYTEBLOCK );
    ENUMOUT( YT_PATH );
    ENUMOUT( YT_SYMBOL );
    ENUMOUT( YT_DECLARATION );
    ENUMOUT( YT_LOCALE );
    ENUMOUT( YT_LIST );
    ENUMOUT( YT_TERM );
    ENUMOUT( YT_MAP );
    ENUMOUT( YT_BLOCK );
    ENUMOUT( YT_BUILTIN );
    ENUMOUT( YT_IDENTIFIER );
    ENUMOUT( YT_ERROR );
#undef ENUMOUT
  }
  return str << "UNKNOWN(YCPValueType)";
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
inline ostream & operator<<( ostream & str, const YCPValue & obj )
{
  return str << obj->valuetype();
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkgModuleFunctions::CallbackHandler::YCPCallbacks
/**
 * @short Stores YCPCallback related data and communicates with YCPInterpreter
 *
 * For each YCPCallback, it's data identified by the <CODE>CB_id</CODE>
 * are stored in maps.
 *
 * To invoke a YCPCallback:
 * <PRE>
 *   YCPTerm callback = createCallback( CB_PatchProgress ); // create callback Term
 *   callback->add( YCPInteger ( percent ) );               // add arguments
 *   callback->add( YCPString ( pkg ) );
 *   bool result = evaluateBool( callback );                // evaluate
 * </PRE>
 **/
class PkgModuleFunctions::CallbackHandler::YCPCallbacks
{
  public:

    /**
     * Unique id for each YCPCallback we may have to trigger
     **/
    enum CBid {
      CB_ProgressRebuildDB,
      CB_StartConvertDb,
      CB_ProgressConvertDb,
      CB_NotifyConvertDb,
      CB_StopConvertDb,
      CB_StartProvide,
      CB_ProgressProvide,
      CB_DoneProvide,
      CB_StartPackage,
      CB_ProgressPackage,
      CB_DonePackage,
      CB_MediaChange,
      CB_SourceChange,
      CB_YouProgress,
      CB_YouPatchProgress,
      CB_YouExecuteYcpScript,
      CB_YouScriptProgress
    };

    /**
     * Returns the enum name without the leading "CB_"
     * (e.g. "StartProvide" for CB_StartProvide).
     **/
    static string cbName( CBid id_r ) {
      switch ( id_r ) {
#define ENUM_OUT(N) case CB_##N: return #N
	ENUM_OUT( ProgressRebuildDB );
	ENUM_OUT( StartConvertDb );
	ENUM_OUT( ProgressConvertDb );
	ENUM_OUT( NotifyConvertDb );
	ENUM_OUT( StopConvertDb );
	ENUM_OUT( StartProvide );
	ENUM_OUT( ProgressProvide );
	ENUM_OUT( DoneProvide );
	ENUM_OUT( StartPackage );
	ENUM_OUT( ProgressPackage );
	ENUM_OUT( DonePackage );
	ENUM_OUT( MediaChange );
	ENUM_OUT( SourceChange );
	ENUM_OUT( YouProgress );
	ENUM_OUT( YouPatchProgress );
	ENUM_OUT( YouExecuteYcpScript );
	ENUM_OUT( YouScriptProgress );
#undef ENUM_OUT
	// no default! let compiler warn missing values
      }
      return stringutil::form( "CBid(%d)", id_r );
    }

  private:

    // mutable: queries may create empty entries if they do not yet exist
    mutable map<CBid,string> _mModules;
    mutable map<CBid,string> _mSymbols;

#warning Hack to share refcounted PkgModule between WFMInterpreter instances
    YCPInterpreter *& _interpreter; // pointer reference to PkgModuleCtrl

  public:

    /**
     * Constructor.
     **/
    YCPCallbacks( YCPInterpreter *& interpreter )
      : _interpreter( interpreter )
    {
      if ( !_interpreter ) {
	INT << "NULL YCPInterpreter: can't send callbacks" << endl;
      }
    }

    /**
     * Set a YCPCallbacks data from string "module::symbol"
     **/
    void setCallback( CBid id_r, const string & name_r ) {
      string::size_type colonpos = name_r.find("::");
      if ( colonpos != string::npos ) {
	_mModules[id_r] = name_r.substr ( 0, colonpos );
	_mSymbols[id_r] = name_r.substr ( colonpos + 2 );
      } else {
	_mModules[id_r] = "";
	_mSymbols[id_r] = name_r;
      }
    }
    /**
     * Set a YCPCallbacks data according to args_r.
     **/
    bool setCallback( CBid id_r, const YCPList & args ) {
      if ( ! ( args->size() == 1 && args->value(0)->isString() ) ) {
	return false;
      }
      string name = args->value(0)->asString()->value();
      setCallback( id_r, name );
      return true;
    }
    /**
     * Set the YCPCallback according to args_r.
     * @return YCPVoid on success, otherwise YCPError.
     **/
    YCPValue setYCPCallback( CBid id_r, const YCPList & args_r ) {
      if ( ! setCallback( id_r, args_r ) ) {
	return YCPError( string("Bad args to Pkg::Callback") + cbName( id_r ) );
      }
      return YCPVoid();
    }

    /**
     * @return Whether the YCPCallback is set. If not, there's
     * no need to create and evaluate it.
     **/
    bool isSet( CBid id_r ) const {
      return( _interpreter && !_mSymbols[id_r].empty() );
    }

  public:

    /**
     * @return The YCPCallback term, ready to append any arguments.
     **/
    YCPTerm createCallback( CBid id_r ) const {
      return YCPTerm( YCPSymbol( _mSymbols[id_r], false ), _mModules[id_r] );
    }

    /**
     * @return Evaluated YCPCallback term.
     **/
    YCPValue evaluate( const YCPTerm & callback ) const {
      if ( !_interpreter ) {
	return YCPVoid();
      }
      return _interpreter->evaluate( callback );
    }

  public:

    /**
     * @short Convenience base class for YCPCallback sender
     *
#warning Free interface for YCPCallback sending is ok, but a functional one is desired too.
     * A functional interface for sending YCPCallbacks with well known arguments
     * and return values is desirable. Esp. for YCPcallbacks triggered from
     * multiple recipients. Currently each recipient has to implememt correct
     * number and type of arguments, as well as the returned type. That's bad
     * if something changes. As soon as YCPCallbacks provides them (as const methods),
     * Y2PMRecipients::Recipient should no longer inherit Send, but provide
     * an easy access to RecipientCtl::_ycpcb.
     **/
    class Send {
      public:
	/**
	 * @short Convenience class for YCPCallback sending
	 **/
	struct CB {
	  const Send & _send;
	  bool     _set;
	  YCPTerm  _term;
	  YCPValue _result;
	  CB( const Send & send_r, CBid func )
	    : _send( send_r )
	    , _set( _send.ycpcb().isSet( func ) )
	    , _term( _send.ycpcb().createCallback( func ) )
	    , _result( YCPVoid() )
	  {}

	  CB & addStr( const string & arg ) { _term->add( YCPString( arg ) ); return *this; }
	  CB & addStr( const Pathname & arg ) { return addStr( arg.asString() ); }

	  CB & addInt( long long arg ) { _term->add( YCPInteger( arg ) ); return *this; }

	  CB & addBool( bool arg ) { _term->add( YCPBoolean( arg ) ); return *this; }

	  bool isStr() const { return _result->isString(); }
	  bool isInt() const { return _result->isInteger(); }
	  bool isBool() const { return _result->isBoolean(); }

	  bool expecting( YCPValueType exp_r ) const {
	    if ( _result->valuetype() == exp_r )
	      return true;
	    INT << "Wrong return type " << _result->valuetype() << ": Expected " << exp_r << endl;
	    return false;
	  }

	  bool evaluate() {
	    if ( _set ) {
	      _result = _send.ycpcb().evaluate( _term );
	      return true;
	    }
	    return false;
	  }

	  bool evaluate( YCPValueType exp_r ) {
	    return evaluate() && expecting( exp_r );
	  }

	  string evaluateStr( const string & def_r = "" ) {
	    return evaluate( YT_STRING ) ? _result->asString()->value() : def_r;
	  }

	  long long evaluateInt( const long long & def_r = 0 ) {
	    return evaluate( YT_INTEGER ) ? _result->asInteger()->value() : def_r;
	  }

	  bool evaluateBool( const bool & def_r = false ) {
	    return evaluate( YT_BOOLEAN ) ? _result->asBoolean()->value() : def_r;
	  }
	};
      private:
	const YCPCallbacks & _ycpcb;
      public:
	Send( const YCPCallbacks & ycpcb_r ) : _ycpcb( ycpcb_r ) {}
	virtual ~Send() {}
	const YCPCallbacks & ycpcb() const { return _ycpcb; }
	CB ycpcb( CBid func ) const { return CB( *this, func ); }
    };
};

///////////////////////////////////////////////////////////////////

#endif // PkgModuleCallbacksYCP_h
