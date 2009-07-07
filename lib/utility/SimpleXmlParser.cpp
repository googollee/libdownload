#include "SimpleXmlParser.h"

namespace Utility
{

static void start_element(GMarkupParseContext *context,
                          const gchar         *element_name,
                          const gchar        **attribute_names,
                          const gchar        **attribute_values,
                          gpointer             user_data,
                          GError             **error)
{
    SimpleXmlParser *parser = (SimpleXmlParser*)user_data;
    parser->startElement(element_name, attribute_names, attribute_values);
}

static void end_element(GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
    SimpleXmlParser *parser = (SimpleXmlParser*)user_data;
    parser->endElement(element_name);
}

static void text(GMarkupParseContext *context,
                 const gchar         *text,
                 gsize                text_len,
                 gpointer             user_data,
                 GError             **error)
{
    SimpleXmlParser *parser = (SimpleXmlParser*)user_data;
    parser->text(text, text_len);
}

static void passthrough(GMarkupParseContext *context,
                        const gchar         *passthrough_text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error)
{
    SimpleXmlParser *parser = (SimpleXmlParser*)user_data;
    parser->passthrough(passthrough_text, text_len);
}

static void error(GMarkupParseContext *context,
                  GError              *error,
                  gpointer             user_data)
{
    SimpleXmlParser *parser = (SimpleXmlParser*)user_data;
    setParserError(parser, error);
    parser->error(error->code, error->message);
}

void setParserError(SimpleXmlParser *parser, GError *error)
{
    parser->error_ = error;
}

static GMarkupParser subparser =
{
    start_element,
    end_element,
    text,
    passthrough,
    error
};

SimpleXmlParser::SimpleXmlParser()
    : context_(NULL),
      error_(NULL)
{
    context_ = g_markup_parse_context_new(
        &subparser,
        G_MARKUP_TREAT_CDATA_AS_TEXT,
        this,
        NULL);
}

SimpleXmlParser::~SimpleXmlParser()
{
    g_markup_parse_context_free(context_);
    g_clear_error(&error_);
}

bool SimpleXmlParser::feed(const char *str, int len)
{
    return g_markup_parse_context_parse(context_, str, len, &error_) == TRUE;
}

bool SimpleXmlParser::feed(const char *str)
{
    return feed(str, -1);
}

bool SimpleXmlParser::finish()
{
    return g_markup_parse_context_end_parse(context_, &error_) == TRUE;
}

const char* SimpleXmlParser::getElement()
{
    return g_markup_parse_context_get_element(context_);
}

const char* SimpleXmlParser::getError(int *code)
{
    if (error_ == NULL)
        return NULL;

    if (code != NULL)
        *code = error_->code;

    return error_->message;
}

void SimpleXmlParser::startElement(const char  *elementName,
                                   const char **attributeNames,
                                   const char **attributeValues)
{}

void SimpleXmlParser::endElement(const char *elementName)
{}

void SimpleXmlParser::text(const char *text,
                           size_t textLen)
{}

void SimpleXmlParser::passthrough(const char *text,
                                  size_t textLen)
{}

void SimpleXmlParser::error(int err, const char *errorstr)
{}

}
