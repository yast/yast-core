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

   File:       YCPElement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCP Element data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPElement.h"
#include "YCPValue.h"
#include "YCPStatement.h"



// YCPElement

YCPElement::YCPElement() 
    : element(0)
{
}

YCPElement::YCPElement(const YCPNull&) 
    : element(0)
{
}

YCPElement::YCPElement(const YCPElementRep *e) 
    : element(e ? e->clone() : 0)
{
}

YCPElement::YCPElement(const YCPElement &e) 
    : element(e.element ? e.element->clone() : 0)
{
}

YCPElement::~YCPElement()
{
  if (element) element->destroy();
}

const YCPElement& YCPElement::operator=(const YCPElement& e)
{
  if (this != &e) {
    if (element) element->destroy();
    element = e.element ? e.element->clone() : 0;
  }
  return *this;
}

// YCPElementRep

YCPElementRep::YCPElementRep()
    : reference_counter(0)
{
}

YCPElementRep::~YCPElementRep()
{
}

void YCPElementRep::destroy() const
{
    reference_counter--;
    if (reference_counter == 0) delete this;
    else if (reference_counter < 0) {
	y2internal("Negative reference counter");
    }
}

const YCPElementRep *YCPElementRep::clone() const
{
    reference_counter++;
    return this;
}

YCPValue YCPElementRep::asValue() const
{
    return YCPValue(static_cast<const YCPValueRep *>(this));
}

YCPStatement YCPElementRep::asStatement() const
{
    return YCPStatement(static_cast<const YCPStatementRep *>(this));
}
