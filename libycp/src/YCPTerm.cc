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

   File:       YCPTerm.cc
		YCPTerm data type

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "y2log.h"
#include "YCPTerm.h"



// YCPTermRep
YCPTermRep::YCPTermRep(const YCPSymbol& s)
    : s(s)
{
}


YCPTermRep::YCPTermRep(const YCPSymbol& s, const string& n)
    : s(s)
    , n(n)
{
}


YCPTermRep::YCPTermRep(const YCPSymbol& s, const YCPList& l)
    : s(s)
    , l(l) 
{
}


YCPTermRep::YCPTermRep(const YCPSymbol& s, const string& n, const YCPList& l)
    : s(s)
    , n(n)
    , l(l) 
{
}


YCPTermRep::YCPTermRep(string s, bool quoted)
    : s(s, quoted)
{
}


// get symbol of term
YCPSymbol YCPTermRep::symbol() const
{
    return s;
}


// get namespace of term
string YCPTermRep::name_space () const
{
    return n;
}


// get list of term
YCPList YCPTermRep::args() const
{
    return l;
}


bool YCPTermRep::isQuoted() const
{
    return s->isQuoted();
}


//compare terms
YCPOrder YCPTermRep::compare(const YCPTerm& t) const
{
    YCPOrder order = YO_EQUAL;

    int ncomp = n.compare( t->n );
    if (ncomp != 0)
    {
	order = (ncomp < 0) ? YO_LESS : YO_GREATER;
    }
    else
    {
	order = s->compare( t->s );

	if ( order == YO_EQUAL )
	{
	    order = l->compare( t->l );
	}
    }

    return order;
}


// clone term
YCPTerm YCPTermRep::shallowCopy() const
{
   return YCPTerm (s, n, l->shallowCopy());
}


// add value to term
YCPTerm YCPTermRep::functionalAdd(const YCPValue& val) const
{
   return YCPTerm (s, n, l->functionalAdd(val));
}


// get the term as string
string YCPTermRep::toString() const
{
    return (n.empty()
	    ? n
	    : ((n == GLOBALNAME)
	       ? string ("::")
	       : (n + "::")))
	   + s->toString() + " (" + l->commaList() + ")";   // comma separated list
}


// test if the term's list is empty
bool YCPTermRep::isEmpty() const
{
  return l->isEmpty();
}


// get the size of the term's list
int YCPTermRep::size() const
{
    return l->size();
}


// Reserves a number of elements in the term's list.
void YCPTermRep::reserve (int size)
{
    l->reserve (size);
}


// get the n-th element of the term's list
YCPValue YCPTermRep::value(int n) const
{
    return l->value(n);
}


// add an element to the term's list
void YCPTermRep::add(const YCPValue& value)
{
    l->add(value);
}


YCPValueType YCPTermRep::valuetype() const
{
    return YT_TERM;
}

