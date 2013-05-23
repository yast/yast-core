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

   File:	Y2StdioFunction.h
		a remote function call

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2StdioFunction_h
#define Y2StdioFunction_h

#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>

class Y2ProgramComponent;

class Y2StdioFunction : public Y2Function {

    string m_namespace;
    string m_name;
    constFunctionTypePtr m_type;
    vector<YCPValue> m_parameters;

    Y2ProgramComponent* m_sender;
    
public:
    Y2StdioFunction (string ns, string name
	, constFunctionTypePtr type, Y2ProgramComponent* sender);
    
    virtual ~Y2StdioFunction ();

    /**
     * Attaches a parameter to a given position to the call.
     * @return false if there was a type mismatch
     */
    virtual bool attachParameter (const YCPValue& arg, const int position);

    /**
     * What type is expected for the next appendParameter (val) ?
     * (Used when calling from Perl, to be able to convert from the
     * simple type system of Perl to the elaborate type system of YCP)
     * @return Type::Any if number of parameters exceeded
     */
    virtual constTypePtr wantedParameterType () const;

    /**
     * Appends a parameter to the call.
     * @return false if there was a type mismatch
     */
    virtual bool appendParameter (const YCPValue& arg);

    /**
     * Signal that we're done adding parameters.
     * @return false if there was a parameter missing
     */
    virtual bool finishParameters ();

    /**
     * Executes the call
     */
    virtual YCPValue evaluateCall ();

    /**
     * Reset the currecn parameters, so the instance
     * can be reused for the next call (appendParameter etc)
     */    
    virtual bool reset ();
    
    virtual string name () const;
};

#endif // Y2StdioFunction_h
