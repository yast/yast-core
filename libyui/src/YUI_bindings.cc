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

   File:	YUI_bindings.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>

   Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#include "UI.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#define y2log_component "ui"
#include "ycp/y2log.h"

#include "YUI.h"


static YCPValue
UISetLanguage( const YCPString & language )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetLanguage( language );

    return YCPVoid();
}


static YCPValue
UISetLanguage2( const YCPString & language, const YCPString & encoding )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetLanguage( language, encoding );

    return YCPVoid();
}


static YCPValue
UIGetProductName()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateGetProductName();
    else
	return YCPVoid();
}


static YCPValue
UISetProductName( const YCPString & name )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetProductName( name );

    return YCPVoid();
}


static YCPValue
UISetConsoleFont( const YCPString & console_magic,
		  const YCPString & font,
		  const YCPString & screen_map,
		  const YCPString & unicode_map,
		  const YCPString & encoding )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetConsoleFont( console_magic,
						 font,
						 screen_map,
						 unicode_map,
						 encoding );
    return YCPVoid();
}


static YCPValue
UISetKeyboard()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetKeyboard();

    return YCPVoid();
}


static YCPValue
UIGetLanguage( const YCPBoolean & strip )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateGetLanguage( strip );
    else
	return YCPVoid();
}


static YCPValue
UIUserInput()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateUserInput();
    else
	return YCPVoid();
}


static YCPValue
UIPollInput()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluatePollInput();
    else
	return YCPVoid();
}


static YCPValue
UITimeoutUserInput( const YCPInteger& timeout )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateTimeoutUserInput( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateWaitForEvent();
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent1( const YCPInteger& timeout )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateWaitForEvent( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog2( const YCPTerm & opts, const YCPTerm & dialog_term )
{
    if ( YUI::instance() )
	// Notice: Parameter order is switched!
	return YUI::instance()->evaluateOpenDialog( dialog_term, opts );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog( const YCPTerm & dialog_term )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateOpenDialog( dialog_term );
    else
	return YCPVoid();
}


static YCPValue
UICloseDialog()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateCloseDialog();
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget( const YCPSymbol & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget1( const YCPTerm & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget( const YCPSymbol & widget_id, const YCPSymbol & property )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget1( const YCPSymbol & widget_id, const YCPTerm & property )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget2( const YCPTerm & widget_id, const YCPSymbol & property )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget3( const YCPTerm & widget_id, const YCPTerm & property )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget( const YCPSymbol & widget_id, const YCPTerm & new_widget )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget1( const YCPTerm & widget_id, const YCPTerm & new_widget )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus( const YCPSymbol & widget_id )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus1( const YCPTerm & widget_id )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIBusyCursor()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateBusyCursor();

    return YCPVoid();
}


static YCPValue
UIRedrawScreen()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateRedrawScreen();

    return YCPVoid();
}


static YCPValue
UINormalCursor()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateNormalCursor();

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot1( const YCPString & filename )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateMakeScreenShot( filename );

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot()
{
    if ( YUI::instance() )
	return UIMakeScreenshot1( YCPNull() );
    else
	return YCPVoid();
}


static YCPValue
UIDumpWidgetTree()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateDumpWidgetTree();

    return YCPVoid();
}


static YCPValue
UIRecordMacro( const YCPString & filename )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateRecordMacro( filename );

    return YCPVoid();
}


static YCPValue
UIStopRecordMacro()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateStopRecordMacro();

    return YCPVoid();
}


static YCPValue
UIPlayMacro( const YCPString & filename )
{
    if ( YUI::instance() )
	YUI::instance()->evaluatePlayMacro( filename );

    return YCPVoid();
}


static YCPValue
UIFakeUserInput( const YCPValue & next_input )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateFakeUserInput( next_input );

    return YCPVoid();
}


static YCPValue
UIGlyph( const YCPSymbol & glyphSym  )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateGlyph( glyphSym );
    else
	return YCPString( "*" );
}


static YCPValue
UIGetDisplayInfo()
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateGetDisplayInfo();
    else
	return YCPVoid();
}


static YCPValue
UIRecalcLayout()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateRecalcLayout();

    return YCPVoid();
}


static YCPValue
UIPostponeShortcutCheck()
{
    if ( YUI::instance() )
	YUI::instance()->evaluatePostponeShortcutCheck();

    return YCPVoid();
}

static YCPValue
UICheckShortcuts()
{
    if ( YUI::instance() )
	YUI::instance()->evaluateCheckShortcuts();

    return YCPVoid();
}


