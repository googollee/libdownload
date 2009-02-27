#include "utility/SimpleXmlParser.h"

#include <gtest/gtest.h>

#include <sstream>

using namespace std;

class MyXmlParser : public SimpleXmlParser
{
private:
    void startElement(const char  *elementName,
                      const char **attributeNames,
                      const char **attributeValues)
        {
            out << "enter element \"" << elementName << "\"";
            if (attributeNames[0] != NULL)
            {
                out << " with ";
                int i = 0;
                while (attributeNames[i] != NULL)
                {
                    out << attributeNames[i] << "=" << attributeValues[i];
                    ++i;
                }
            }
            out << "\n";
        }

    void endElement(const char *elementName)
        {
            out << "exit element \"" << elementName << "\"\n";
        }

    void text(const char *text,
              size_t textLen)
        {
            string data(text, textLen);
            out << "text in " << getElement() << ": " << data << "\n";;
        }

    void passthrough(const char *text,
                     size_t textLen)
        {
            string data(text, textLen);
            out << "passthrought in "
                << ( (getElement() == NULL) ? "(null)" : getElement() )
                << ": " << data << "\n";;
        }

    void error(int err, const char *errorstr)
        {
            out << "error: " << errorstr << "\n";
        }

public:
    ostringstream out;
};

TEST(SimpleXmlParserTest, Parser)
{
    MyXmlParser parser;
    string data = "<hello/>\n<somedata>\n123\n</somedata><!--comment-->"
        "<withattr key='value'>hei<hello>aaa</hello>hei</withattr>";

    parser.feed(data.c_str(), -1);
    parser.finish();

    string result =
        "enter element \"hello\"\n"
        "exit element \"hello\"\n"
        "enter element \"somedata\"\n"
        "text in somedata: \n"
        "123\n"
        "\n"
        "exit element \"somedata\"\n"
        "passthrought in (null): <!--comment-->\n"
        "enter element \"withattr\" with key=value\n"
        "text in withattr: hei\n"
        "enter element \"hello\"\n"
        "text in hello: aaa\n"
        "exit element \"hello\"\n"
        "text in withattr: hei\n"
        "exit element \"withattr\"\n";
    EXPECT_EQ(parser.out.str(), result);
}
