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

/** 
 * @file Y2NamespaceCPP.h
 * @brief Helper classes/macros for accessing a C++-based namespace from YCP interpreter
 *
 * These macros and helper classes ease building a table of a functions to be avaiable
 * to the rest of YaST.
 *
 * Each function must be implemented using a class. The class declaration and all needed machinery
 * are hidden in the @ref Y2FUNCTIONCALL macros. Then, the macro must be registered
 * at runtime using the corresponding @ref REGISTERFUNCTIONCALL macro.
 */

#ifndef Y2NamespaceCPP_h
#define Y2NamespaceCPP_h

#include <string>
using std::string;

#include "ycp/YCPValue.h"
#include "ycp/YCode.h"
#include "ycp/YBlock.h"
#include "ycp/SymbolEntryPtr.h"
#include "ycp/y2log.h"

class SymbolTable;
class Y2Function;

class Y2CPPFunction;

class Y2CPPFunctionCallBase : public YBlock
{
protected:
    SymbolEntryPtr m_param1;
    SymbolEntryPtr m_param2;
    SymbolEntryPtr m_param3;
    SymbolEntryPtr m_param4;
    
    string m_signature;

    void newParameter (YBlockPtr decl, uint pos, constTypePtr type);

public:
    Y2CPPFunctionCallBase (string ns, string signature) : 
	YBlock (ns, YBlock::b_unknown)
	, m_param1 (NULL)
	, m_param2 (NULL)
	, m_param3 (NULL)
	, m_param4 (NULL)
	, m_signature (signature)
    {}

    virtual void registerParameters (YBlockPtr decl) = 0;
    
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

    SymbolEntryPtr sentry (uint position);
};


/**
 * @def Y2FUNCTIONCALL(namespace, name, signature, impl_class, impl_func)
 * @brief Macro to declare a class for calling a C++-based method without parameters.
 *
 * The method @ref impl_func must return a YCPValue.
 *
 * @param namespace	in which namespace should the method exist
 * @param name		the name in the namespace, how should the method be known
 * @param signature	textual representation of the method signature
 * @param impl_class	C++ class implementing the method
 * @param impl_func	method of the @ref impl_class to be called
 *
 * The following example will declare a class allowing to call string Pkg::YouGetDirectory(), which
 * is implemented by a method YCPValue PkgModuleFunctions::YouGetDirectory ():
 *
 * Y2FUNCTIONCALL (Pkg, YouGetDirectory, "string ()", PkgModuleFunctions, YouGetDirectory)
 */
#define Y2FUNCTIONCALL(namespace, name, signature, impl_class, impl_func)	\
class namespace##name##Function : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlockPtr)		\
    {							\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	return m_instance->impl_func ();		\
    }							\
}


/**
 * @def Y2FUNCTIONCALL1(namespace, name, signature, param1type, impl_class, impl_func)
 * @brief Macro to declare a class for calling a C++-based method with a single parameter.
 *
 * The method @ref impl_func must return a YCPValue. The type of a parameter is prefixed
 * with YCP to get a valid YCPValue-based class. As a consequence, the parameter
 * type must start with the uppercase letter, for example List for YCPList.
 *
 * @param namespace	in which namespace should the method exist
 * @param name		the name in the namespace, how should the method be known
 * @param signature	textual representation of the method signature
 * @param param1type	the type of the method parameter
 * @param impl_class	C++ class implementing the method
 * @param impl_func	method of the @ref impl_class to be called
 *
 * The following example will declare a class allowing to call boolean Pkg::IsAvailable(string tag), which
 * is implemented by a method YCPValue PkgModuleFunctions::IsAvailable (const YCPString& tag):
 *
 * Y2FUNCTIONCALL1 (Pkg, IsAvailable, "boolean (string)", PkgModuleFunctions, IsAvailable)
 */
#define Y2FUNCTIONCALL1(namespace, name, signature, param1type, impl_class, impl_func)	\
class namespace##name##Function1 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function1(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlockPtr decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	YCPValue param1 = m_param1->value ();		\
	if (param1.isNull () || param1->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	return m_instance->impl_func (param1->as##param1type ());	\
    }							\
}

/**
 * @def Y2FUNCTIONCALL2(namespace, name, signature, param1type, param2type, impl_class, impl_func)
 * @brief Macro to declare a class for calling a C++-based method with two parameters.
 *
 * The method @ref impl_func must return a YCPValue. The types of parameters are prefixed
 * with YCP to get a valid YCPValue-based classes. As a consequence, the parameter
 * type must start with the uppercase letter, for example List for YCPList.
 *
 * @param namespace	in which namespace should the method exist
 * @param name		the name in the namespace, how should the method be known
 * @param signature	textual representation of the method signature
 * @param param1type	the type of the method 1st parameter
 * @param param2type	the type of the method 2nd parameter
 * @param impl_class	C++ class implementing the method
 * @param impl_func	method of the @ref impl_class to be called
 *
 * The following example will declare a class allowing to call boolean Pkg::TargetInit(string root, boolean new), 
 * which is implemented by a method YCPValue PkgModuleFunctions::TargetInit (const YCPString& root, const YCPString& new):
 *
 * Y2FUNCTIONCALL2 (Pkg, TargetInit, "boolean (string, boolean)", PkgModuleFunctions, TargetInit)
 */
