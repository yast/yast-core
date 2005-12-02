/*---------------------------------------------------------------------\
|                                                                       |
|                       __   __    ____ _____ ____                      |
|                       \ \ / /_ _/ ___|_   _|___ \                     |
|                        \ V / _` \___ \ | |   __) |                    |
|                         | | (_| |___) || |  / __/                     |
|                         |_|\__,_|____/ |_| |_____|                    |
|                                                                       |
|                                core system                            |
|                                          (C) SUSE Linux Products GmbH |
\----------------------------------------------------------------------/

   File:       YMLparser.cc

   Author:     Jens Daniel Schmidt <jdsn@suse.de>
   Maintainer: Jens Daniel Schmidt <jdsn@suse.de>

/-*/
// -*- c++ -*-

/*

typedef enum {
    XML_ELEMENT_NODE=       1,
    XML_ATTRIBUTE_NODE=     2,
    XML_TEXT_NODE=      3,
    XML_CDATA_SECTION_NODE= 4,
    XML_ENTITY_REF_NODE=    5,
    XML_ENTITY_NODE=        6,
    XML_PI_NODE=        7,
    XML_COMMENT_NODE=       8,
    XML_DOCUMENT_NODE=      9,
    XML_DOCUMENT_TYPE_NODE= 10,
    XML_DOCUMENT_FRAG_NODE= 11,
    XML_NOTATION_NODE=      12,
    XML_HTML_DOCUMENT_NODE= 13,
    XML_DTD_NODE=       14,
    XML_ELEMENT_DECL=       15,
    XML_ATTRIBUTE_DECL=     16,
    XML_ENTITY_DECL=        17,
    XML_NAMESPACE_DECL=     18,
    XML_XINCLUDE_START=     19,
    XML_XINCLUDE_END=       20
#ifdef LIBXML_DOCB_ENABLED
   ,XML_DOCB_DOCUMENT_NODE= 21
#endif
} xmlElementType;

*/

#define TESTFILE    "test/minitest.xml"

#include <ycp/YCPTerm.h>
#include <ycp/YCPValue.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPString.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xmlstring.h>
#include <string>



YCPList xml2ycp(xmlNode * node, bool symbol = false)
{
    xmlNode *cur_node = node;
    YCPList ymlList;

    for (  ; cur_node ; cur_node = cur_node->next)
    {
        YCPValue newelement;
        YCPList childs;
        newelement = YCPVoid();

        // identify kind of element; create a YCPValue
        switch (cur_node->type)
        {
            case XML_ELEMENT_NODE       :
   
                  if ( !xmlStrcmp(cur_node->name,  (const xmlChar*) "symbol" ) ) {
                      char *prop = (char*) xmlGetProp(cur_node, (const xmlChar *)"name");
                      if (prop)
                        newelement = YCPSymbol( prop );
                      else {
                        fprintf (stderr, "Error! Symbol without a name!\n");
                        newelement = YCPVoid();
                      }
                  }
                  else newelement = YCPTerm(  (char*) cur_node->name );
                  break;

            case XML_ATTRIBUTE_NODE     :
                  newelement = YCPVoid();
                  break;

            case XML_TEXT_NODE          :
                  if ( xmlIsBlankNode(cur_node) )
                      newelement = YCPVoid();
                  else newelement = YCPString( (char*) cur_node->content );
                      break;

            case XML_CDATA_SECTION_NODE : newelement = YCPVoid(); break;
                  // cur_node->content);

            case XML_ENTITY_REF_NODE    : newelement = YCPVoid(); break;
            case XML_ENTITY_NODE        : newelement = YCPVoid(); break;
            case XML_PI_NODE            : newelement = YCPVoid(); break;
            case XML_COMMENT_NODE       : newelement = YCPVoid(); break;
            case XML_DOCUMENT_NODE      : newelement = YCPVoid(); break;
            case XML_DOCUMENT_TYPE_NODE : newelement = YCPVoid(); break;
            case XML_DOCUMENT_FRAG_NODE : newelement = YCPVoid(); break;
            case XML_NOTATION_NODE      : newelement = YCPVoid(); break;
            case XML_HTML_DOCUMENT_NODE : newelement = YCPVoid(); break;
            case XML_DTD_NODE           : newelement = YCPVoid(); break;
            case XML_ELEMENT_DECL       : newelement = YCPVoid(); break;
            case XML_ATTRIBUTE_DECL     : newelement = YCPVoid(); break;
            case XML_ENTITY_DECL        : newelement = YCPVoid(); break;
            case XML_NAMESPACE_DECL     : newelement = YCPVoid(); break;
            case XML_XINCLUDE_START     : newelement = YCPVoid(); break;
            case XML_XINCLUDE_END       : newelement = YCPVoid(); break;
            case XML_DOCB_DOCUMENT_NODE : newelement = YCPVoid(); break;
            default                     : newelement = YCPVoid(); break;
        }


        // now we've got to add the found element to our ymlList
        if ( !newelement.isNull() )
        {
            // at first - get the elements childs
            childs = xml2ycp(cur_node->children);

            if ( ! childs.isEmpty() )
                for (int i=0; i < childs.size(); i++ )
                    newelement = newelement->asTerm()->functionalAdd( childs->value(i) );
            if ( !newelement->isVoid() ) ymlList.add( newelement );
        }
    }
    return ymlList;
}


