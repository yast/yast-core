
#include <y2util/y2log.h>
#include <y2/Y2Namespace.h>
#include <y2/Y2NamespaceCPP.h>
#include <y2/Y2Component.h>
#include <y2/Y2ComponentCreator.h>

#include <ycp/YBlock.h>
#include <ycp/YExpression.h>
#include <ycp/YCPVoid.h>

class ExampleNamespace : public Y2Namespace {
public:

    YCPValue evaluateOpenDialog (YCPTerm foo)
    {
	return YCPString ("It's a live!");
    }

    Y2FUNCTIONCALL1 ( Fun, Function, "string (term)", Term,  ExampleNamespace, evaluateOpenDialog );
    
    ExampleNamespace () 
    {
	REGISTERFUNCTIONCALL1 ( 0, Fun, Function );
	
	y2debug ("Example namespace: %s", table ()->toString ().c_str ());
    }
    
    virtual const string name () const
    {
	return "Fun";
    }
    
    virtual const string filename () const
    {
	return "FunFile";
    }
    
    virtual string toString () const
    {
	return "{ string Fun }";
    }
    
    virtual YCPValue evaluate (bool cse = false )
    {
	if (cse) return YCPNull ();
	else return YCPVoid ();
    }
    
    virtual Y2Function* createFunctionCall (const string name)
    {
	TableEntry *func_te = table ()->find (name.c_str (), SymbolEntry::c_function);
	if (func_te)
	{
	    return new YEFunction (func_te->sentry ());
	}
	y2error ("No such function %s", name.c_str ());
	return NULL;
    }

};

class Y2ExampleComponent : public Y2Component {
public:
    virtual Y2Namespace *import (const char* name, const char* timestamp)
    {
	// for internal components, we don't have to compare the timestamps
	// ... well, we should track changes in symbol numbering
	if ( strcmp (name, "Example") == 0)
	{
	    return new ExampleNamespace ();
	}
	
	return 0;
    }
    virtual string name () const { return "example";}

} ExampleComponent;


class Y2ExampleCC : public Y2ComponentCreator
{
public:
    Y2ExampleCC() : Y2ComponentCreator(Y2ComponentBroker::SCRIPT) {}
    virtual  Y2Component *provideNamespace(const char *name)
    {
	if (strcmp (name, "Example") == 0)
    	    return &ExampleComponent;
	else
	    return 0;
    }
    virtual bool isServerCreator () const { return true;};
} examplecc;