#define Y2FUNCTIONCALL2(namespace, name, signature, param1type, param2type, impl_class, impl_func)	\
class namespace##name##Function2 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function2(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlockPtr decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	YCPValue param1 = m_param1->value ();		\
	YCPValue param2 = m_param2->value ();		\
	if (param1.isNull () || param1->isVoid ())	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param2.isNull () || param2->isVoid ())	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	return m_instance->impl_func (			\
	    param1->as##param1type ()			\
	    , param2->as##param2type ());		\
    }							\
}

/**
 * @def Y2FUNCTIONCALL3(namespace, name, signature, param1type, param2type, param3type, impl_class, impl_func)
 * @brief Macro to declare a class for calling a C++-based method with three parameters.
 *
 * The method @ref impl_func must return a YCPValue. The types of parameters are prefixed
 * with YCP to get a valid YCPValue-based classes. As a consequence, the parameter
 * type must start with the uppercase letter, for example List for YCPList.
 *
 * @param namespace	in which namespace should the method exist
 * @param name		the name in the namespace, how should the method be known
 * @param signature	textual representation of the method signature
 * @param param1type	the type of the method 1st parameter
 * @param param2type	the type of the method 2nd parameter
 * @param param3type	the type of the method 3rd parameter
 * @param impl_class	C++ class implementing the method
 * @param impl_func	method of the @ref impl_class to be called
 *
 * The following example will declare a class allowing to call string Pkg::SourceProvideFile (integer SrcId, integer medianr, string file), 
 * which is implemented by a method 
 * YCPValue PkgModuleFunctions::SourceProvideFile (const YCPInteger& id, const YCPInteger& mid, const YCPString& f):
 *
 * Y2FUNCTIONCALL3 (Pkg, SourceProvideFile, "string (integer, integer, string)", PkgModuleFunctions, SourceProvideFile)
 */
