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
  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUIComponent_h
#define YUIComponent_h


#include <Y2.h>
#include <ycp/YCPValue.h>


class YUI;

/**
 * @short abstract base class for YaST2 user interface components.
 * Derive your own component class from this one and implement createUI().
 **/
class YUIComponent : public Y2Component
{
protected:
    /**
     * Constructor.
     */
    YUIComponent();

    /**
     * Destructor.
     */
    virtual ~YUIComponent();

    /**
     * Create the UI. This is called when all the information for doing that is
     * complete, in setServerOptions().
     * 'argc' and 'argv' are the command line arguments.
     *
     * Implement this method in derived classes.
     **/
    virtual YUI * createUI( int argc, char **argv, bool with_threads, const char * macro_file ) = 0;

public:

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
     * YUIComponent level call handler; this creates the actual UI instance
     * upon its first call and then hands over the function to be called to the
     * UI's call handler. Weird, huh? ;-)
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
     */
    void setServerOptions( int argc, char ** argv );

    /**
     * The name of the component - the prefix used for builtin calls like
     * UI::OpenDialog() etc.
     */
    virtual string name() const { return string( "UI" ); }
    
    /**
     * Set a callback component - call the UI's setCallback() method.
     **/
    void setCallback( Y2Component * callback );
    
    /**
     * Returns the UI's callback component previously set with setCallback();
     * calls the UI's getCallback() method.
     **/
    Y2Component * getCallback() const;
    
    
private:

    static YUI *		_ui;
    static YUIComponent *	_uiComponent;

    int			_argc;
    char **		_argv;
    const char *	_macro_file;
    bool		_with_threads;

    
};

#endif // YUI_h
