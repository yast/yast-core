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

  File:	      YUISymbols.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUISymbols_h
#define YUISymbols_h


// UI builtin functions

#define YUIBuiltin_BusyCursor			"BusyCursor"
#define YUIBuiltin_ChangeWidget			"ChangeWidget"
#define YUIBuiltin_CloseDialog			"CloseDialog"
#define YUIBuiltin_DumpWidgetTree		"DumpWidgetTree"
#define YUIBuiltin_GetDisplayInfo		"GetDisplayInfo"
#define YUIBuiltin_GetLanguage			"GetLanguage"
#define YUIBuiltin_GetModulename		"GetModulename"
#define YUIBuiltin_Glyph			"Glyph"
#define YUIBuiltin_HasSpecialWidget		"HasSpecialWidget"
#define YUIBuiltin_MakeScreenShot		"MakeScreenShot"
#define YUIBuiltin_NormalCursor			"NormalCursor"
#define YUIBuiltin_OpenDialog			"OpenDialog"
#define YUIBuiltin_PollInput			"PollInput"
#define YUIBuiltin_QueryWidget			"QueryWidget"
#define YUIBuiltin_RecalcLayout			"RecalcLayout"
#define YUIBuiltin_Recode			"Recode"
#define YUIBuiltin_RedrawScreen			"RedrawScreen"
#define YUIBuiltin_ReplaceWidget		"ReplaceWidget"
#define YUIBuiltin_SetConsoleFont		"SetConsoleFont"
#define YUIBuiltin_SetFocus			"SetFocus"
#define YUIBuiltin_SetLanguage			"SetLanguage"
#define YUIBuiltin_SetModulename		"SetModulename"
#define YUIBuiltin_UserInput			"UserInput"
#define YUIBuiltin_WidgetExists			"WidgetExists"

#define YUIBuiltin_RecordMacro			"RecordMacro"
#define YUIBuiltin_StopRecordMacro		"StopRecordMacro"
#define YUIBuiltin_PlayMacro			"PlayMacro"
#define YUIBuiltin_FakeUserInput		"FakeUserInput"
#define YUIBuiltin_WFM				"WFM"
#define YUIBuiltin_SCR				"SCR"



// Mandatory widgets

#define YUIWidget_Bottom			"Bottom"
#define YUIWidget_CheckBox			"CheckBox"
#define YUIWidget_ComboBox			"ComboBox"
#define YUIWidget_Empty				"Empty"
#define YUIWidget_Frame				"Frame"
#define YUIWidget_HBox				"HBox"
#define YUIWidget_HCenter			"HCenter"
#define YUIWidget_HSpacing			"HSpacing"
#define YUIWidget_HSquash			"HSquash"
#define YUIWidget_HStretch			"HStretch"
#define YUIWidget_HVCenter			"HVCenter"
#define YUIWidget_HVSquash			"HVSquash"
#define YUIWidget_HVStretch			"HVStretch"
#define YUIWidget_HWeight			"HWeight"
#define YUIWidget_Heading			"Heading"
#define YUIWidget_Image				"Image"
#define YUIWidget_IntField			"IntField"
#define YUIWidget_Label				"Label"
#define YUIWidget_Left				"Left"
#define YUIWidget_LogView			"LogView"
#define YUIWidget_MenuButton			"MenuButton"
#define YUIWidget_MultiLineEdit			"MultiLineEdit"
#define YUIWidget_MultiSelectionBox		"MultiSelectionBox"
#define YUIWidget_Password			"Password"
#define YUIWidget_ProgressBar			"ProgressBar"
#define YUIWidget_PushButton			"PushButton"
#define YUIWidget_RadioButton			"RadioButton"
#define YUIWidget_RadioButtonGroup		"RadioButtonGroup"
#define YUIWidget_ReplacePoint			"ReplacePoint"
#define YUIWidget_RichText			"RichText"
#define YUIWidget_Right				"Right"
#define YUIWidget_SelectionBox			"SelectionBox"
#define YUIWidget_Table				"Table"
#define YUIWidget_TextEntry			"TextEntry"
#define YUIWidget_Top				"Top"
#define YUIWidget_Tree				"Tree"
#define YUIWidget_VBox				"VBox"
#define YUIWidget_VCenter			"VCenter"
#define YUIWidget_VSpacing			"VSpacing"
#define YUIWidget_VSquash			"VSquash"
#define YUIWidget_VStretch			"VStretch"
#define YUIWidget_VWeight			"VWeight"


