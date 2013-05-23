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

#ifndef Y2SystemNamespace_h
#define Y2SystemNamespace_h

#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>

class Y2ProgramComponent;
class Y2SystemFunction;

class Y2SystemNamespace : public Y2Namespace {

    Y2Namespace* m_local_ns;
    Y2ProgramComponent* m_remote_sender;
    bool m_use_remote;

    vector<Y2SystemFunction*> m_functions;

    string m_name;

    void unregisterFunction(Y2SystemFunction *f);

    friend class Y2SystemFunction;

public:
    Y2SystemNamespace (Y2Namespace* local_ns);

    virtual ~Y2SystemNamespace();

    //! what namespace do we implement
    virtual const string name () const;
    
    virtual const string filename () const;
    
    virtual YCPValue evaluate(bool);
    
    /**
     * Creates a function call instance, which can be used to call a 
     * function from this namespace. The object is NOT owned anymore by this
     * instance, the caller can (and should) delete it.
     *
     * @param name	name of the required function
     * @param type	the type of the function (needed for overloading)
     * @return 		an object, that can be used to call the function, or NULL on error
     */
    virtual Y2Function* createFunctionCall (const string name, constFunctionTypePtr type);
    
    void useRemote (Y2ProgramComponent* sender);
    
    void useLocal ();
};


#endif // Y2SystemNamespace_h
