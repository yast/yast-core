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

#include <fstream>

#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstData.h>
#include <y2pm/PMSelectionManager.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

using std::string;


/**
 * helper function, get selectable by name
 */

PMSelectablePtr
PkgModuleFunctions::getSelectionSelectable (const std::string& name)
{
    PMSelectablePtr selectable;
    if (!name.empty())
	selectable = _y2pm.selectionManager().getItem(name);
    if (!selectable)
    {
	y2error ("Selection '%s' not found", name.c_str());
    }
    return selectable;
}


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
PkgModuleFunctions::GetSelections (const YCPSymbol& stat, const YCPString& cat)
{
    string status = stat->symbol();
    string category = cat->value();

    YCPList selections;

    for (PMManager::PMSelectableVec::const_iterator it = _y2pm.selectionManager().begin();
	 it != _y2pm.selectionManager().end();
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
	    if ((*it)->to_install())
	    {
		selection = (*it)->candidateObj();
	    }
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


static void
tiny_helper_no1 (YCPMap* m, const char* k, const PMSolvable::PkgRelList_type& l)
{
    const list <string> t1 = PMSolvable::PkgRelList2StringList (l);
    YCPList t2;
    for (list <string>::const_iterator it = t1.begin (); it != t1.end (); it++)
	t2->add (YCPString (*it));
    m->add (YCPString (k), t2);
}


// ------------------------
/**
   @builtin Pkg::SelectionData (string selection) -> map
  	->	$["summary" : "This is a nice selection",
  		"category" : "Network",
  		"visible" : true,
  		"recommends" : ["sel1", "sel2", ...],
  		"suggests" : ["sel1", "sel2", ...],
  		"archivesize" : 12345678
  		"order" : "042",
		"requires" : ["a", "b"],
		"conflicts" : ["c"],
		"provides" : ["d"],
		"obsoletes" : ["e", "f"],
		]

   Get summary (aka label), category, visible, recommends, suggests, archivesize,
  	order attributes of a selection, requires, conflicts, provides and obsoletes.
   Returns an empty list if no selection found
   Returns nil if called with wrong arguments

*/

YCPValue
PkgModuleFunctions::SelectionData (const YCPString& sel)
{
    YCPMap data;
    string name = sel->value();

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

    data->add (YCPString ("summary"), YCPString (selection->summary(_y2pm.getPreferredLocale())));
    data->add (YCPString ("category"), YCPString (selection->category()));
    data->add (YCPString ("visible"), YCPBoolean (selection->visible()));

    std::list<std::string> recommends = selection->recommends();
    YCPList recommendslist;
    for (std::list<std::string>::iterator recIt = recommends.begin();
	recIt != recommends.end(); ++recIt)
    {
	if (!((*recIt).empty()))
	    recommendslist->add (YCPString (*recIt));
    }
    data->add (YCPString ("recommends"), recommendslist);

    std::list<std::string> suggests = selection->suggests();
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

    tiny_helper_no1 (&data, "requires", selection->requires ());
    tiny_helper_no1 (&data, "conflicts", selection->conflicts ());
    tiny_helper_no1 (&data, "provides", selection->provides ());
    tiny_helper_no1 (&data, "obsoletes", selection->obsoletes ());

    return data;
}


// ------------------------
/**
   @builtin Pkg::SelectionContent (string selection, boolean to_delete, string language) -> list
  	->	["aaa_base", "k_deflt", ... ]

   Get list of packages listed in a selection

   selection	= name of selection
   to_delete	= if false, return packages to be installed
		  if true, return packages to be deleted
   language	= if "" (empty), return only non-language specific packages
		  else return only packages machting the language

   Returns an empty list if no matching selection found
   Returns nil if called with wrong arguments

*/

YCPValue
PkgModuleFunctions::SelectionContent (const YCPString& sel, const YCPBoolean& to_delete, const YCPString& lang)
{
    YCPList data;
    string name = sel->value();

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

    std::list<std::string> pacnames;
    YCPList paclist;
    LangCode locale (lang->value());

    if (to_delete->value() == false)			// inspacks
    {
	pacnames = selection->inspacks (locale);
    }
    else
    {
	pacnames = selection->delpacks (locale);
    }

    for (std::list<std::string>::iterator pacIt = pacnames.begin();
	pacIt != pacnames.end(); ++pacIt)
    {
	if (!((*pacIt).empty()))
	    paclist->add (YCPString (*pacIt));
    }

    return paclist;
}


// ------------------------

// internal
// sel selection by string

bool
PkgModuleFunctions::SetSelectionString (std::string name, bool recursive)
{
    PMSelectablePtr selectable = _y2pm.selectionManager().getItem(name);
    if (selectable)
    {
	PMSelectionPtr selection = selectable->theObject();
	if (selection)
	{
	    if (!recursive
		&& selection->isBase())
	    {
		y2milestone ("Changing base selection, re-setting manager");
		_y2pm.selectionManager().setNothingSelected();
		_y2pm.packageManager().setNothingSelected();
	    }
	    else if (selectable->to_install())
	    {
		// don't recurse if already selected
		return true;
	    }
	}

	if (!selectable->user_set_install())
	{
	    y2error ("Cant select %s", name.c_str());
	    return false;
	}

	// RECURSION
	// select all recommended selections of a base selection

	if (selection->isBase())
	{
	    y2milestone ("Base ! Selecting all required and recommends");
	    const std::list<std::string> recommends = selection->recommends();
	    for (std::list<std::string>::const_iterator it = recommends.begin();
		 it != recommends.end(); ++it)
	    {
		SetSelectionString (*it, true);
	    }
	}

	PkgDep::ResultList good;
	PkgDep::ErrorResultList bad;

	if (!_y2pm.selectionManager().solveInstall(good, bad))
	{
	    std::ofstream out ("/var/log/YaST2/badselections");
	    out << bad.size() << " selections failed" << std::endl;
	    for (PkgDep::ErrorResultList::const_iterator p = bad.begin();
		 p != bad.end(); ++p )
	    {
		out << *p << std::endl;
	    }

	    y2error ("%zd selections failed", bad.size());
	    return false;
	}
	return true;
    }
    y2warning ("Unknown selection '%s'", name.c_str());
    return false;
}

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
YCPBoolean
PkgModuleFunctions::SetSelection (const YCPString& selection)
{
    string name = selection->value();

    return YCPBoolean (SetSelectionString (name));
}

// ------------------------
/**
   @builtin Pkg::ClearSelection (string selection) -> bool

   Clear a selected selection


*/
YCPValue
PkgModuleFunctions::ClearSelection (const YCPString& selection)
{
    y2internal("ClearSelection");

    string name = selection->value();
    PMSelectablePtr selectable = _y2pm.selectionManager().getItem(name);
    if (selectable)
    {
	// if base selection -> clear everything
	// only happens during install, no change of base selection during runtime
	PMSelectionPtr candidate = selectable->candidateObj();
	if (candidate
	    && candidate->isBase())
	{
	    _y2pm.selectionManager().setNothingSelected();
	    _y2pm.packageManager().setNothingSelected();
	}

	bool ret = selectable->user_unset();

	return YCPBoolean (ret);
    }
    return YCPError ("No selectable found", YCPBoolean (false));
}


// ------------------------
/**
   @builtin Pkg::ActivateSelections () -> bool

   Activate all selected selections

   To be called when user is done with selections and wants
   to continue on the package level (or finish)

   This will transfer the selection status to package status
*/
YCPBoolean
PkgModuleFunctions::ActivateSelections ()
{
    _y2pm.selectionManager().activate (_y2pm.packageManager());

    return YCPBoolean (true);
}

