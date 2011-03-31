

/*
 * Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef _y2crypt_h
#define _y2crypt_h


#include <string>

using std::string;


enum crypt_t { CRYPT, MD5, BIGCRYPT, BLOWFISH, SHA256, SHA512 };

bool
crypt_pass (string unencrypted, crypt_t use_crypt, string* encrypted);


#endif
