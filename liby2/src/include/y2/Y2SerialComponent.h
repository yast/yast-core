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

   File:       Y2SerialComponent.h

   Author:     Thomas Roelz <tom@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/

/*
 * Component that communicates via serial line
 */

#ifndef Y2SerialComponent_h
#define Y2SerialComponent_h

#include "Y2Component.h"
#include <ycp/Parser.h>

/**
 * @short Interface to a component via serial line
 */
class Y2SerialComponent : public Y2Component
{
   /**
    * The name of the device to use
    */
   string device_name;
   
   /**
    * The baud rate to use
    */
   long baud_rate;
   
   /**
    * file descriptor for serial connection
    */
   int fd_serial;

   /**
    * Full name of component
    */
   string full_name;
   
   /**
    * Parser used to parse input
    */
   Parser parser;
   

    /**
     * After so many seconds cancel to try establishing
     * the connection and to an exit(10);
     */
    int timeout_seconds;
    
public:

   /**
    * Creates a new serial component.
    */
   Y2SerialComponent(string device_name, long baud_rate);
   
   /**
    * Cleans up
    */
   ~Y2SerialComponent();

    /**
     * Returns the name of the component.
     */
    string name() const;

   /**
    * Defined only in the server role
    */
   YCPValue evaluate(const YCPValue& command);

   /**
    * Defined only in the server role
    */ 
   void result(const YCPValue& result);

   /**
    * Sets the commandline options of the server. Server options
    * for the cat server are simply ignored.
    *
    * This method is only defined, if the component is a server.
    */
   void setServerOptions(int argc, char **argv);

   /**
    * Here the client does its actual work.
    * @param arglist YCPList of client arguments.
    * @param user_interface Option display server (user interface)
    * @return The result value (exit code) of the called client. The
    * result code has </i>not<i> yet been sent to the display server.
    * Destroy it after use.
    *
    * This method is only defined, if the component is a client.
    */
   YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);

private:

   /**
    * Open the given tty and return the corresponding file descriptor on success.
    */
   int open_tty();

   /**
    * close serial line if necessary and reset flag
    */
   void close_tty();
   
   /**
    * Setup serial device
    */
   int setup_serial_device();

   /**
    * Set raw mode 8,N,1, no parity for serial line
    */
   int make_raw();

   /**
    * Set line speed for serial line
    */
   int set_fixed_line_speed(long speed);

   /**
    * wait with timeout (microseconds) for readability
    */
   bool await_readable(long timeout);

   /**
    * initializes the serial connection
    */
   bool initializeConnection();

   /**
    * Send a YCPValue over the serial line
    */
   void sendToSerial(const YCPValue& v);  
	 

   /**
    * Reads one YCP value from the serial line. Return 0 if none could
    * be read.
    */
   YCPValue receiveFromSerial();
};

#endif   // Y2SerialComponent_h