// Special (optional) widgets

#define YUISpecialWidget_BarGraph		"BarGraph"
#define YUISpecialWidget_ColoredLabel		"ColoredLabel"
#define YUISpecialWidget_DownloadProgress	"DownloadProgress"
#define YUISpecialWidget_DummySpecialWidget	"DummySpecialWidget"
#define YUISpecialWidget_PartitionSplitter	"PartitionSplitter"
#define YUISpecialWidget_Slider			"Slider"


// Widget properties

#define YUIProperty_CurrentButton		"CurrentButton"
#define YUIProperty_CurrentItem			"CurrentItem"
#define YUIProperty_Enabled			"Enabled"
#define YUIProperty_ExpectedSize		"ExpectedSize"
#define YUIProperty_Filename			"Filename"
#define YUIProperty_Item			"Item"
#define YUIProperty_Items			"Items"
#define YUIProperty_Label			"Label"
#define YUIProperty_Labels			"Labels"
#define YUIProperty_LastLine			"LastLine"
#define YUIProperty_Notify			"Notify"
#define YUIProperty_SelectedItems		"SelectedItems"
#define YUIProperty_ValidChars			"ValidChars"
#define YUIProperty_Value			"Value"
#define YUIProperty_Values			"Values"
#define YUIProperty_WindowID			"WindowID"
#define YUIProperty_EasterEgg			"EasterEgg"


// Widget and dialog options

#define YUIOpt_animated				"animated"
#define YUIOpt_autoShortcut			"autoShortcut"
#define YUIOpt_debugLayout			"debugLayout"
#define YUIOpt_decorated			"decorated"
#define YUIOpt_default				"default"
#define YUIOpt_defaultsize			"defaultsize"
#define YUIOpt_disabled				"disabled"
#define YUIOpt_editable				"editable"
#define YUIOpt_easterEgg			"easterEgg"
#define YUIOpt_hstretch				"hstretch"
#define YUIOpt_hvstretch			"hvstretch"
#define YUIOpt_immediate			"immediate"
#define YUIOpt_infocolor			"infocolor"
#define YUIOpt_keepSorting			"keepSorting"
#define YUIOpt_notify				"notify"
#define YUIOpt_outputField			"outputField"
#define YUIOpt_plainText			"plainText"
#define YUIOpt_scaleToFit			"scaleToFit"
#define YUIOpt_shrinkable			"shrinkable"
#define YUIOpt_tiled				"tiled"
#define YUIOpt_vstretch				"vstretch"
#define YUIOpt_warncolor			"warncolor"
#define YUIOpt_zeroWidth			"zeroWidth"
#define YUIOpt_zeroHeight			"zeroHeight"
#define YUIOpt_countShowDelta			"countShowDelta"


// Predefined glyphs for builtin Glyph()
//
// - remember there must be a substitute that can be displayed in plain ASCII,
// so don't include just everything here that is included in Unicode / UTF-8!

#define YUIGlyph_ArrowLeft			"ArrowLeft"
#define YUIGlyph_ArrowRight			"ArrowRight"
#define YUIGlyph_ArrowUp			"ArrowUp"
#define YUIGlyph_ArrowDown			"ArrowDown"

#define YUIGlyph_CheckMark			"CheckMark"
#define YUIGlyph_BulletArrowRight		"BulletArrowRight"
#define YUIGlyph_BulletCircle			"BulletCircle"
#define YUIGlyph_BulletSquare			"BulletSquare"



// Display capabilities for GetDisplayInfo()

#define YUICap_Width				"Width"
#define YUICap_Height				"Height"
#define YUICap_Depth				"Depth"
#define YUICap_Colors				"Colors"
#define YUICap_DefaultWidth			"DefaultWidth"
#define YUICap_DefaultHeight			"DefaultHeight"
#define YUICap_TextMode				"TextMode"
#define YUICap_HasImageSupport			"HasImageSupport"
#define YUICap_HasLocalImageSupport		"HasLocalImageSupport"
#define YUICap_HasAnimationSupport		"HasAnimationSupport"
#define YUICap_HasIconSupport			"HasIconSupport"
#define YUICap_HasFullUtf8Support		"HasFullUtf8Support"



// Misc

#define YUISymbol_id				"id"
#define YUISymbol_opt				"opt"
#define YUISymbol_item				"item"
#define YUISymbol_menu				"menu"
#define YUISymbol_header			"header"
#define YUISymbol_rgb				"rgb"


#endif // YUISymbols_h
