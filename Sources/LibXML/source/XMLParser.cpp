#include "Dependencies.h"

#include <LibXML/include/XMLParser.h>

namespace XML
{

XercesPlatform::XercesPlatform()
{
    try
    {
        xercesc::XMLPlatformUtils::Initialize();
    }
    catch(const xercesc::XMLException&)
    {}
}

XercesPlatform::~XercesPlatform()
{
    //Release Xerces used
    xercesc::XMLPlatformUtils::Terminate();
}

XercesParser::XercesParser()
{
    MouCa::preCondition(isNull());
}

XercesParser::~XercesParser()
{
    MouCa::preCondition(!isLoaded());
}

void XercesParser::release()
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(_parseStack.empty()); // DEV Issue: you need to clean stack BEFORE call release: bad pair Push/Pop.

    _parser.reset();
    _filename.clear();
}

void XercesParser::openXMLFile(const Core::Path& strFilePath)
{
    MouCa::preCondition(!strFilePath.empty() || !_filename.empty());
    MouCa::preCondition(_parser == nullptr);

    // Use best file (priority to given file then latest register)
    const Core::Path fileXML = !strFilePath.empty() ? strFilePath : _filename;

    try
    {
        _parser = std::make_unique<xercesc::XercesDOMParser>();
    }
    catch(const xercesc::XMLException&)
    {
        throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError") << "_parser"); // DEV Issue: Missing XML::Platform !
    }
    MouCa::assertion(_parser != nullptr);

    //Basic configuration
    _parser->setValidationScheme(xercesc::XercesDOMParser::Val_Never);
    _parser->setDoNamespaces(false);
    _parser->setDoSchema(false);
    _parser->setLoadExternalDTD(false);

    try
    {
        //Parse XML File
        _parser->parse(fileXML.string().c_str());

        if(_parser->getDocument()==nullptr)
        {
            _parser.reset();
            throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError") << "_parser->getDocument()");
        }
    }
    catch(const xercesc::XMLException&)
    {}

    _filename = fileXML;

    MouCa::postCondition(!isNull());
}

ResultUPtr XercesParser::getNode(const Core::StringView& strName) const
{
    MouCa::preCondition(!isNull());

    xercesc::DOMNodeList* nodeList = nullptr;
    if (_parseStack.empty())
    {
        // no need to free this pointer - owned by the parent parser object
        xercesc::DOMDocument* pXMLDoc = _parser->getDocument();
        if (pXMLDoc != nullptr)
        {
            nodeList = pXMLDoc->getElementsByTagName(XercesString(strName).toXMLChar());
        }
        else
        {
            throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError"));
        }
    }
    else
    {
        MouCa::assertion(_parseStack.top() != NULL);

        nodeList = _parseStack.top()->getElementsByTagName(XercesString(strName).toXMLChar());
    }

    if (nodeList == nullptr)
    {
        throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError"));
    }

    auto result = std::make_unique<XercesResult>();
    result->initialize(nullptr, nodeList);
    return result;
}

XercesParser::AutoPop XercesParser::autoPushNode(const Node& node)
{
    pushNode(node);
    return AutoPop(*this);
}

void XercesParser::pushNode(const Node& node)
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(dynamic_cast<const XercesNode*>(&node) !=  nullptr);
    const DOMElement* element = static_cast<const XercesNode*>(&node)->getNode();
    MouCa::preCondition(element != nullptr);

    _parseStack.push(element);
}

void XercesParser::popNode()
{
    MouCa::preCondition(!isNull());
    MouCa::preCondition(!_parseStack.empty());
    _parseStack.pop();
}

NodeUPtr XercesParser::searchNodeGeneric(const ResultUPtr& result, const Core::StringView& parameterLabel, const Core::String& value) const
{
    NodeUPtr node;
    if (result->getNbElements() > 0)
    {
        bool bFind = false;
        size_t szSearch = 0;
        Core::String read;
        do
        {
            //Search attribute
            result->getNode(szSearch)->getAttribute(Core::String(parameterLabel), read);

            //Compare value and continue or quit
            bFind = (value == read);
            if (bFind == false)
            {
                szSearch++;
            }
        } while (bFind == false && szSearch < result->getNbElements());

        //If find data we return node
        if (szSearch < result->getNbElements())
        {
            node = result->getNode(szSearch);
        }
        else
        {
            throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingNodeError") << Core::String(parameterLabel));
        }
    }
    return node;
}

NodeUPtr XercesParser::searchNode(const Core::StringView& strNodeLabel, const Core::StringView& strParameterLabel, const Core::String& strValue) const
{
    MouCa::preCondition(!isNull());

    return searchNodeGeneric(getNode(strNodeLabel), strParameterLabel, strValue);
}

ResultUPtr XercesParser::getNodeFrom(const Node& node, const Core::String& strName) const
{
    MouCa::preCondition(!isNull());

    xercesc::DOMNodeList* nodeList = nullptr;
    if (node.isNull())
    {
        // no need to free this pointer - owned by the parent parser object
        xercesc::DOMDocument* pXMLDoc = _parser->getDocument();
        MouCa::assertion(pXMLDoc != nullptr);
        nodeList = pXMLDoc->getElementsByTagName(XercesString(strName).toXMLChar());
    }
    else
    {
        nodeList = static_cast<const XercesNode*>(&node)->getNode()->getElementsByTagName(XercesString(strName).toXMLChar());
    }

    if (nodeList == nullptr)
    {
        throw Core::Exception(Core::ErrorData("BasicError", "NULLPointerError"));
    }

    auto result = std::make_unique<XercesResult>();
    result->initialize(nullptr, nodeList);
    return result;
}

NodeUPtr XercesParser::searchNodeFrom(const Node& node, const Core::String& nodeLabel, const Core::String& parameterLabel, const Core::String& value) const
{
    MouCa::preCondition(!isNull());

    return searchNodeGeneric(getNodeFrom(node, nodeLabel), parameterLabel, value);
}

}