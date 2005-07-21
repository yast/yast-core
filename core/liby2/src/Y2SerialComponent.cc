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

   File:       Y2SerialComponent.cc

   Author:     Thomas Roelz <tom@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * Component that communicates via serial line
 *
 * Author: Thomas Roelz <tom@suse.de>
 */

#include <stdio.h>
#include <unistd.h>        
#include <stdlib.h>
#include <sys/ioctl.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <term.h>
#include <linux/types.h>
#include <linux/serial.h>
#include <sys/time.h> // timeval
#include <time.h>     // nanosleep


#include "Y2SerialComponent.h"
#include <ycp/Parser.h>
#include <ycp/y2log.h>

#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPCode.h>

#define NUMSPACES 	16
#define TIMEOUT		1000	// microseconds


Y2SerialComponent::Y2SerialComponent(string device_name, long baud_rate)
    : device_name(device_name)
    , baud_rate(baud_rate)
    , fd_serial(-1)
    , timeout_seconds(-1)
{
   char buf[50];
   snprintf(buf, 50, "%ld", baud_rate);
   full_name = "serial(" + string(buf) + "):" + device_name;
}



Y2SerialComponent::~Y2SerialComponent()
{
}


string Y2SerialComponent::name() const
{
   return full_name;
}


YCPValue Y2SerialComponent::evaluate(const YCPValue& command)
{
   if (fd_serial < 0)   // not yet initialized
   { 
      if (! initializeConnection()) return YCPVoid();
   }

   sendToSerial(command);

   YCPValue ret = receiveFromSerial();

   if (ret.isNull())
   {
      y2error ("Couldn't get value from serial: %s", device_name.c_str());
      return YCPVoid();
   }

   return ret;
}


void Y2SerialComponent::result(const YCPValue& result)
{
   YCPTerm resultterm("result");
   resultterm->add(result);
   sendToSerial(resultterm);
   close_tty();
}


void Y2SerialComponent::setServerOptions(int argc, char **argv)
{
    for (int i=1; i<argc; i++)
    {
	if (!strcmp(argv[i], "--timeout"))
	{   
	    if (i+1 <argc) {
		i ++;
		int newtimeout_seconds = atoi(argv[i]);
		if (newtimeout_seconds >= 0)
		{
		    timeout_seconds = newtimeout_seconds;
		    y2milestone ("Setting timeout to %d seconds", timeout_seconds);
		}
		else
		{
		    y2warning ("Invalid timeout value %s. Using no timeout",
			  argv[i]);
		}
	    }
	    else
	    {
		y2warning ("--timeout option missing argument");
	    }
	}
	else
	{
	    y2warning ("Ignoring invalid option %s", argv[i]);
	}
    }
}


YCPValue Y2SerialComponent::doActualWork(const YCPList& arglist, Y2Component *user_interface)
{
   if (!initializeConnection()) return YCPVoid();
    
   // We do NOT send the arglist to our partner on the other side!
   // The YaST2 serial protocol defines, that the client is set up
   // by the remote side and is given its arguments also there.
   // The client arguments are just ignored here!

   if (arglist->size() > 0)
   {
      y2warning ("The %ld arguments are ignored. Remote side provides client arguments",
	    (long)arglist->size());
   }
   
   YCPValue value = YCPNull();

   while (!(value = receiveFromSerial()).isNull())
   {
      if (value->isTerm() 
	  && value->asTerm()->size() == 1 
	  && value->asTerm()->name ()=="result")
      {
	 close_tty();
	 return value; 
      }
      sendToSerial(user_interface->evaluate(value));
   }
   y2warning ("Communication ended prior to result() message");
   return YCPVoid();
}    


int Y2SerialComponent::open_tty()
{
    fd_serial = open(device_name.c_str(), O_RDWR | O_NOCTTY);
    
    if (fd_serial < 0)
    {
       y2error ("Couldn't open device: %s", device_name.c_str());
    }
    
    return fd_serial;   // success?
}


void Y2SerialComponent::close_tty()
{
   if (fd_serial >= 0) close(fd_serial);
   fd_serial = -1;
}


int Y2SerialComponent::setup_serial_device()
{
    struct serial_struct info;
    int ret = ioctl(fd_serial, TIOCGSERIAL, &info);   // get serial info
    
    if (ret >= 0)
    {
       info.flags |=  ASYNC_CTS_FLOW;	// set RTS/CTS flow control
       info.custom_divisor = 0;	        // clear custom divisor
       info.flags &= ~ASYNC_SPD_MASK;   // clear custom divisor

       ret = ioctl(fd_serial, TIOCSSERIAL, &info);   // set serial info

       if (ret < 0)
       {
	  y2error ("Couldn't set flow control for: %s", device_name.c_str());
       }
       return ret;
    }

    y2error ("Couldn't get serial info for: %s", device_name.c_str());

    return ret;
}


