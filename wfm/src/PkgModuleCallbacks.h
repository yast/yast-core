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

  File:       PkgModuleCallbacks.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Handler for Callbacks from Y2PM to UI/WFM.

/-*/
#ifndef PkgModuleCallbacks_h
#define PkgModuleCallbacks_h

#include <iosfwd>

#include <PkgModuleFunctions.h>

class YCPInterpreter;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkgModuleFunctions::CallbackHandler
/**
 * @short Handler for Callbacks received or triggered. Needs access to WFM.
 *
 * <B>NOTE:</B> Public references to YCPCallbacks and Y2PMReceive are not
 * usable outside PkgModuleCallbacks.cc
 **/
class PkgModuleFunctions::CallbackHandler {
  CallbackHandler & operator=( const CallbackHandler & );
  CallbackHandler            ( const CallbackHandler & );

  public:

    /**
     * Manages the YCPCallbacks we trigger.
     **/
    class YCPCallbacks;
    YCPCallbacks & _ycpCallbacks;

    /**
     * Manages the Y2PMCallbacks we receive.
     **/
    class Y2PMReceive;
    Y2PMReceive & _y2pmReceive;

  public:

    /**
     * We don't like to, but need access to these data, until
     * unique InstSrcId's and inst_order are handled within
     * InstSrcManager. (InstSrcId's are pointer values, thus
     * unique among existing sources, but reusable if sources
     * vanish and appear);
     *
     * Without this no nedd to include PkgModuleFunctions.h.
     **/
    struct ReferencesNeeded {
      const vector<InstSrcManager::ISrcId> & _sources;
      const InstSrcManager::ISrcIdList &     _inst_order;
      ReferencesNeeded( const vector<InstSrcManager::ISrcId> & sources_r,
			const InstSrcManager::ISrcIdList &     inst_order_r )
	: _sources( sources_r )
	, _inst_order( inst_order_r )
      {}
    };

    /**
     * Constructor. Setup handler and redirect Y2PMCallbacks
     * to the Y2PMReceiver.
     **/
    CallbackHandler( YCPInterpreter *const wfm_r, ReferencesNeeded args_r );

    /**
     * Destructor. Reset Y2PMCallbacks to it's defaults.
     **/
    ~CallbackHandler();
};

///////////////////////////////////////////////////////////////////

#endif // PkgModuleCallbacks_h
