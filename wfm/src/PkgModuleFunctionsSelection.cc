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

   File:	PkgModuleFunctionsSelection.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to PMSelectionManager
		Handles selection related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstData.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;

// ------------------------
// 
// @builtin Pkg::GetSelections (symbol which) -> ["sel1", "sel2", ...]
//
// returns a list of selection names matching the 'which' symbol
// which can be:<br>
// `all		: all known selections<br>
// `avail_base	: available base selections<br>
// `avail_addon	: available addon selections<br>
// `inst_base	: installed base selection (usually one !)<br>
// `inst_addon	: installed addon selections<br>
// 

YCPValue
PkgModuleFunctions::GetSelections (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isSymbol()))
    {
	return YCPError ("Bad args to Pkg::GetSelections");
    }

    string which = args->value(0)->asSymbol()->symbol();
    YCPList selections;

    if (which == "all")
    {
        selections->add (YCPString ("Standard"));
        selections->add (YCPString ("Minimal"));
        selections->add (YCPString ("Development"));
        selections->add (YCPString ("Games"));
        selections->add (YCPString ("Multimedia"));
    }
    else if (which == "avail_base")
    {
        selections->add (YCPString ("Standard"));
        selections->add (YCPString ("Minimal"));
    }
    else if (which == "inst_base")
    {
        selections->add (YCPString ("Standard"));
    }
    else if (which == "avail_addon")
    {
        selections->add (YCPString ("Development"));
        selections->add (YCPString ("Games"));
        selections->add (YCPString ("Multimedia"));
    }
    else if (which == "inst_addon")
    {
        selections->add (YCPString ("Development"));
        selections->add (YCPString ("Multimedia"));
    }
    return selections;
}


// ------------------------
// 
// @builtin Pkg::SelSummary (string selection) -> "This is a nice selection"
//
// Get summary (aka label) of a selection
//
YCPValue
PkgModuleFunctions::SelSummary (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))

    {
	return YCPError ("Bad args to Pkg::SelSummary");
    }

    return YCPString ("This is a nice selection");
}

// ------------------------
// 
// @builtin Pkg::SetSelection (string selection) -> bool
//
// Set a new base selection
// This effetively resets the current package selection to
// the packages of the newly selected base selection
// Usually returns true
// Returns false if the given string does not match
// a base selection.
//
YCPValue
PkgModuleFunctions::SetSelection (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetSelection");
    }

    return YCPBoolean (true);
}

// ------------------------
// 
// @builtin Pkg::IsManualSelection () -> bool
//
// return true if the original list of packages (since the
// last Pkg::SetSelection was changed.
//
YCPValue
PkgModuleFunctions::IsManualSelection (YCPList args)
{

    return YCPBoolean (false);
}

