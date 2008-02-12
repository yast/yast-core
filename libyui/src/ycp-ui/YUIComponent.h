/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:		YUIComponent.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUIComponent_h
#define YUIComponent_h


#include <Y2.h>
#include <ycp/YCPValue.h>


class YUI;
class Y2Namespace;

/**
 * abstract base class for YaST2 user interface components.
 * Derive your own component class from this one and implement createUI().
 **/
class YUIComponent : public Y2Component
{
public:
    /**
     * Constructor.
     **/
    YUIComponent();

    /**
     * Destructor.
     **/
    virtual ~YUIComponent();

protected:
    /**
     * Create the UI. This is called when all the information for doing that is
     * complete, in setServerOptions().
     * 'argc' and 'argv' are the command line arguments.
     *
     * Implement this method in derived classes.
     **/
    virtual YUI * createUI( bool withThreads );

public:

    virtual Y2Namespace *import (const char* name);

    /**
     * Returns the instance of the UI component 0 if none has been created yet.
     **/
    static YUIComponent * uiComponent();

    /**
     * Returns the instance of the UI or 0 if none has been created yet.
     * Note: This does _not_ create a UI on the first call; this happens in the
     * first call of a UI builtin via the YUIComponent's call handler which
     * creates a UI upon its first call and then calls the UI's call handler.
     **/
    static YUI * ui() { return _ui; }

    /**
     * Create a UI instance. The UI component normally handles that all by
     * itself when the first UI builtin is called.
     **/
    void createUI();

    /**
     * YUIComponent level call handler; this creates the actual UI instance
     * upon its first call and then hands over the function to be called to the
     * UI's call handler to make sure it is executed in the correct thread.
     **/
    YCPValue callBuiltin( void * function, int fn_argc, YCPValue fn_argv[] );
    
    /**
     * Called from generic frontend upon session close.
     * This deletes the UI.
     **/
    void result( const YCPValue & result );

    /**
     * This is called by the generic frontend after it parsed the commandline.
     * This actually creates an UI instance with createUI().
     **/
    void setServerOptions( int argc, char ** argv );

    /**
     * The name of the component - the prefix used for builtin calls like
     * UI::OpenDialog() etc.
     **/
    virtual string name() const { return string( "UI" ); }

    /**
     * The name of a macro file that might have been passed as a -macro
     * command line argument or 0 if none
     **/
    const char * macroFile() const { return _macroFile; }
    
    /**
     * Set a callback component.
     **/
    void setCallback( Y2Component * callbackComponent )
	{ _callbackComponent = callbackComponent; }
    
    /**
     * Return the UI's callback component previously set with setCallback().
     **/
    Y2Component * getCallback() const { return _callbackComponent; }
    
    
private:

    static YUI *		_ui;
    static YUIComponent *	_uiComponent;
    Y2Namespace *		_namespace;
    Y2Component *		_callbackComponent;

    bool			_withThreads;
    const char *		_macroFile;
    bool			_haveServerOptions;
};

#endif // YUIComponent_h
