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

   File:	Y2NamespaceCPP.h
		a helper classes/macros for accessing a C++-based namespace from YCP interpreter

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2NamespaceCPP_h
#define Y2NamespaceCPP_h

#include <string>
using std::string;

#include "ycp/YCPValue.h"
#include "ycp/YCode.h"
#include "ycp/YBlock.h"

class SymbolEntry;
class SymbolTable;
class Y2Function;

class Y2CPPFunction;

class Y2CPPFunctionCallBase : public YBlock
{
protected:
    SymbolEntry* m_param1;
    SymbolEntry* m_param2;
    SymbolEntry* m_param3;
    SymbolEntry* m_param4;
    
    string m_signature;

    void newParameter (YBlock* decl, uint pos, constTypePtr type);

public:
    Y2CPPFunctionCallBase (string ns, string signature) : 
	YBlock (ns, YBlock::b_unknown)
	, m_param1 (NULL)
	, m_param2 (NULL)
	, m_param3 (NULL)
	, m_param4 (NULL)
	, m_signature (signature)
    {}

    virtual void registerParameters (YBlock* decl) = 0;
    
    friend class Y2CPPFunction;
};

template <class T> class Y2CPPFunctionCall : public Y2CPPFunctionCallBase
{
protected:
    T* m_instance;
    
public:
    Y2CPPFunctionCall (string signature, T* instance) : 
	Y2CPPFunctionCallBase (instance->name (), signature)
	, m_instance (instance)
    {}
    
    friend class Y2CPPFunction;
};


class Y2CPPFunction : public YFunction {
    string m_name;
    Y2Namespace* m_parent;
    Y2CPPFunctionCallBase* m_impl;
public:
    Y2CPPFunction( Y2Namespace* parent, string name, Y2CPPFunctionCallBase* call_impl );

    SymbolEntry* sentry (uint position);
};


#define Y2FUNCTIONCALL(namespace, name, signature, impl_class, impl_func)	\
class namespace##name##Function : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlock*)		\
    {							\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func ();		\
    }							\
}


#define Y2FUNCTIONCALL1(namespace, name, signature, param1type, impl_class, impl_func)	\
class namespace##name##Function1 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function1(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlock* decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func (m_param1->value ()->as##param1type ());	\
    }							\
}

#define Y2FUNCTIONCALL2(namespace, name, signature, param1type, param2type, impl_class, impl_func)	\
class namespace##name##Function2 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function2(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlock* decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func (			\
	    m_param1->value ()->as##param1type ()	\
	    , m_param2->value ()->as##param2type ());	\
    }							\
}

#define Y2FUNCTIONCALL3(namespace, name, signature, param1type, param2type, param3type, impl_class, impl_func)	\
class namespace##name##Function3 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function3(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlock* decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
	newParameter (decl, 3, Type::Const##param3type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func ( m_param1->value ()->as##param1type ()	\
	    ,m_param2->value ()->as##param2type ()	\
	    ,m_param3->value ()->as##param3type ());	\
    }							\
}

#define Y2FUNCTIONCALL4(namespace, name, signature, param1type, param2type, param3type, param4type, impl_class, impl_func)	\
class namespace##name##Function4 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function4(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlock* decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
	newParameter (decl, 3, Type::Const##param3type );	\
	newParameter (decl, 4, Type::Const##param4type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func (m_param1->value ()->as##param1type ()	\
	    ,m_param2->value ()->as##param2type ()	\
	    ,m_param3->value ()->as##param3type ()	\
	    ,m_param4->value ()->as##param4type ());	\
    }							\
}

#define REGISTERFUNCTIONCALL(position, namespace, name)			\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function (this));			\
	enterSymbol (#name, mf->sentry (position), 0);			\
    } while (0)


#define REGISTERFUNCTIONCALL1(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function1 (this));			\
	enterSymbol (#name, mf->sentry (position), 0);			\
    } while (0)


#define REGISTERFUNCTIONCALL2(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function2 (this));			\
	enterSymbol (#name, mf->sentry (position), 0);			\
    } while (0)


#define REGISTERFUNCTIONCALL3(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function3 (this));			\
	enterSymbol (#name, mf->sentry (position), 0);			\
    } while (0)


#define REGISTERFUNCTIONCALL4(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function4 (this));			\
	enterSymbol (#name, mf->sentry (position), 0);			\
    } while (0)


#endif // Y2Namespace_h
