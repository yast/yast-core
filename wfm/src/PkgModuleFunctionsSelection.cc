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
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;

// ------------------------
/**   
   @builtin Pkg::GetSelections (symbol status, string category) -> ["sel1", "sel2", ...]

   returns a list of selection names matching the status symbol
     and the category.
   If category == "base", base selections are returned
   If category == "", addon selections are returned
   else selections matching the given category are returned

   status can be:<br>
   `all		: all known selections<br>
   `available	: available selections<br>
   `selected	: selected but not yet installed selections<br>
   `installed	: installed selection<br>
   
*/
YCPValue
PkgModuleFunctions::GetSelections (YCPList args)
{
    if ((args->size() != 2)
	|| !(args->value(0)->isSymbol())
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad args to Pkg::GetSelections");
    }

    string status = args->value(0)->asSymbol()->symbol();
    string category = args->value(1)->asString()->value();

    YCPList selections;

    for (PMManager::PMSelectableVec::const_iterator it = Y2PM::selectionManager().begin();
	 it != Y2PM::selectionManager().end();
	 ++it)
    {
	PMSelectionPtr selection;
	if (status == "all")
	{
	    selection = (*it)->theObject();
	}
	else if (status == "available")
	{
	    selection = (*it)->candidateObj();
	}
	else if (status == "selected")
	{
	    if ((*it)->status() == PMSelectable::S_Install)
		selection = (*it)->candidateObj();
	}
	else if (status == "installed")
	{
	    selection = (*it)->installedObj();
	}
	else
	{
	    y2warning (string ("Unknown status in Pkg::GetSelections("+status+", ...)").c_str());
	    break;
	}

	if (!selection)
	{
	    continue;
	}

	if (category == "base")
	{
	    if (!selection->isBase())
		continue;			// asked for base, not base
	}
	else if (category != "")
	{
	    if (selection->category() != category)
	    {
		continue;			// asked for explicit category
	    }
	}
	else	// category == ""
	{
	    if (selection->isBase())
	    {
		continue;			// asked for non-base
	    }
	}
	selections->add (YCPString (selection->name()));
    }
    return selections;
}


// ------------------------
/**   
   @builtin Pkg::SelectionData (string selection)
  	->	$["summary" : "This is a nice selection",
  		"category" : "Network",
  		"visible" : true,
  		"suggests" : ["sel1", "sel2", ...],
  		"archivesize" : 12345678
  		"order" : "042"]

   Get summary (aka label), category, visible, suggests, archivesize,
  	and order attributes of a selection
   Returns an empty list if no selection found
   Returns nil if called with wrong arguments

*/
YCPValue
PkgModuleFunctions::SelectionData (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SelSummary");
    }
    YCPMap data;
    string name = args->value(0)->asString()->value();
    y2milestone ("looking up (%s)", name.c_str());
    PMSelectablePtr selectable = _y2pm.selectionManager().getItem(name);
    if (!selectable)
    {
	return YCPError ("Selection '"+name+"' not found", data);
    }
    PMSelectionPtr selection = selectable->theObject();
    if (!selection)
    {
	return YCPError ("Selection '"+name+"' no object", data);
    }
    y2milestone ("startRetrieval for (%s)", name.c_str());
    selection->startRetrieval();
    y2milestone ("PkgModuleFunctions::SelectionData(%s)", name.c_str());
    data->add (YCPString ("summary"), YCPString (selection->summary("")));
    data->add (YCPString ("category"), YCPString (selection->category()));
    data->add (YCPString ("visible"), YCPBoolean (selection->visible()));
    std::list<std::string> suggests = selection->suggests();
    y2milestone ("  with (%d) suggestions", suggests.size());
    YCPList suggestslist;
    for (std::list<std::string>::iterator sugIt = suggests.begin();
	sugIt != suggests.end(); ++sugIt)
    {
	if (!((*sugIt).empty()))
	    suggestslist->add (YCPString (*sugIt));
    }
    data->add (YCPString ("suggests"), suggestslist);
    data->add (YCPString ("archivesize"), YCPInteger ((long long) (selection->archivesize())));
    data->add (YCPString ("order"), YCPString (selection->order()));
    selection->stopRetrieval();
    y2milestone (" data : %s", data->toString().c_str());
    return data;
}

// ------------------------
/**   
   @builtin Pkg::SetSelection (string selection) -> bool

   Set a new selection

   If the selection is a base selection,
   this effetively resets the current package selection to
   the packages of the newly selected base selection
   Usually returns true
   Returns false if the given string does not match
   a known selection.

*/
YCPValue
PkgModuleFunctions::SetSelection (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetSelection");
    }
    string name = args->value(0)->asString()->value();
    PMSelectablePtr selectable = _y2pm.selectionManager().getItem(name); 
    if (selectable)
    {
	return YCPBoolean (selectable->set_status(PMSelectable::S_Install, true));
    }
    return YCPError ("Selectable not available", YCPBoolean (false));
}

// ------------------------
/**   
   @builtin Pkg::ClearSelection (string selection) -> bool

   Clear a selected selection


*/
YCPValue
PkgModuleFunctions::ClearSelection (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetSelection");
    }
    string name = args->value(0)->asString()->value();
    PMSelectablePtr selectable = _y2pm.selectionManager().getItem(name);
    if (selectable)
    {
	bool ret = true;
	if (selectable->status() == PMSelectable::S_Install)
	    ret = selectable->set_status (PMSelectable::S_NoInst, true);
	else if (selectable->status() == PMSelectable::S_Update)
	    ret = selectable->set_status (PMSelectable::S_KeepInstalled, true);
	return YCPBoolean (ret);
    }
    return YCPError ("No selectable found", YCPBoolean (false));
}

