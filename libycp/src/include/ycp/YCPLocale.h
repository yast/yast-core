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

   File:	YCPLocale.h

   Author:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPLocale_h
#define YCPLocale_h


#include "YCPValue.h"
#include "YCPString.h"
#include "YCPInteger.h"


/**
 * @short language dependent string constant
 * The syntactical representation is _("some string").
 */
class YCPLocaleRep : public YCPValueRep
{
    /**
     * Should be use gettext or ngettext?
     */
    bool has_msg2;

    /**
     * The arguments for gettext and ngettext.
     */
    const YCPString msg1;
    const YCPString msg2;
    const YCPInteger num;

protected:

    friend class YCPLocale;

    /**
     * Creates a new YCPLocaleRep value that depends on the
     * locale setting of the computer runnging the interpreter.
     */
    YCPLocaleRep (const YCPString& msg1);

    /**
     * Creates a new YCPLocaleRep value that depends on the
     * locale setting of the computer runnging the interpreter.
     */
    YCPLocaleRep (const YCPString& msg1, const YCPString& msg2,
		  const YCPInteger& num);

    /**
     * Cleans up.
     */
    ~YCPLocaleRep () {}

public:

    /**
     * Return the msg1.
     */
    YCPString value () const;

    /**
     * Translates the locate.
     */
    YCPString translate (const char *textdomain) const;

    /**
     * Compares two YCPLocales for equality, greaterness or smallerness.
     * @param v value to compare against
     * @param rl respect locale
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare (const YCPLocale& v, bool rl = false) const;

    /**
     * Returns a string representation of this objects value.
     */
    string toString () const;

    /**
     * Returns YT_LOCALE. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype () const;
};

/**
 * @short Wrapper for YCPLocaleRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPLocaleRep
 * with the arrow operator. See @ref YCPLocaleRep.
 */
class YCPLocale : public YCPValue
{
    DEF_COMMON (Locale, Value);
public:
    YCPLocale (const YCPString& msg1)
	: YCPValue (new YCPLocaleRep (msg1)) {}
    YCPLocale (const YCPString& msg1, const YCPString& msg2,
	       const YCPInteger& num)
	: YCPValue (new YCPLocaleRep (msg1, msg2, num)) {}
};

#endif   // YCPLocale_h
