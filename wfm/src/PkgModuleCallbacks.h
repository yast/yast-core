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

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkgModuleFunctions::CallbackHandler
/**
 * @short Handler for Callbacks received or triggered. Needs access to WFM.
 *
 * <B>NOTE:</B> Public references to @ref YCPCallbacks and @ref Y2PMReceive are
 * intentionally not usable outside PkgModuleCallbacks.cc, because both class
 * definitions reside within the implementation file. They are public because
 * callback realated @ref PkgModuleFunctions methods are defined in the same file
 * and use them to set the YCP callbacks. Appart from this, there's no need to
 * propagate the interface.
 *
 * <H5>How to introduce new YCP callbacks</H5>
 * <OL>
 * <LI>In class @ref YCPCallbacks introduce a new value in enum CBid.
 *     This enum value is used to set and access the YCP callbacks data.
 * <LI>In class @ref PkgModuleFunctions declare a method CallbackWhateverName,
 *     and implement it in PkgModuleCallbacks.cc to set the calback data.
 * <LI>Finaly adjust @ref PkgModule::evaluate to make the method available to
 *     the YCP code.
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
 * See also class @ref Report (in liby2util).
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
     * Constructor. Setup handler and redirect Y2PMCallbacks
     * to the Y2PMReceiver.
     **/
    CallbackHandler();

    /**
     * Destructor. Reset Y2PMCallbacks to it's defaults.
     **/
    ~CallbackHandler();
};

///////////////////////////////////////////////////////////////////

#endif // PkgModuleCallbacks_h