int Y2SerialComponent::make_raw()
{
    struct termios tty;
    int ret = tcgetattr(fd_serial, &tty);   // get attributes

    if (ret >= 0)
    {
       cfmakeraw(&tty);
       tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 data bits
       tty.c_cflag &= ~CSTOPB;                     // only one stop bit.
       tty.c_cflag &= ~PARENB;                     // no parenty generation / checking
       
       ret =  tcsetattr(fd_serial, 0 , &tty);      // set attributes
       
       if (ret < 0)
       {
	  y2error ("Couldn't set raw mode for: %s", device_name.c_str());
       }
       return ret;
    }

    y2error ("Couldn't get attributes for: %s", device_name.c_str());

    return ret;
}


int Y2SerialComponent::set_fixed_line_speed(long speed) // returns 0 on success
{
    static struct { int mask; long speed; } 
    a_speed[] =
    {
	{ B50,         50L }, { B75,       75L }, { B110,     110L }, { B134,     134L },
	{ B150,       150L }, { B200,     200L }, { B300,     300L }, { B600,     600L },
	{ B1200,     1200L }, { B1800,   1800L }, { B2400,   2400L }, { B4800,   4800L },
	{ B9600,     9600L }, { B19200, 19200L }, { B38400, 38400L }, { B57600, 57600L },
	{ B115200, 115200L }, { 0,         -1L }
    };

    for (int i=0; a_speed[i].speed >= 0L; i++)
    {
	if (a_speed[i].speed == speed)
	{
	    struct termios tty;
	    int ret = tcgetattr(fd_serial, &tty);   // get attributes
	    
	    if (ret >= 0)
	    {
		cfsetispeed(&tty, a_speed[i].mask);
		cfsetospeed(&tty, a_speed[i].mask);
		
		ret = tcsetattr(fd_serial, 0 , &tty);   // set attributes

		if (ret < 0)
		{
		   y2error ("Couldn't set speed to %ld baud for: %s",
			 speed, device_name.c_str());
		}
		return ret;
	    }
	    else
	    {
	       y2error ("Couldn't get attributes for: %s", device_name.c_str());
	       return ret;
	    }
	}
    }

    y2error ("No fixed standard line speed of %ld baud supported by %s.",
	  speed, device_name.c_str());
    
    y2error ("Allowed speeds are:");
    
    for (int i=0; a_speed[i].speed >= 0; i++)
       y2error ("Baud: %ld", a_speed[i].speed);
    
    return -1; // No such fixed speed
}


bool Y2SerialComponent::await_readable(long timeout)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd_serial, &set);

    struct timeval tv;
    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;
    return (select(fd_serial+1, &set, NULL, NULL, &tv) > 0);
}


bool Y2SerialComponent::initializeConnection()
{
   if (fd_serial >= 0) return true;             // already established
   
   int ret = open_tty();                        // logs for itself

   if (ret >= 0) ret = setup_serial_device();   // logs for itself

   if (ret >= 0) ret = make_raw();              // logs for itself

   if (ret >= 0) ret = set_fixed_line_speed(baud_rate);   // logs for itself

   if (ret >= 0)   // everything OK up to now
   {
       time_t begin_time = time(0);

      int spaces_read = 0;
      char space_buf[2*NUMSPACES];

      memset(space_buf, ' ', 2 * NUMSPACES);

      // to start communication we want to read NUMSPACES spaces in sequence
      while (spaces_read < NUMSPACES)
      {
	  time_t now = time(0);
	  if (timeout_seconds >= 0 && now - begin_time > timeout_seconds)
	  {
	      y2error ("Couldn't establish serial connection for %d seconds. Aborting.",
		    timeout_seconds);
	      close(fd_serial);
	      exit(13); // This exit code is used by general/YaST2. Try to find a better solution, if you want.
	  }

	 char buf[2];
      
	 // first write one space so the other side gets
	 // fullfilled its necessities
	 write(fd_serial, space_buf, 1);

	 // now wait for a space to be read
	 // (the other side has the same behaviour)
	 if (await_readable(TIMEOUT))
	 {
	    read(fd_serial, buf, 1);	            // read one byte

	    if (buf[0] == ' ') spaces_read++;       // was a space
	    else               spaces_read = 0;     // trash read --> reset
	 }
      }

      // now that we've got our NUMSPACES spaces write 2 * NUMSPACES spaces to
      // the other side to avoid syncing problems. the remote parser will kindly
      // ignore them.
      write(fd_serial, space_buf, 2 * NUMSPACES);

      // now set our parser to the serial line to start communication
      parser.setInput(fd_serial, device_name.c_str());

      return true;
   }

   close_tty();   // cleanup, connection NOT established
   
   return false;
}


void Y2SerialComponent::sendToSerial(const YCPValue& v)
{
    string s  = "(" + v->toString() + ")\n";    // store string in string variable ..
    const char *cs = s.c_str();        		// c_str is valid als long as s
    write(fd_serial, cs, strlen(cs));
}

YCPValue Y2SerialComponent::receiveFromSerial()
{
   return YCPCode (parser.parse ());  // set to the serial line in initializeConnection()
}
