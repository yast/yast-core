/**
*
*  LosetupAgent.cc
*
*  Purpose:	setup a crypted loop device
*		 non interactive "losetup"
*  Creator:	mike@suse.de
*  Maintainer:	mike@suse.de
*
*  $Id$
**/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "LosetupAgent.h"
#include "LosetupIO.h"
#include <ycp/y2log.h>


LosetupAgent::LosetupAgent()
{
}


YCPValue
LosetupAgent::Read (const YCPPath& path, const YCPValue& arg)
{
    y2internal( "Sorry. No reading of losetups implemented yet");
    return YCPVoid();
}


YCPValue
LosetupAgent::Write (const YCPPath& path, const YCPValue& value,
		     const YCPValue& arg)
{
    if (!value->isMap())
	return YCPBoolean (false);

    YCPMap arg_map = value->asMap();
    int    dummy1  = 0;
    int    dummy2  = 0;

    YCPValue ycp_encryption    = arg_map->value(YCPString("encryption"));
    YCPValue ycp_loop_dev      = arg_map->value(YCPString("loop_dev"));
    YCPValue ycp_partitionName = arg_map->value(YCPString("partitionName"));
    YCPValue ycp_passwd        = arg_map->value(YCPString("passwd"));

     y2milestone("Arguments for LosetupAgent p.%s l.%s  e.%s p.%s <%s> <%s> <%s> ",
     		   ycp_partitionName->isString() ? "true":"false",
     		   ycp_loop_dev->isString() ? "true":"false",
     		   ycp_encryption->isString() ? "true":"false",
     		   ycp_passwd->isString() ? "true":"false",
     		   ycp_partitionName->toString().c_str(),
     		   ycp_loop_dev->toString().c_str(),
     		   ycp_encryption->toString().c_str() );

    char *xloop = strdup(ycp_loop_dev->toString().c_str());
    char *xpart = strdup(ycp_partitionName->toString().c_str());
    char *xencr = strdup(ycp_encryption->toString().c_str());
    char *xpass = strdup(ycp_passwd->toString().c_str());

    if ( xloop
	 && xpart
	 && xencr
	 && xpass
	 && ycp_partitionName->isString()
	 && ycp_loop_dev->isString()
	 && ycp_encryption->isString()
	 && ycp_passwd->isString() )
    {
       // remove trailing "   <"/dev/hda1"> -> </dev/hda1>
       xloop++;
       xpart++;
       xencr++;
       xpass++;

       xloop[strlen(xloop)-1]=0;
       xpart[strlen(xpart)-1]=0;
       xencr[strlen(xencr)-1]=0;
       xpass[strlen(xpass)-1]=0;

       ///////////////////////////////////////////////////////////////////////////
       int ret_code =  xset_loop (xloop, xpart, dummy1, xencr, &dummy2, xpass );
       ///////////////////////////////////////////////////////////////////////////

	return( YCPBoolean(ret_code == 0) );
    }
    else
    {
       if ( !xloop || !xpart || !xpass || !xencr )
       {
	  y2error("No memory: LosetupAgent %s", value->toString().c_str() );
       }
       else
       {
	  y2error("Wrong arguments for LosetupAgent %s", value->toString().c_str() );
       }

       return( YCPBoolean(false));
    }
}


YCPValue LosetupAgent::Dir(const YCPPath& path)
{
    return YCPList();
}


YCPValue Y2LosetupComponent::evaluate(const YCPValue& value)
{
    if (!interpreter) {
	agent = new LosetupAgent();
	interpreter = new SCRInterpreter(agent);
    }

    return interpreter->evaluate(value);
}


Y2Component *Y2CCLosetup::create(const char *name) const
{
    if (!strcmp(name, "ag_losetup")) return new Y2LosetupComponent();
    else return 0;
}

Y2CCLosetup g_y2ccag_losetup;

