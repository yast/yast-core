

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#include <errno.h>

#include "y2log.h"
#include "y2string.h"


template <class In, class Out> bool
recode_all (iconv_t cd, const In& in, Out* out, const typename Out::value_type
	    errorsign)
{
    typedef typename In::value_type in_t;
    typedef typename Out::value_type out_t;

    char* in_ptr = (char*)(in.data ());
    size_t in_len = in.length () * sizeof (in_t);

    const size_t buffer_size = 1024;
    out_t buffer[buffer_size];

    out->clear ();

    while (in_len != (size_t)(0))
    {
	char* tmp_ptr = (char*)(buffer);
	size_t tmp_len = buffer_size * sizeof (out_t);

	size_t r = iconv (cd, &in_ptr, &in_len, &tmp_ptr, &tmp_len);
	size_t n = (out_t*)(tmp_ptr) - buffer;

	out->append (buffer, n);

	if (r == (size_t)(-1))
	{
	    if (errno == EINVAL || errno == EILSEQ)
	    {
		// more or less harmless
		out->append (1, errorsign);
		in_ptr += sizeof (in_t);
		in_len -= sizeof (in_t);
	    }
	    else if (errno == E2BIG && n == 0)
	    {
		// fatal: the buffer is too small to hold a
		// single multi-byte sequence
		return false;
	    }
	}
    }

    return true;
}


bool
recode (iconv_t cd, const std::string& in, std::string* out)
{
    return recode_all (cd, in, out, '?');
}


bool
recode (iconv_t cd, const std::string& in, std::wstring* out)
{
    return recode_all (cd, in, out, L'?');
}


bool
recode (iconv_t cd, const std::wstring& in, std::string* out)
{
    return recode_all (cd, in, out, '?');
}


bool
recode (iconv_t cd, const std::wstring& in, std::wstring* out)
{
    return recode_all (cd, in, out, L'?');
}


bool
utf82wchar (const std::string& in, std::wstring* out)
{
    static iconv_t cd = (iconv_t)(-1);

    if (cd == (iconv_t)(-1))
    {
	cd = iconv_open ("WCHAR_T", "UTF-8");

	// TODO: also call iconv_close somewhere, perhaps use
	// pthread_key_create et.al.

	if (cd == (iconv_t)(-1))
	{
	    static bool shown_once = false;
	    if (!shown_once) {
		y2error ("iconv_open: %m");
		shown_once = true;
	    }

	    return false;
	}
    }

    return recode (cd, in, out);
}


bool
wchar2utf8 (const std::wstring& in, std::string* out)
{
    static iconv_t cd = (iconv_t)(-1);

    if (cd == (iconv_t)(-1))
    {
	cd = iconv_open ("UTF-8", "WCHAR_T");

	// TODO: also call iconv_close somewhere, perhaps use
	// pthread_key_create et.al.

	if (cd == (iconv_t)(-1))
	{
	    static bool shown_once = false;
	    if (!shown_once) {
		y2error ("iconv_open: %m");
		shown_once = true;
	    }

	    return false;
	}
    }

    return recode (cd, in, out);
}
