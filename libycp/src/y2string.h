

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef _y2string_h
#define _y2string_h


#include <string>

using std::string;
using std::wstring;


/**
 *  Convert a UTF-8 encoded string into a wide character string.
 *  Illegal input sequences are replaces by question marks.
 *
 *  Return false if no conversion was possible due to some general
 *  error. It does not return false if the input only contains
 *  illegal sequences.
 *
 *  The special feature of this function is that it does not depend
 *  on the current locale.
 */
bool
utf82wchar (const string& in, wstring* out);


#endif