#define Y2FUNCTIONCALL3(namespace, name, signature, param1type, param2type, param3type, impl_class, impl_func)	\
class namespace##name##Function3 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function3(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlockPtr decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
	newParameter (decl, 3, Type::Const##param3type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	YCPValue param1 = m_param1->value ();		\
	YCPValue param2 = m_param2->value ();		\
	YCPValue param3 = m_param3->value ();		\
	if (param1.isNull () || param1->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param2.isNull () || param2->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param3.isNull () || param3->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	return m_instance->impl_func ( param1->as##param1type ()	\
	    ,param2->as##param2type ()			\
	    ,param3->as##param3type ());		\
    }							\
}

/**
 * @def Y2FUNCTIONCALL4(namespace, name, signature, param1type, param2type, param3type, param4type, impl_class, impl_func)
 * @brief Macro to declare a class for calling a C++-based method with three parameters.
 *
 * The method @ref impl_func must return a YCPValue. The types of parameters are prefixed
 * with YCP to get a valid YCPValue-based classes. As a consequence, the parameter
 * type must start with the uppercase letter, for example List for YCPList.
 *
 * @param namespace	in which namespace should the method exist
 * @param name		the name in the namespace, how should the method be known
 * @param signature	textual representation of the method signature
 * @param param1type	the type of the method 1st parameter
 * @param param2type	the type of the method 2nd parameter
 * @param param3type	the type of the method 3rd parameter
 * @param param4type	the type of the method 4th parameter
 * @param impl_class	C++ class implementing the method
 * @param impl_func	method of the @ref impl_class to be called
 *
 * The following example will declare a class allowing to call 
 * list<string> Pkg::FilterPackages (bool byAuto, bool byApp, bool byUser, bool names_only), 
 * which is implemented by a method 
 * YCPValue PkgModuleFunctions::FilterPackages(const YCPBoolean& byAuto, const YCPBoolean& byApp, const YCPBoolean& byUser, const YCPBoolean& names_only):
 *
 * Y2FUNCTIONCALL4 ( Pkg, FilterPackages, "list<string> (boolean, boolean, boolean, boolean)", Boolean, 
 * Boolean, Boolean, Boolean, PkgModuleFunctions, FilterPackages);
 */
#define Y2FUNCTIONCALL4(namespace, name, signature, param1type, param2type, param3type, param4type, impl_class, impl_func)	\
class namespace##name##Function4 : public Y2CPPFunctionCall <impl_class> {	\
public:							\
    namespace##name##Function4(impl_class* instance) : 	\
	Y2CPPFunctionCall <impl_class> (signature, instance)	\
    {}							\
    virtual void registerParameters (YBlockPtr decl)	\
    {							\
	newParameter (decl, 1, Type::Const##param1type );	\
	newParameter (decl, 2, Type::Const##param2type );	\
	newParameter (decl, 3, Type::Const##param3type );	\
	newParameter (decl, 4, Type::Const##param4type );	\
    }							\
    virtual YCPValue evaluate (bool cse=false)		\
    {							\
	if (cse) return YCPNull ();			\
	YCPValue param1 = m_param1->value ();		\
	YCPValue param2 = m_param2->value ();		\
	YCPValue param3 = m_param3->value ();		\
	YCPValue param4 = m_param4->value ();		\
	if (param1.isNull () || param1->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param2.isNull () || param2->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param3.isNull () || param3->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	if (param4.isNull () || param4->isVoid ()) 	\
	{						\
	    ycperror ("Passing 'nil' to %s::%s", #namespace, #name);	\
	    return YCPNull (); 				\
	}						\
	return m_instance->impl_func (param1->as##param1type ()	\
	    ,param2->as##param2type ()			\
	    ,param3->as##param3type ()			\
	    ,param4->as##param4type ());		\
    }							\
}

/**
 * @def REGISTERFUNCTIONCALL(position, namespace, name)
 * @brief Registers a function without parameter in a namespace.
 *
 * The function class must be already declared using the @ref Y2FUNCTIONCALL macro.
 * This macro registers the symbol in a table of globally visible symbols
 * of the namespace.
 *
 * @param position	integer ID of the function in the namespace, must be unique
 * @param namespace	the namespace, where the function should be registered
 * @param name		the name of the function in the namespace
 */
#define REGISTERFUNCTIONCALL(position, namespace, name)			\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function (this));			\
	enterSymbol (mf->sentry (position), 0);			\
    } while (0)


/**
 * @def REGISTERFUNCTIONCALL1(position, namespace, name)
 * @brief Registers a function with a single parameter in a namespace.
 *
 * The function class must be already declared using the @ref Y2FUNCTIONCALL1 macro.
 * This macro registers the symbol in a table of globally visible symbols
 * of the namespace.
 *
 * @param position	integer ID of the function in the namespace, must be unique
 * @param namespace	the namespace, where the function should be registered
 * @param name		the name of the function in the namespace
 */
#define REGISTERFUNCTIONCALL1(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function1 (this));			\
	enterSymbol (mf->sentry (position), 0);			\
    } while (0)


/**
 * @def REGISTERFUNCTIONCALL2(position, namespace, name)
 * @brief Registers a function with two parameters in a namespace.
 *
 * The function class must be already declared using the @ref Y2FUNCTIONCALL2 macro.
 * This macro registers the symbol in a table of globally visible symbols
 * of the namespace.
 *
 * @param position	integer ID of the function in the namespace, must be unique
 * @param namespace	the namespace, where the function should be registered
 * @param name		the name of the function in the namespace
 */
#define REGISTERFUNCTIONCALL2(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function2 (this));			\
	enterSymbol (mf->sentry (position), 0);			\
    } while (0)


/**
 * @def REGISTERFUNCTIONCALL3(position, namespace, name)
 * @brief Registers a function with three parameters in a namespace.
 *
 * The function class must be already declared using the @ref Y2FUNCTIONCALL3 macro.
 * This macro registers the symbol in a table of globally visible symbols
 * of the namespace.
 *
 * @param position	integer ID of the function in the namespace, must be unique
 * @param namespace	the namespace, where the function should be registered
 * @param name		the name of the function in the namespace
 */
#define REGISTERFUNCTIONCALL3(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function3 (this));			\
	enterSymbol (mf->sentry (position), 0);			\
    } while (0)


/**
 * @def REGISTERFUNCTIONCALL4(position, namespace, name)
 * @brief Registers a function with four parameters in a namespace.
 *
 * The function class must be already declared using the @ref Y2FUNCTIONCALL4 macro.
 * This macro registers the symbol in a table of globally visible symbols
 * of the namespace.
 *
 * @param position	integer ID of the function in the namespace, must be unique
 * @param namespace	the namespace, where the function should be registered
 * @param name		the name of the function in the namespace
 */
#define REGISTERFUNCTIONCALL4(position, namespace, name)		\
    do {								\
	Y2CPPFunction* mf = new Y2CPPFunction (				\
	    this, 							\
	    #name,							\
	    new namespace##name##Function4 (this));			\
	enterSymbol (mf->sentry (position), 0);			\
    } while (0)


#endif // Y2Namespace_h
