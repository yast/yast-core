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

   File:	UI.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>

   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "UI.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

#include "Y2UIComponent.h"


static YCPValue
UISetLanguage( const YCPString & language )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetLanguage( language );

    return YCPVoid();
}


static YCPValue
UISetLanguage2( const YCPString & language, const YCPString & encoding )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetLanguage( language, encoding );

    return YCPVoid();
}


static YCPValue
UIGetProductName()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateGetProductName();
    else
	return YCPVoid();
}


static YCPValue
UISetProductName( const YCPString & name )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetProductName( name );

    return YCPVoid();
}


static YCPValue
UISetConsoleFont( const YCPString & console_magic,
		  const YCPString & font,
		  const YCPString & screen_map,
		  const YCPString & unicode_map,
		  const YCPString & encoding )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetConsoleFont( console_magic,
							   font,
							   screen_map,
							   unicode_map,
							   encoding );
    return YCPVoid();
}


static YCPValue
UISetKeyboard()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetKeyboard();

    return YCPVoid();
}


static YCPValue
UIGetLanguage( const YCPBoolean & strip )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateGetLanguage( strip );
    else
	return YCPVoid();
}


static YCPValue
UIUserInput()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateUserInput();
    else
	return YCPVoid();
}


static YCPValue
UIPollInput()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluatePollInput();
    else
	return YCPVoid();
}


static YCPValue
UITimeoutUserInput( const YCPInteger& timeout )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateTimeoutUserInput( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateWaitForEvent();
    else
	return YCPVoid();
}


static YCPValue
UIWaitForEvent1( const YCPInteger& timeout )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateWaitForEvent( timeout );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog2( const YCPTerm & opts, const YCPTerm & dialog_term )
{
    if ( Y2UIComponent::instance() )
	// Notice: Parameter order is switched!
	return Y2UIComponent::instance()->evaluateOpenDialog( dialog_term, opts );
    else
	return YCPVoid();
}


static YCPValue
UIOpenDialog( const YCPTerm & dialog_term )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateOpenDialog( dialog_term );
    else
	return YCPVoid();
}


static YCPValue
UICloseDialog()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateCloseDialog();
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget( const YCPSymbol & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIChangeWidget1( const YCPTerm & widget_id, const YCPValue & property, const YCPValue & new_value )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateChangeWidget( widget_id, property, new_value );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget( const YCPSymbol & widget_id, const YCPSymbol & property )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget1( const YCPSymbol & widget_id, const YCPTerm & property )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget2( const YCPTerm & widget_id, const YCPSymbol & property )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIQueryWidget3( const YCPTerm & widget_id, const YCPTerm & property )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateQueryWidget( widget_id, property );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget( const YCPSymbol & widget_id, const YCPTerm & new_widget )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UIReplaceWidget1( const YCPTerm & widget_id, const YCPTerm & new_widget )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateReplaceWidget( widget_id, new_widget );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus( const YCPSymbol & widget_id )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UISetFocus1( const YCPTerm & widget_id )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateSetFocus( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIBusyCursor()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateBusyCursor();

    return YCPVoid();
}


static YCPValue
UIRedrawScreen()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateRedrawScreen();

    return YCPVoid();
}


static YCPValue
UINormalCursor()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateNormalCursor();

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot1( const YCPString & filename )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateMakeScreenShot( filename );

    return YCPVoid();
}


static YCPValue
UIMakeScreenshot()
{
    if ( Y2UIComponent::instance() )
	return UIMakeScreenshot1( YCPNull() );
    else
	return YCPVoid();
}


static YCPValue
UIDumpWidgetTree()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateDumpWidgetTree();

    return YCPVoid();
}


static YCPValue
UIRecordMacro( const YCPString & filename )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateRecordMacro( filename );

    return YCPVoid();
}


static YCPValue
UIStopRecordMacro()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateStopRecordMacro();

    return YCPVoid();
}


static YCPValue
UIPlayMacro( const YCPString & filename )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluatePlayMacro( filename );

    return YCPVoid();
}


static YCPValue
UIFakeUserInput( const YCPValue & next_input )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateFakeUserInput( next_input );

    return YCPVoid();
}


static YCPValue
UIGlyph( const YCPSymbol & glyphSym  )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateGlyph( glyphSym );
    else
	return YCPString( "*" );
}


static YCPValue
UIGetDisplayInfo()
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateGetDisplayInfo();
    else
	return YCPVoid();
}


static YCPValue
UIRecalcLayout()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateRecalcLayout();

    return YCPVoid();
}


static YCPValue
UIPostponeShortcutCheck()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluatePostponeShortcutCheck();

    return YCPVoid();
}

static YCPValue
UICheckShortcuts()
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateCheckShortcuts();

    return YCPVoid();
}


static YCPValue
UIWidgetExists( const YCPSymbol & widget_id )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIWidgetExists1( const YCPTerm & widget_id )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateWidgetExists( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIRunPkgSelection( const YCPValue & widget_id )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateRunPkgSelection( widget_id );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingDirectory( const YCPString & startDir, const YCPString & headline )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateAskForExistingDirectory( startDir, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForExistingFile( const YCPString & startWith, const YCPString & filter, const YCPString & headline  )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateAskForExistingFile( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UIAskForSaveFileName( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateAskForSaveFileName( startWith, filter, headline );
    else
	return YCPVoid();
}


static YCPValue
UISetFunctionKeys( const YCPMap & new_fkeys )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetFunctionKeys( new_fkeys );

    return YCPVoid();
}


static YCPValue
UIRecode( const YCPString & from, const YCPString & to, const YCPString & text )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateRecode( from, to, text );
    else
	return YCPVoid();
}


static YCPValue
UISetModulename( const YCPString & name )
{
    if ( Y2UIComponent::instance() )
	Y2UIComponent::instance()->evaluateSetModulename( name );

    return YCPVoid();
}


static YCPValue
UIHasSpecialWidget( const YCPSymbol & widget )
{
    if ( Y2UIComponent::instance() )
	return Y2UIComponent::instance()->evaluateHasSpecialWidget( widget );
    else
	return YCPBoolean( false );
}



UI::UI()
{
    // Declarations for UI builtins and mapping to the respective C++ function.
    // NOTE: This must be static, registerDeclarations saves a pointer to it!
    
    static declaration_t ui_builtins[] =
	{
	    { "UI",			"",				NULL, 			DECL_NAMESPACE 			},
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


