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
 * usable outside PkgModuleCallbacks.cc because both class definitions reside
 * within the implementation file.
 *
 * <H5>How to introduce new YCP callbacks</H5>
 * <OL>
 * <LI>Consult PkgModuleCallbacks.YCP.h and introduce a new value in enum CBid
 *     (and adjust switch in cbName). This enum value is used to set and access
 *     the YCP callbacks data.
 * <LI>Consult PkgModuleCallbacks.cc and implement PkgModuleFunctions::CallbackWhateverName,
 *     to set the calbacks module and symbol.
 * </OL>
 * <H5>How to introduce new recipient which triggers the YCP callbacks</H5>
 * <OL>
 * <LI>Consult PkgModuleCallbacks.cc
 * <LI>Within namespace Y2PMRecipients define the new recipient class, which is
 *     usg. derived from Recipient and some calback interface class provided
 *     by Y2PM or some of it's components.
 * <LI>In class Y2PMReceive create an instance of your recipient, and adjust
 *     constructor and destructor to setup and clear the redirection of the
 *     Report (also provided by Y2PM or some of it's components) you want to
 *     receive.
 * </OL>
 * Sounds more complicated than it actually is. Take an existing recipient as
 * example. Consider class RecipientCtl, which is inherited by Y2PMReceive and
 * shared among the recipient classes, if you need to exchage data or coordinate
 * different recipients.
 *
 * See also class @ref Report (in libutil).
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