string YMLParser(xmlNodePtr &node)
{
    YCPList ymlList;
    ymlList = xml2ycp(node);
    string ycpString="";

    for(int i=0; i<=ymlList->size() ; i++ )
        if ( !ymlList->value(i).isNull() && !ymlList->value(i)->isVoid() )
        {
            ycpString += ymlList->value(i)->toString().c_str();
            if (i != ymlList->size()-1) ycpString += ", ";
        }

    return ycpString;
}




string YMLDocumentParser(xmlNodePtr &node)
{
    if ( xmlStrcmp(node->name, (const xmlChar*) "YML") )
    {   printf("no yml-tag found\n"); 
        exit (0);
    }

    xmlNodePtr commands;
    commands=node->children;
    string commandString;
    string subtree;

    for ( ; commands ;  commands=commands->next)
    {
        if ( !xmlStrcmp(commands->name, (const xmlChar*) "command") )
        {
            // we found a command node - lets create a string

            commandString += "UI::";
            commandString += (char*) xmlGetProp(commands, (const xmlChar*) "action");
            commandString += "(";

            //subtree = xml2ycp(commands->children);
            subtree = YMLParser(commands->children);
            commandString += subtree;

            commandString += ");\n";
        }

    }
    return commandString;
}






/*  New Parser Structure


+parse doc
    -test/validate
    -find YML-root-node

    +build output string
        -echo "{\n"
        +parse inner commands
            +while (next-node)
                -find command
                -parse (nested) parameters
                -echo $command
            -end
        -echo "\n}"
    -output string
-exit

*/





int main( void )
{
    xmlDocPtr xmldoc;
    xmlNodePtr current;

    xmldoc = xmlParseFile(TESTFILE);

    if (xmldoc == NULL)
    {
        fprintf(stderr, "error: could not parse document.\n");
        return 0;
    }

    current = xmlDocGetRootElement(xmldoc);

    if (current == NULL)
    {
        fprintf(stderr, "info: document is empty.\n");
        xmlFreeDoc(xmldoc);
        return 0;
    }

    if ( xmlStrcmp(current->name, (const xmlChar*) "YML") )
    {
       fprintf(stderr, "error: wrong document type, root node YML expected"); 
       xmlFreeDoc(xmldoc); 
       return 0;
    }

   // fprintf(stderr, "success: obviously parsing the document worked quite fine!\n");

    YCPValue yt;
    //yt = YMLParser(current);
    printf("{\n\n%s\n}", YMLDocumentParser(current).c_str() );


/*  temporary disabled
    if ( yt.isNull() ) printf("yt is NULL\n");
    else
    {
        // printf(" yt is not NULL\n");
        if ( yt->isVoid() )
        {
            printf("yt is VOID\n");
            return 0;
        }
        else printf("\n%s\n", yt->toString().c_str() );
    }
*/

    xmlFreeDoc(xmldoc);
    return 0;
}
    /*
    YCPTerm test("TEST"); test->add( YCPTerm("pups")); test->add( YCPTerm("furz")); YCPTerm popel("popel"); popel->add( YCPTerm("Müller")); popel->add( YCPTerm("Meier")); popel->add( YCPTerm("Piepenhuber")); test->add(popel); element = test; 
    printf("TEST %s\n", test->toString().c_str() );
                             for (int i=0; i++ < offset; printf(" "));
                             printf("found ELEMENT %s\n", cur_node->name);
    */