static YCPValue
UIWidgetExists( const YCPSymbol & widget_id )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIWidgetExists1( const YCPTerm & widget_id )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIRunPkgSelection( const YCPValue & widget_id )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateRunPkgSelection( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingDirectory( const YCPString & startDir, const YCPString & headline )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateAskForExistingDirectory( startDir, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingFile( const YCPString & startWith, const YCPString & filter, const YCPString & headline  )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateAskForExistingFile( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForSaveFileName( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateAskForSaveFileName( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UISetFunctionKeys( const YCPMap & new_fkeys )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetFunctionKeys( new_fkeys );

    return YCPVoid();
}


static YCPValue
UIRecode( const YCPString & from, const YCPString & to, const YCPString & text )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateRecode( from, to, text );
    else
	return YCPVoid();
}


static YCPValue
UISetModulename( const YCPString & name )
{
    if ( YUI::instance() )
	YUI::instance()->evaluateSetModulename( name );

    return YCPVoid();
}


static YCPValue
UIHasSpecialWidget( const YCPSymbol & widget )
{
    if ( YUI::instance() )
	return YUI::instance()->evaluateHasSpecialWidget( widget );
    else
	return YCPBoolean( false );
}


static YCPValue
UICallHandler( void * ptr, int argc, YCPValue argv[] )
{
    if ( YUI::instance() )
    {
	y2debug( "Calling builtin" );
	return YUI::instance()->callBuiltin( ptr, argc, argv );
    }
    else
    {
	// FIXME: this is an error in fact
	y2warning( "No UI instance available yet!" );
	return YCPVoid();
    }
}



UI::UI()
{
    // Declarations for UI builtins and mapping to the respective C++ function.
    // NOTE: This must be static, registerDeclarations saves a pointer to it!
    
    static declaration_t ui_builtins[] =
	{
	    { "UI",			"",			(void *) &UICallHandler, DECL_NAMESPACE | DECL_CALL_HANDLER 	},
	    { "OpenDialog",		"void (term)",						(void*) UIOpenDialog 		},
	    { "OpenDialog",		"void (term, term)",					(void*) UIOpenDialog2 		},
	    { "CloseDialog",		"boolean ()",						(void*) UICloseDialog 		},
	    { "UserInput",		"any ()",						(void*) UIUserInput 		},
	    { "TimeoutUserInput",	"any (integer)",					(void*) UITimeoutUserInput 	},
	    { "WaitForEvent",		"map<any,any> (integer)",				(void*) UIWaitForEvent1 	},
	    { "WaitForEvent",		"map<any,any> ()",					(void*) UIWaitForEvent 		},
	    { "PollInput",		"any ()",						(void*) UIPollInput 		},
	    { "SetLanguage",		"void (string)",					(void*) UISetLanguage 		},
	    { "SetLanguage",		"void (string, string)",				(void*) UISetLanguage2 		},
	    { "GetLanguage",		"string (boolean)",					(void*) UIGetLanguage 		},
	    { "GetProductName",		"string ()",						(void*) UIGetProductName	},
	    { "SetProductName",		"void (string)",					(void*) UISetProductName 	},
	    { "SetConsoleFont",		"void (string, string, string, string, string)",	(void*) UISetConsoleFont 	},
	    { "SetKeyboard",		"void ()",						(void*) UISetKeyboard 		},
	    { "BusyCursor",		"void ()",						(void*) UIBusyCursor 		},
	    { "NormalCursor",		"void ()",						(void*) UINormalCursor 		},
	    { "RedrawScreen",		"void ()",						(void*) UIRedrawScreen 		},
	    { "RecalcLayout",		"void ()",						(void*) UIRecalcLayout 		},
	    { "DumpWidgetTree",		"void ()",						(void*) UIDumpWidgetTree 	},
	    { "PostponeShortcutCheck",	"void ()",						(void*) UIPostponeShortcutCheck },
	    { "CheckShortcuts",		"void ()",						(void*) UICheckShortcuts 	},
	    { "MakeScreenShot",		"void ()",						(void*) UIMakeScreenshot 	},
	    { "MakeScreenShot",		"void (string)",					(void*) UIMakeScreenshot1 	},
	    { "RecordMacro",		"void (string)",					(void*) UIRecordMacro 		},
	    { "PlayMacro",		"void (string)",					(void*) UIPlayMacro 		},
	    { "StopRecordMacro",	"void ()",						(void*) UIStopRecordMacro 	},
	    { "FakeUserInput",		"void (any)",						(void*) UIFakeUserInput 	},
	    { "SetFunctionKeys",	"void (map <any, any>)",				(void*) UISetFunctionKeys 	},
	    { "ChangeWidget",		"boolean (symbol, symbol, any)",			(void*) UIChangeWidget 		},
	    { "ChangeWidget",		"boolean (term, symbol, any)",				(void*) UIChangeWidget1 	},
	    { "ChangeWidget",		"boolean (term, term, any)",				(void*) UIChangeWidget1 	},
	    { "QueryWidget",		"any (symbol, symbol)",					(void*) UIQueryWidget 		},
	    { "QueryWidget",		"any (symbol, term)",					(void*) UIQueryWidget1 		},
	    { "QueryWidget",		"any (term, symbol)",					(void*) UIQueryWidget2 		},
	    { "QueryWidget",		"any (term, term)",					(void*) UIQueryWidget3 		},
	    { "ReplaceWidget",		"boolean (symbol, term)",				(void*) UIReplaceWidget 	},
	    { "ReplaceWidget",		"boolean (term, term)",					(void*) UIReplaceWidget1 	},
	    { "SetFocus",		"boolean (symbol)",					(void*) UISetFocus 		},
	    { "SetFocus",		"boolean (term)",					(void*) UISetFocus1 		},
	    { "WidgetExists"	,	"boolean (symbol)",					(void*) UIWidgetExists 		},
	    { "WidgetExists",		"boolean (term)",					(void*) UIWidgetExists1 	},
	    { "Glyph",			"string (symbol)",					(void*) UIGlyph 		},
	    { "GetDisplayInfo",		"map <any, any> ()",					(void*) UIGetDisplayInfo 	},
	    { "RunPkgSelection",	"any (term)",						(void*) UIRunPkgSelection 	},
	    { "AskForExistingDirectory","string (string, string)",				(void*) UIAskForExistingDirectory },
	    { "AskForExistingFile",	"string (string, string, string)",			(void*) UIAskForExistingFile 	},
	    { "AskForSaveFileName",	"string (string, string, string)",			(void*) UIAskForSaveFileName 	},
	    { "Recode",			"any (string, string, string)",				(void*) UIRecode 		},
	    { "SetModulename",		"void (string)",					(void*) UISetModulename 	},
	    { "HasSpecialWidget",	"boolean (symbol)",					(void*) UIHasSpecialWidget 	},
	    { 0, 0, 0, 0, 0, 0, 0 }
	};

    extern StaticDeclaration static_declarations;
    static_declarations.registerDeclarations ("UI", ui_builtins );
}


