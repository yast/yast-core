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

   File:       SCR.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// libscr.h

/**
 * @short external (client) view of SCR library
 *
 * SCR is based on the general workflow of YaST2
 * it knows about values and descriptions
 *
 * values are considered stored as a set, with imposed names which give
 * (one of possibly more) views on values
 *
 * To gain access, the client must identify itself for authorization
 * and logging purposes. If identification is successfull a SCRView
 * is returned.
 *
 */

#ifndef SCR_H
#define SCR_H

#include <YCP.h>
#include <ycp/Parser.h>

enum SCRMode { A_CREATE, A_READ, A_WRITE, A_APPEND };

#define SCRSEPARATOR '/'

/**
 * @short View to repository
 * A view is a start point to access values in the repository. Instead of
 * specifying the whole path to a value, views to sub-trees can be instantiated
 * and handled. This might later extended to implement local locking.
 */

class SCRView {
public:
    SCRView (const string& path);
    virtual ~SCRView ();

    /**
     * return path to root of view
     */
    const string &Root () const;

    /**
     * get access code of value at (sub-)view
     */
    const SCRMode Mode (const string& path) const;

    /**
     * get value at (sub-)view
     */
    virtual const YCPValueRep *getValue (const string& path) const;

    /**
     * set value at view path, return error
     */
    virtual int setValue (const string& path, const YCPValueRep *val);
    
protected:
    string rootPath;			// current path
    static Parser *parser;

private:
    static int refcount;
};

#endif // SCR_H
