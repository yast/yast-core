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

#include <stack>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>
#include <y2util/Date.h>
#include <y2util/FSize.h>
#include <y2util/Url.h>
#include <y2util/Pathname.h>

#include <y2/Y2ComponentBroker.h>
#include <y2/Y2Component.h>
#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>

#include <ycpTools.h>
#include <PkgModuleCallbacks.h>

#include <ycp/y2log.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkgModuleFunctions::CallbackHandler::YCPCallbacks
/**
 * @short Stores YCPCallback related data and communicates with Y2ComponentBroker
 *
 * For each YCPCallback it's data, identified by a <CODE>@ref CBid</CODE>,
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
     * Unique id for each YCPCallback we may have to trigger.
     * On changes here, adapt @ref cbName.
     **/
    enum CBid {
      CB_StartRebuildDb, CB_ProgressRebuildDb, CB_NotifyRebuildDb, CB_StopRebuildDb,
      CB_StartConvertDb, CB_ProgressConvertDb, CB_NotifyConvertDb, CB_StopConvertDb,
      CB_StartProvide, CB_ProgressProvide, CB_DoneProvide,
      CB_StartPackage, CB_ProgressPackage, CB_DonePackage,
      CB_StartDownload, CB_ProgressDownload, CB_DoneDownload,
      CB_MediaChange,
      CB_SourceChange,
      CB_YouProgress,
      CB_YouPatchProgress,
      CB_YouError,
      CB_YouMessage,
      CB_YouLog,
      CB_YouExecuteYcpScript,
      CB_YouScriptProgress
    };

    /**
     * Returns the enum name without the leading "CB_"
     * (e.g. "StartProvide" for CB_StartProvide). Should
     * be in sync with @ref CBid.
     **/
    static string cbName( CBid id_r ) {
      switch ( id_r ) {
#define ENUM_OUT(N) case CB_##N: return #N
	ENUM_OUT( StartRebuildDb );
	ENUM_OUT( ProgressRebuildDb );
	ENUM_OUT( NotifyRebuildDb );
	ENUM_OUT( StopRebuildDb );
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
	ENUM_OUT( StartDownload );
	ENUM_OUT( ProgressDownload );
	ENUM_OUT( DoneDownload );
	ENUM_OUT( MediaChange );
	ENUM_OUT( SourceChange );
	ENUM_OUT( YouProgress );
	ENUM_OUT( YouPatchProgress );
	ENUM_OUT( YouError );
        ENUM_OUT( YouMessage );
	ENUM_OUT( YouLog );
	ENUM_OUT( YouExecuteYcpScript );
	ENUM_OUT( YouScriptProgress );
#undef ENUM_OUT
	// no default! let compiler warn missing values
      }
      return stringutil::form( "CBid(%d)", id_r );
    }

  private:

    struct CBdata
    {
       CBdata (string module, string symbol, Y2Namespace* component)
           : module (module), symbol (symbol), component (component)
           { }

       const string module, symbol;
       Y2Namespace* component;
    };

    typedef map <CBid, stack <CBdata> > _cbdata_t;
    _cbdata_t _cbdata;

  public:

    /**
     * Constructor.
     **/
    YCPCallbacks( )
    {}


    void popCallback( CBid id_r ) {
       _cbdata_t::iterator tmp1 = _cbdata.find(id_r);
       if (tmp1 != _cbdata.end() && !tmp1->second.empty())
           tmp1->second.pop();
    }

    /**
     * Set a YCPCallbacks data from string "module::symbol"
     **/
    void setCallback( CBid id_r, const string & name_r ) {
      y2debug ("Registering callback %s", name_r.c_str ());
      string::size_type colonpos = name_r.find("::");
      if ( colonpos != string::npos ) {

        string module = name_r.substr ( 0, colonpos );
	string symbol = name_r.substr ( colonpos + 2 );

        Y2Component *c = Y2ComponentBroker::getNamespaceComponent (module.c_str());
        if (c == NULL)
        {
          ycp2error ("No component can provide namespace %s for a callback of %s (callback id %d)",
                 module.c_str (), symbol.c_str (), id_r);
          return;
        }

        Y2Namespace *ns = c->import (module.c_str ());
        if (ns == NULL)
        {
          y2error ("Component %p could not provide namespace %s for a callback of %s",
                 c, module.c_str (), symbol.c_str ());
	  return;
        }

	// ensure it is an initialized namespace
	ns->initialize ();

        _cbdata[id_r].push (CBdata (module, symbol, ns));
      } else {
	ycp2error ("Callback must be a part of a namespace");
      }
    }
    /**
     * Set a YCPCallbacks data according to args_r.
     **/
    bool setCallback( CBid id_r, const YCPString & args ) {
      string name = args->value();
      setCallback( id_r, name );
      return true;
    }
    /**
     * Set the YCPCallback according to args_r.
     * @return YCPVoid on success, otherwise YCPError.
     **/
    YCPValue setYCPCallback( CBid id_r, const YCPString & args_r ) {
       if (!args_r->value().empty ())
       {
    	   if ( ! setCallback( id_r, args_r ) ) {
		return YCPError( string("Bad args to Pkg::Callback") + cbName( id_r ) );
	    }
       }
       else
       {
           popCallback( id_r );
       }
       return YCPVoid();
    }

    /**
     * @return Whether the YCPCallback is set. If not, there's
     * no need to create and evaluate it.
     **/
    bool isSet( CBid id_r ) const {
       const _cbdata_t::const_iterator tmp1 = _cbdata.find(id_r);
       return tmp1 != _cbdata.end() && !tmp1->second.empty();
    }

  public:

    /**
     * @return The YCPCallback term, ready to append any arguments.
     **/
    Y2Function* createCallback( CBid id_r ) const {
       const _cbdata_t::const_iterator tmp1 = _cbdata.find(id_r);
       if (tmp1 == _cbdata.end() || tmp1->second.empty())
           return NULL;
       const CBdata& tmp2 = tmp1->second.top();

      string module = tmp2.module;
      string name = tmp2.symbol;
      Y2Namespace *ns = tmp2.component;
      if (ns == NULL)
      {
          y2error ("No namespace %s for a callback of %s", module.c_str (), name.c_str ());
	  return NULL;
      }

      Y2Function* func = ns->createFunctionCall (name, Type::Unspec); // FIXME: here we can setup the type check
      if (func == NULL)
      {
          ycp2error ("Cannot find function %s in module %s as a callback", name.c_str (), module.c_str());
	  return NULL;
      }

      return func;
    }

  public:

#warning Free interface for YCPCallback sending is ok, but a functional one is desired too.
    /**
     * @short Convenience base class for YCPCallback sender
     *
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
	  CBid _id;
	  bool     _set;
	  Y2Function* _func;
	  YCPValue _result;
	  CB( const Send & send_r, CBid func )
	    : _send( send_r )
	    , _id( func )
	    , _set( _send.ycpcb().isSet( func ) )
	    , _func( _send.ycpcb().createCallback( func ) )
	    , _result( YCPVoid() )
	  {}

	  ~CB ()
	  {
	    if (_func) delete _func;
	  }

	  CB & addStr( const string & arg ) { if (_func != NULL) _func->appendParameter( YCPString( arg ) ); return *this; }
	  CB & addStr( const Pathname & arg ) { return addStr( arg.asString() ); }
	  CB & addStr( const Url & arg ) { return addStr( arg.asString() ); }

	  CB & addInt( long long arg ) { if (_func != NULL) _func->appendParameter( YCPInteger( arg ) ); return *this; }

	  CB & addBool( bool arg ) { if (_func != NULL) _func->appendParameter( YCPBoolean( arg ) ); return *this; }

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
	    if ( _set && _func ) {
	      y2debug ("Evaluating callback");
	      _result = _func->evaluateCall ();

	      delete _func;
	      _func = _send.ycpcb().createCallback( _id );
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
