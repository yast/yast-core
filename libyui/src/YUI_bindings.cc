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
UISetLanguageAndEncoding( const YCPString & language, const YCPString & encoding )
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


YCPValue
UITimeoutUserInput( const YCPInteger& timeout )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateTimeoutUserInput( timeout );
    else
	return YCPVoid();
}


YCPValue
UIWaitForEvent()
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWaitForEvent();
    else
	return YCPVoid();
}


YCPValue
UIWaitForEventTimeout( const YCPInteger & timeout )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWaitForEvent( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialogOpt( const YCPTerm & opts, const YCPTerm & dialog_term )
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
UIChangeWidgetOld( const YCPSymbol & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget( const YCPTerm & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidgetTerm( const YCPTerm & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    return UIChangeWidget( widget_id, property, new_value );
}


static YCPValue
UIQueryWidgetOld( const YCPSymbol & widget_id, const YCPSymbol & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidgetOldTerm( const YCPSymbol & widget_id, const YCPTerm & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget( const YCPTerm & widget_id, const YCPSymbol & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidgetTerm( const YCPTerm & widget_id, const YCPTerm & property )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidgetOld( const YCPSymbol & widget_id, const YCPTerm & new_widget )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget( const YCPTerm & widget_id, const YCPTerm & new_widget )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UISetFocusOld( const YCPSymbol & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus( const YCPTerm & widget_id )
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
UIMakeScreenshotToFile( const YCPString & filename )
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateMakeScreenShot( filename );

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot()
{
    if ( YUIComponent::ui() )
	return UIMakeScreenshotToFile( YCPNull() );
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
UIFakeUserInputNil()
{
    if ( YUIComponent::ui() )
	YUIComponent::ui()->evaluateFakeUserInput( YCPVoid() );

    return YCPVoid();
}


static YCPValue
UIFakeUserInput( const YCPValue & next_input )
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
UIWidgetExistsOld( const YCPSymbol & widget_id )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIWidgetExists( const YCPTerm & widget_id )
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
UIWizardCommand( const YCPTerm & command )
{
    if ( YUIComponent::ui() )
	return YUIComponent::ui()->evaluateWizardCommand( command );
    else
	return YCPBoolean( false );
}


static YCPValue
UICallHandler( void * ptr, int argc, YCPValue argv[] )
{
    if ( YUIComponent::uiComponent() )
    {
	return YUIComponent::uiComponent()->callBuiltin( ptr, argc, argv );
    }
    else
    {
	y2warning( "No UI instance available yet!" );
	return YCPVoid();
    }
}

bool UI::registered = false;

UI::UI()
{
    if (registered)
    {
	return;
    }

    // Declarations for UI builtins and mapping to the respective C++ function.
    // NOTE: This must be static, registerDeclarations saves a pointer to it!

    // This table is also processed to a YCP stub that makes the builtins
    // callable from other languages. To ease parsing, a type corresponding
    // to a single value must not contain whitespace
    static declaration_t ui_builtins[] =
	{
	    { "UI",			"",			(void *) &UICallHandler, DECL_NAMESPACE | DECL_CALL_HANDLER 	},
	    { "OpenDialog",		"void (term)",						(void*) UIOpenDialog 		},
	    { "OpenDialog",		"void (term, term)",					(void*) UIOpenDialogOpt		},
	    { "CloseDialog",		"boolean ()",						(void*) UICloseDialog 		},
	    { "UserInput",		"any ()",						(void*) UIUserInput 		},
	    { "TimeoutUserInput",	"any (integer)",					(void*) UITimeoutUserInput 	},
	    { "WaitForEvent",		"map<string,any> (integer)",				(void*) UIWaitForEventTimeout 	},
	    { "WaitForEvent",		"map<string,any> ()",					(void*) UIWaitForEvent 		},
	    { "PollInput",		"any ()",						(void*) UIPollInput 		},
	    { "SetLanguage",		"void (string)",					(void*) UISetLanguage 		},
	    { "SetLanguage",		"void (string, string)",				(void*) UISetLanguageAndEncoding},
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
	    { "MakeScreenShot",		"void (string)",					(void*) UIMakeScreenshotToFile 	},
	    { "RecordMacro",		"void (string)",					(void*) UIRecordMacro 		},
	    { "PlayMacro",		"void (string)",					(void*) UIPlayMacro 		},
	    { "StopRecordMacro",	"void ()",						(void*) UIStopRecordMacro 	},
	    { "FakeUserInput",		"void ()",						(void*) UIFakeUserInputNil 	},
	    { "FakeUserInput",		"void (any)",						(void*) UIFakeUserInput 	},
	    { "SetFunctionKeys",	"void (map<string,integer>)",				(void*) UISetFunctionKeys 	},
	    { "ChangeWidget",		"boolean (symbol, symbol, any)",			(void*) UIChangeWidgetOld,	DECL_NIL },
	    { "ChangeWidget",		"boolean (term, symbol, any)",				(void*) UIChangeWidget,		DECL_NIL },
	    { "ChangeWidget",		"boolean (term, term, any)",				(void*) UIChangeWidgetTerm,	DECL_NIL },
	    { "QueryWidget",		"any (symbol, symbol)",					(void*) UIQueryWidgetOld	},
	    { "QueryWidget",		"any (symbol, term)",					(void*) UIQueryWidgetOldTerm	},
	    { "QueryWidget",		"any (term, symbol)",					(void*) UIQueryWidget 		},
	    { "QueryWidget",		"any (term, term)",					(void*) UIQueryWidgetTerm	},
	    { "ReplaceWidget",		"boolean (symbol, term)",				(void*) UIReplaceWidgetOld 	},
	    { "ReplaceWidget",		"boolean (term, term)",					(void*) UIReplaceWidget 	},
	    { "SetFocus",		"boolean (symbol)",					(void*) UISetFocusOld		},
	    { "SetFocus",		"boolean (term)",					(void*) UISetFocus 		},
	    { "WidgetExists"	,	"boolean (symbol)",					(void*) UIWidgetExistsOld	},
	    { "WidgetExists",		"boolean (term)",					(void*) UIWidgetExists	 	},
	    { "Glyph",			"string (symbol)",					(void*) UIGlyph 		},
	    { "GetDisplayInfo",		"map<string,any> ()",					(void*) UIGetDisplayInfo 	},
	    { "RunPkgSelection",	"any (term)",						(void*) UIRunPkgSelection 	},
	    { "AskForExistingDirectory","string (string, string)",				(void*) UIAskForExistingDirectory },
	    { "AskForExistingFile",	"string (string, string, string)",			(void*) UIAskForExistingFile 	},
	    { "AskForSaveFileName",	"string (string, string, string)",			(void*) UIAskForSaveFileName 	},
	    { "Recode",			"any (string, string, string)",				(void*) UIRecode 		},
	    { "SetModulename",		"void (string)",					(void*) UISetModulename 	},
	    { "HasSpecialWidget",	"boolean (symbol)",					(void*) UIHasSpecialWidget 	},
	    { "WizardCommand",		"boolean (term)",					(void*) UIWizardCommand		},
	    { 0, 0, 0, 0, 0, 0, 0 }
	};

    extern StaticDeclaration static_declarations;
    static_declarations.registerDeclarations ("UI", ui_builtins );

    registered = true;
}


