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

   File:       SCRAgent.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/

#include <ycp/y2log.h>

#include "include/scr/SCRAgent.h"
#include "include/scr/SCR.h"

#include "ycp/Parser.h"
#include "ycp/YCode.h"

SCRAgent* SCRAgent::current_scr = 0;

YCPMap SCRAgent::unspecified_error;

SCRAgent::SCRAgent ()
    : mainscragent (0)
{
    if( current_scr == 0 ) current_scr = this;
    if (unspecified_error.size () == 0)
    {
	unspecified_error->add (YCPString ("code"),
				YCPString ("UNSPEC"));
	unspecified_error->add (YCPString ("summary"),
				YCPString ("Unspecified error"));
    }
}


SCRAgent::~SCRAgent ()
{
    if( current_scr == this ) current_scr = 0;
}

SCRAgent* SCRAgent::instance()
{
    return current_scr;
}

#if 0
YCPValue
SCRAgent::Execute (const YCPPath& path, const YCPValue& value,
		   const YCPValue& arg)
{
    return YCPNull();
}
#endif


YCPValue
SCRAgent::otherCommand (const YCPTerm&)
{
    return YCPNull();
}


YCPValue
SCRAgent::readconf (const char *filename)
{
    FILE *file = fopen (filename, "r");
    if (!file)
    {
	ycp2error ("Can't open %s for reading.", filename);
	return YCPNull ();
    }

    // find first line starting with "."
    const int size = 250;
    char line[size];
    char *fgets_result;
    do
    {
	fgets_result = fgets (line, size, file);
    }
    while ((fgets_result != 0) && (line[0] != '.'));

    Parser parser (file, filename);
    YCode* tmpvalue = parser.parse ();
    fclose (file);

    y2debug( "Parsed value (%p): %s", tmpvalue, tmpvalue != 0 ? tmpvalue->toString().c_str() : "not available" );
    if (tmpvalue == 0 || tmpvalue->kind () != YCode::yeTerm )
    {
	ycp2error ("Not a term in scr file.");
	return YCPNull();
    }

    // it is a term, generate YCPValue through its evaluation
    YCPValue ret = tmpvalue->evaluate();
    delete tmpvalue;
    return ret;
}

