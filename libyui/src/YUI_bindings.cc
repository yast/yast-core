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
#include "YUIComponent.h"


static YCPValue
UISetLanguage( const YCPString & language )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetLanguage( language );

    return YCPVoid();
}


static YCPValue
UISetLanguage2( const YCPString & language, const YCPString & encoding )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetLanguage( language, encoding );

    return YCPVoid();
}


static YCPValue
UIGetProductName()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateGetProductName();
    else
	return YCPVoid();
}


static YCPValue
UISetProductName( const YCPString & name )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetProductName( name );

    return YCPVoid();
}


static YCPValue
UISetConsoleFont( const YCPString & console_magic,
		  const YCPString & font,
		  const YCPString & screen_map,
		  const YCPString & unicode_map,
		  const YCPString & encoding )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetConsoleFont( console_magic,
						 font,
						 screen_map,
						 unicode_map,
						 encoding );
    return YCPVoid();
}


static YCPValue
UISetKeyboard()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetKeyboard();

    return YCPVoid();
}


static YCPValue
UIGetLanguage( const YCPBoolean & strip )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateGetLanguage( strip );
    else
	return YCPVoid();
}


YCPValue
UIUserInput()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateUserInput();
    else
	return YCPVoid();
}


static YCPValue
UIPollInput()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluatePollInput();
    else
	return YCPVoid();
}


static YCPValue
UITimeoutUserInput( const YCPInteger& timeout )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateTimeoutUserInput( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWaitForEvent();
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent1( const YCPInteger& timeout )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWaitForEvent( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog2( const YCPTerm & opts, const YCPTerm & dialog_term )
{
    if ( YUIComponent::ui() )
	// Notice: Parameter order is switched!
	return YUIComponent::ui()->evaluateOpenDialog( dialog_term, opts );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog( const YCPTerm & dialog_term )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateOpenDialog( dialog_term );
    else
	return YCPVoid();
}


static YCPValue
UICloseDialog()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateCloseDialog();
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget( const YCPSymbol & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget1( const YCPTerm & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget( const YCPSymbol & widget_id, const YCPSymbol & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget1( const YCPSymbol & widget_id, const YCPTerm & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget2( const YCPTerm & widget_id, const YCPSymbol & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget3( const YCPTerm & widget_id, const YCPTerm & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget( const YCPSymbol & widget_id, const YCPTerm & new_widget )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget1( const YCPTerm & widget_id, const YCPTerm & new_widget )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus( const YCPSymbol & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus1( const YCPTerm & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIBusyCursor()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateBusyCursor();

    return YCPVoid();
}


static YCPValue
UIRedrawScreen()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateRedrawScreen();

    return YCPVoid();
}


static YCPValue
UINormalCursor()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateNormalCursor();

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot1( const YCPString & filename )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateMakeScreenShot( filename );

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot()
{
    if ( YUIComponent::ui() )
	return UIMakeScreenshot1( YCPNull() );
    else
	return YCPVoid();
}


static YCPValue
UIDumpWidgetTree()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateDumpWidgetTree();

    return YCPVoid();
}


static YCPValue
UIRecordMacro( const YCPString & filename )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateRecordMacro( filename );

    return YCPVoid();
}


static YCPValue
UIStopRecordMacro()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateStopRecordMacro();

    return YCPVoid();
}


static YCPValue
UIPlayMacro( const YCPString & filename )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluatePlayMacro( filename );

    return YCPVoid();
}

static YCPValue
UIFakeUserInput1()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateFakeUserInput( YCPVoid() );

    return YCPVoid();
}


static YCPValue
UIFakeUserInput2( const YCPValue & next_input )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateFakeUserInput( next_input );

    return YCPVoid();
}



static YCPValue
UIGlyph( const YCPSymbol & glyphSym  )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateGlyph( glyphSym );
    else
	return YCPString( "*" );
}


static YCPValue
UIGetDisplayInfo()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateGetDisplayInfo();
    else
	return YCPVoid();
}


static YCPValue
UIRecalcLayout()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateRecalcLayout();

    return YCPVoid();
}


static YCPValue
UIPostponeShortcutCheck()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluatePostponeShortcutCheck();

    return YCPVoid();
}

static YCPValue
UICheckShortcuts()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateCheckShortcuts();

    return YCPVoid();
}


static YCPValue
UIWidgetExists( const YCPSymbol & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIWidgetExists1( const YCPTerm & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIRunPkgSelection( const YCPValue & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateRunPkgSelection( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingDirectory( const YCPString & startDir, const YCPString & headline )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateAskForExistingDirectory( startDir, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingFile( const YCPString & startWith, const YCPString & filter, const YCPString & headline  )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateAskForExistingFile( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForSaveFileName( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateAskForSaveFileName( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UISetFunctionKeys( const YCPMap & new_fkeys )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetFunctionKeys( new_fkeys );

    return YCPVoid();
}


static YCPValue
UIRecode( const YCPString & from, const YCPString & to, const YCPString & text )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateRecode( from, to, text );
    else
	return YCPVoid();
}


static YCPValue
UISetModulename( const YCPString & name )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateSetModulename( name );

    return YCPVoid();
}


static YCPValue
UIHasSpecialWidget( const YCPSymbol & widget )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateHasSpecialWidget( widget );
    else
	return YCPBoolean( false );
}


static YCPValue
UICallHandler( void * ptr, int argc, YCPValue argv[] )
{
    if ( YUIComponent::ui() )
    {
	return YUIComponent::ui()->callBuiltin( ptr, argc, argv );
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
	    { "FakeUserInput",		"void ()",						(void*) UIFakeUserInput1 	},
	    { "FakeUserInput",		"void (any)",						(void*) UIFakeUserInput2 	},
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


