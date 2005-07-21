/*------------------------------------------------------------*- c++ -*-\
|									|
|		      __   __    ____ _____ ____			|
|		      \ \ / /_ _/ ___|_   _|___ \			|
|		       \ V / _` \___ \ | |   __) |			|
|			| | (_| |___) || |  / __/			|
|			|_|\__,_|____/ |_| |_____|			|
|									|
|				core system				|
|							  (C) SuSE GmbH |
\-----------------------------------------------------------------------/

   File:	Y2Function.h
		a generic interface for calling a function from a namespace

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2Function_h
#define Y2Function_h

#include <string>
using std::string;

#include "ycp/YCPValue.h"
#include "ycp/Type.h"

/**
 * A function call interface. It is an abstract base for providing
 * an interface for calling a function inside YaST. Any Y2 namespace
 * can provide its own implementation for calling functions provided
 * by the namespace. Typically returned value by 
 * Y2Namespace::createFunctionCall ("funcname", function_type).
 *
 * 
 * <pre>

// an example to call Popup::Message()
 
// first, find out the component for the namespace
Y2Component* impl = Y2ComponentBroker::provideNamespace ("Popup");
if (impl != 0)
{
    // let the component import the namespace
    Y2Namespace* ns = impl->import ("Popup");
    
    if (ns != 0)
    {
	// create a function call object for the function
        Y2Function* fnc = ns->createFunctionCall ("Message"
	    , Type::fromSignature ("void (string)"));
	    
	if (fnc != 0)
	{
	    // pass the parameter for the function
	    fnc->appendParameter (YCPString ("This is my test"));
	    fnc->finishParameters ();
	    
	    // evaluate the call
	    fnc->evaluateCall ();
	    
	    // function is not longer needed, free it
	    delete fnc;
	}
    }
} 
</pre>
 */
class Y2Function {

public:
    /**
     * Whithout this, can't delete YEFunction
     * which is derived from YCode, Y2Function
     */
    virtual ~Y2Function () {};

    /**
     * Attaches a parameter to a given position to the call.
     * @return false if there was a type mismatch
     */
    virtual bool attachParameter (const YCPValue& arg, const int position) = 0;

    /**
     * What type is expected for the next appendParameter (val) ?
     * (Used when calling from Perl, to be able to convert from the
     * simple type system of Perl to the elaborate type system of YCP)
     * @return Type::Any if number of parameters exceeded
     */
    virtual constTypePtr wantedParameterType () const = 0;

    /**
     * Appends a parameter to the call.
     * @return false if there was a type mismatch
     */
    virtual bool appendParameter (const YCPValue& arg) = 0;

    /**
     * Signal that we're done adding parameters.
     * @return false if there was a parameter missing
     */
    virtual bool finishParameters () = 0;

    /**
     * Executes the call
     */
    virtual YCPValue evaluateCall () = 0;

    /**
     * Reset the currecn parameters, so the instance
     * can be reused for the next call (appendParameter etc)
     */    
    virtual bool reset () = 0;
    
    virtual string name () const = 0;
};

#endif // Y2Function_h
