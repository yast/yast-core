/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	Y2CCWFM.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Mathias Kettner <kettner@suse.de>

/-*/

#ifndef Y2CCWFM_h
#define Y2CCWFM_h

#include <y2/Y2ComponentCreator.h>

class Y2CCWFM : public Y2ComponentCreator
{
    /**
     * Allow only one instance of wfm.
     * This used to be the job of Y2ScriptComponent (the code is still there)
     * but now the Perl bindings go around it.
     */
    static Y2Component *m_wfm;

public:

    /**
     * Creates a wfm component creator.
     */
    Y2CCWFM ();

    /**
     * Creates a new wfm component.
     */
    Y2Component *create (const char *name) const;

    /**
     * returns false: WFM is a client component.
     */
    bool isServerCreator () const;
    
    /**
     * WFM does not provide the namespaces itself, Y2CCScript does that.
     */
    Y2Component* provideNamespace (const char*) { return NULL; }
};


#endif // Y2CCWFM_h
