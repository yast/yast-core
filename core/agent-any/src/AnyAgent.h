/**
 *
 *  AnyAgent.h
 *
 *  Creator:	kkaempf@suse.de
 *  Maintainer:	kkaempf@suse.de
 *
 */

// -*- c++ -*-

#ifndef AnyAgent_h
#define AnyAgent_h


#include <stdio.h>
#include <sys/types.h>
#include <stack>
#include <scr/SCRAgent.h>

using std::stack;


/**
 * @short SCR Agent for access to any describeable file
 */
class AnyAgent : public SCRAgent
{
    /**
     * Starts with false and is set to true as soon as the Description
     * is read. Any Read/Write/Dir command prior to the reading of the
     * description is invalid.
     */
    bool description_read;

    /**
     * file cache
     * mtime = file's mtime when alldata was filled
     * cache = parsed file
     * alldata = list of YCPStringRep with all data from file
     */
    time_t mtime;
    YCPValue cache;
    bool cchanged;
    YCPList alldata;
    bool achanged;

    /**
     * true if file is read-only
     */
    bool mReadOnly;

    /**
     * type of mName
     */
    enum { MTYPE_NONE, MTYPE_FILE, MTYPE_PROG, MTYPE_LOCAL } mType;

    /**
     * name of system file or program
     */
    YCPValue mName;

    /**
     * comment characters
     */
    string mComment;
    bool isFillup;

    /**
     * syntax description of system file
     */
    YCPValue mSyntax;

    /**
     * syntax description of header lines
     */
    YCPValue mHeader;

    /**
     * Used for line counting while parsing the target file.
     */
    int line_number;

    /**
     * tuple parsing
     */
    stack <string> tupleName;
    stack <YCPValue> tupleValue;
    bool tupleContinue;

public:

    /**
     * Creates a new AnyAgent.
     */
    AnyAgent ();

    /**
     * Cleans up
     */
    ~AnyAgent ();

    /**
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    YCPValue Read (const YCPPath & path, const YCPValue & arg = YCPNull (), const YCPValue & opt = YCPNull ());

    /**
     * Writes data. Destroy the result after use.
     */
    YCPBoolean Write (const YCPPath & path, const YCPValue & value,
		    const YCPValue & arg = YCPNull ());

    /**
     * Get a list of all subtrees.
     */
    YCPList Dir (const YCPPath & path);

    /**
     * Evaluates the Description () command
     */
    YCPValue otherCommand (const YCPTerm & term);

private:

    YCPValue readValueByPath (const YCPValue & value, const YCPPath & path);
    YCPValue writeValueByPath (const YCPValue & current, const YCPPath & path,
			       const YCPValue & value);

    YCPValue findSyntax (const YCPValue & syntax, const YCPPath & path);

    const char * get_line (FILE * fp);

    //
    // Basic types (AnyAgentBasic)
    //

    YCPValue parseIp4Number (char const *&lptr, bool optional);
    const string unparseIp4Number (const YCPValue & value);

    YCPValue parseBoolean (char const *&lptr, bool optional);
    const string unparseBoolean (const YCPValue & value);

    YCPValue parseNumber (char const *&lptr, bool optional);
    const string unparseNumber (const YCPValue & value);

    YCPValue parseHexval (char const *&lptr, bool optional);
    const string unparseHexval (const YCPValue & value);

    YCPValue parseString (char const *&lptr, const char *set, const char *stripped,
			  bool optional);
    const string unparseString (const YCPValue & syntax, const YCPValue & stripped,
				const YCPValue & value);

    YCPValue parseFloat (char const *&lptr, bool optional);
    const string unparseFloat (const YCPValue & value);

    YCPValue parseHostname (char const *&lptr, bool optional);
    const string unparseHostname (const YCPValue & value);

    YCPValue parseUsername (char const *&lptr, bool optional);
    const string unparseUsername (const YCPValue & value);

    YCPValue parseVerbose (char const *&lptr, const char *match, bool optional);
    const string unparseVerbose (const YCPValue & value);

    YCPValue parseSeparator (char const *&lptr, const char *match, bool optional);
    const string unparseSeparator (const YCPValue & match);

    const char * getLine (void);
    const string putLine (const string s);

    //
    // Complex types (AnyAgentComplex)
    //

    YCPValue parseChoice (char const *&line, const YCPList & syntax, bool optional);
    const string unparseChoice (const YCPList & syntax, const YCPValue & value);

    YCPValue parseSequence (char const *&line, const YCPList & syntax, bool optional);
    const string unparseSequence (const YCPList & syntax, const YCPValue & value);

    YCPValue parseList (char const *&line, const YCPList & syntax, bool optional);
    const string unparseList (const YCPList & syntax, const YCPValue & value);

    YCPValue parseTuple (char const *&line, const YCPList & syntax, bool optional);
    const string unparseTuple (const YCPList & syntax, const YCPValue & value);

    YCPValue parseData (char const *&line, const YCPValue & syntax, bool optional);
    const string unparseData (const YCPValue & syntax, const YCPValue & value);

    YCPValue validateCache (const YCPList & data, const YCPValue & arg = YCPNull ());
    YCPValue readFile (const YCPValue & arg);
    const string writeFile (const YCPValue & arg);

    string evalArg (const YCPValue & arg);

    int lineNumber () const;

};


#endif // AnyAgent_h
