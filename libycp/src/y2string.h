

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef _y2string_h
#define _y2string_h


#include <iconv.h>

#include <string>


bool
recode (iconv_t cd, const std::string& in, std::string* out);

bool
recode (iconv_t cd, const std::string& in, std::wstring* out);

bool
recode (iconv_t cd, const std::wstring& in, std::string* out);

bool
recode (iconv_t cd, const std::wstring& in, std::wstring* out);


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
utf82wchar (const std::string& in, std::wstring* out);


/**
 *  Convert a wide character string into a UTF-8 encoded string.
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
wchar2utf8 (const std::wstring& in, std::string* out);


#endif
