/*-----------------------------------------------------------*- c++ -*-\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                    (C) SUSE LINUX AG |  
\----------------------------------------------------------------------/ 

   File:       YCPCodeCompare.h
   Author:     Martin Vidner <mvidner@suse.cz>

/-*/

#ifndef YCPCodeCompare_h
#define YCPCodeCompare_h

#include "y2/SymbolEntry.h"
#include "ycp/YCPCode.h"
#include "ycp/y2log.h"

/**
 * A function object used by the list builtin
 *  "sort (`a, `b, l, ``( a[0] < b[0] ))"
 * Passed to std::sort
 */
class YCPCodeCompare : public std::binary_function <const YCPValue &, const YCPValue &, bool>
{
private:
    SymbolEntryPtr se1;
    SymbolEntryPtr se2;
    YCPCode order;
public:
    // in fact symbol entries and ycode
    YCPCodeCompare (const YCPValue &asym1, const YCPValue &asym2,
		    const YCPCode &aorder)
	:   se1 (asym1->asEntry ()->entry ())
	  , se2 (asym2->asEntry ()->entry ())
	  , order (aorder)
	{
	}

    result_type operator () (first_argument_type a,
			     second_argument_type b)
	{
	    se1->setValue (a);
	    se2->setValue (b);
	    YCPValue ret = order->evaluate ();

	    if (ret.isNull ())
	    {
		ycp2error ("Bad sort order %s", order->toString ().c_str ());
		return false;	// ???
	    }

	    if (!ret->isBoolean ())
	    {
		ycp2error ("sort(): order %s evaluates to %s, which is not a boolean", order->toString ().c_str () 
			, ret->toString ().c_str ());
		return false;	// ???
	    }

	    return ret->asBoolean ()->value ();
	}
};

#endif
