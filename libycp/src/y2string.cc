

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#include <errno.h>
#include <iconv.h>

#include "y2log.h"
#include "y2string.h"


bool
utf82wchar (const string& in, wstring* out)
{
    static iconv_t cd = (iconv_t)(-1);

    if (cd == (iconv_t)(-1))
    {
	cd = iconv_open ("WCHAR_T", "UTF-8");

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

    size_t in_len = in.length ();
    char* in_ptr = const_cast <char*> (in.c_str ());

    size_t tmp_size = 50 * sizeof (wchar_t);
    char tmp[tmp_size + sizeof (wchar_t)];

    *out = L"";

    do {

	size_t tmp_len = tmp_size;
	char* tmp_ptr = tmp;

	size_t iconv_ret = iconv (cd, &in_ptr, &in_len, &tmp_ptr, &tmp_len);

	*((wchar_t*) tmp_ptr) = L'\0';
	*out += wstring ((wchar_t*) &tmp);

	if (iconv_ret == (size_t)(-1))
        {
	    if (errno == EINVAL || errno == EILSEQ)
            {
                in_ptr++;
		*out += L'?';
		continue;
	    }
	}

    } while (in_len != 0);

    return true;
}
