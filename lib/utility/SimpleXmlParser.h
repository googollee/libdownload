#ifndef SIMPLE_XML_PARSER_CLASS_HEAD
#define SIMPLE_XML_PARSER_CLASS_HEAD

#include <glib.h>

#include <memory>

namespace Utility
{

struct SimpleXmlParserData;

class SimpleXmlParser
{
public:
    /* Called for open tags <foo bar="baz"> */
    virtual void startElement(const char*  elementName,
                              const char** attributeNames,
                              const char** attributeValues);

    /* Called for close tags </foo> */
    virtual void endElement(const char* elementName);

    /* Called for character data */
    /* text is not nul-terminated */
    virtual void text(const char* text,
                      size_t textLen);

    /* Called for strings that should be re-saved verbatim in this same
     * position, but are not otherwise interpretable.  At the moment
     * this includes comments and processing instructions.
     */
    /* text is not nul-terminated. */
    virtual void passthrough(const char* text,
                             size_t textLen);

    /* Called on error, including one set by other
     * methods in the vtable. The GError should not be freed.
     */
    virtual void error(int err, const char* errorstr);

    SimpleXmlParser();
    virtual ~SimpleXmlParser();

    bool feed(const char* str, int len);
    bool feed(const char* str);
    bool finish();

    const char* getElement();
    const char* getError(int* code);

private:
    GMarkupParseContext* context_;
    GError* error_;

    friend void setParserError(SimpleXmlParser* parser, GError* error);
};

}

#endif
