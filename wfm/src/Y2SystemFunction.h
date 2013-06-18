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

   File:	Y2SystemNamespace.h
		a wrapper interface for accessing a LiMaL namespace 
		(configurable via SCROpen/SCRClose)

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2SystemFunction_h
#define Y2SystemFunction_h

#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>

class Y2Component;

class Y2SystemFunction : public Y2Function {

    Y2Function* m_local;
    Y2Function* m_remote;
    
    bool m_use_remote;
    
    constFunctionTypePtr m_type;

    Y2SystemNamespace *m_namespace;
    
public:
    Y2SystemFunction (Y2Function* local_call, constFunctionTypePtr type, Y2SystemNamespace* system_namespace);
    
    virtual ~Y2SystemFunction ();

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
    
    void useRemote (Y2Function* remote_call);
    
    void useLocal ();
    
    string name () const;
    
    constFunctionTypePtr type () const;
};

#endif // Y2SystemFunction_h
