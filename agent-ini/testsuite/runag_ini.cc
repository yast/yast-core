/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       runag_rcconfig.cc

   Author:     Arvin Schnell <arvin@suse.de>
	       Petr Blahos   <pblahos@suse.cz>
   Maintainer: Petr Blahos   <pblahos@suse.cz>

/-*/

#include <stdio.h>
#include <unistd.h>
#include <utime.h>

#include <ycp/y2log.h>
#include <ycp/YCPParser.h>
#include <y2/Y2StdioComponent.h>
#include <scr/SCRInterpreter.h>
#include <scr/SCRAgent.h>

#include "../src/IniAgent.h"


extern int yydebug;

string in_filename;
int utime_set = 0;

class IInterpreter : public SCRInterpreter 
{
public:
    IInterpreter (SCRAgent *a) : SCRInterpreter (a) {};
protected:
    YCPValue evaluateInstantiatedTerm (const YCPTerm& term)
    {
	string sym = term->symbol()->symbol();
	if (sym == "INI")
	{
	    int term_size = term->size();
	    if (1 != term_size && 2 != term_size)
		return YCPNull ();
	    if (!term->value(0)->isString ())
		return YCPNull ();
	    if (2 == term_size && !term->value(1)->isString ())
		return YCPNull ();
	    string filename = (term_size == 1 ? in_filename.c_str() : term->value(1)->asString()->value_cstr());
	    FILE*f = fopen (filename.c_str(), "a");
	    if (NULL != f)
	    {
		struct utimbuf times;
		y2error ("Added at the end of %s: %s", filename.c_str(), term->value(0)->asString()->value().c_str());
		fprintf (f, "%s\n", term->value(0)->asString()->value().c_str());
		fclose (f);
		times.actime = 0;
		times.modtime = utime_set++;
		utime (filename.c_str(),&times);
	    }
	    else
		y2error ("Can not open file %s for appending.", filename.c_str());
	    return YCPVoid ();
	}
	return SCRInterpreter::evaluateInstantiatedTerm (term);
    }
};


int
main (int argc, char *argv[])
{
    const char *fname = 0;
    FILE *infile = stdin;

    if (argc > 1) {
	int argp = 1;
	while (argp < argc) {
	    if ((argv[argp][0] == '-')
		&& (argv[argp][1] == 'l')
		&& (argp + 1 < argc)) {
		argp++;
		set_log_filename (argv[argp]);
	    } else if (fname == 0) {
		fname = argv[argp];
	    } else {
		fprintf (stderr, "Bad argument '%s'\nUsage: runag_ini "
			 "[name.ycp]\n", argv[argp]);
	    }
	    argp++;
	}
    }

    // create parser
    YCPParser *parser = new YCPParser ();
    if (!parser) {
	fprintf (stderr, "Failed to create YCPParser\n");
	exit (EXIT_FAILURE);
    }

    // create stdio as UI component, disable textdomain calls
    Y2Component *user_interface = new Y2StdioComponent (false, true);
    if (!user_interface) {
	fprintf (stderr, "Failed to create Y2StdioComponent\n");
	exit (EXIT_FAILURE);
    }

    // create IniAgent
    SCRAgent *agent = new IniAgent ();
    if (!agent) {
	fprintf (stderr, "Failed to create IniAgent\n");
	exit (EXIT_FAILURE);
    }

    // create interpreter
    IInterpreter *interpreter = new IInterpreter (agent);
    if (!interpreter) {
	fprintf (stderr, "Failed to create IInterpreter\n");
	exit (EXIT_FAILURE);
    }

    // load config file (if existing)
    if (fname) {
	int len = strlen (fname);
	if (len > 5 && strcmp (&fname[len-4], ".ycp") == 0) {
	    char *cname = strdup (fname);
	    cname[len-4] = 0;
	    in_filename = cname;
	    in_filename+= ".in.test";
	    strcpy (&cname[len-4], ".scr");
	    if (access (cname, R_OK) == 0) {
		YCPValue confval = SCRAgent::readconf (cname);
                if (confval.isNull () || !confval->isTerm ()) {
                    fprintf (stderr, "Failed to read '%s'\n", cname);
                    exit (EXIT_FAILURE);
                }
                YCPTerm term = confval->asTerm();
                for (int i = 0; i < term->size (); i++)
                    interpreter->evaluate (term->value (i));
	    }
	}
    }

    // open ycp script
    if (fname != 0) {
	infile = fopen (fname, "r");
	if (infile == 0) {
	    fprintf (stderr, "Failed to open '%s'\n", fname);
	    exit (EXIT_FAILURE);
	}
    } else {
	fname = "stdin";
    }

    // evaluate ycp script
    parser->setInput (infile, fname);
    parser->setBuffered ();
    YCPValue value = YCPVoid ();
    while (true) {
	value = parser->parse ();
	if (value.isNull ())
	    break;
	value = interpreter->evaluate (value);
	printf ("(%s)\n", value->toString ().c_str ());
    }

    if (infile != stdin)
	fclose (infile);

    delete interpreter;
    delete agent;
    delete user_interface;
    delete parser;

    exit (EXIT_SUCCESS);
}
