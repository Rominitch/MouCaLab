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
    MOUCA_PRE_CONDITION(isNull());
}

XercesParser::~XercesParser()
{
    MOUCA_PRE_CONDITION(!isLoaded());
}

void XercesParser::release()
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(_parseStack.empty()); // DEV Issue: you need to clean stack BEFORE call release: bad pair Push/Pop.

    _parser.reset();
    _filename.clear();
}

void XercesParser::openXMLFile(const Core::Path& strFilePath)
{
    MOUCA_PRE_CONDITION(!strFilePath.empty() || !_filename.empty());
    MOUCA_PRE_CONDITION(_parser == nullptr);

    // Use best file (priority to given file then latest register)
    const Core::Path fileXML = !strFilePath.empty() ? strFilePath : _filename;

    try
    {
        _parser = std::make_unique<xercesc::XercesDOMParser>();
    }
    catch(const xercesc::XMLException&)
    {
        MOUCA_THROW_ERROR_1("BasicError", "NULLPointerError", "_parser"); // DEV Issue: Missing XML::Platform !
    }
    MOUCA_ASSERT(_parser != nullptr);

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
            MOUCA_THROW_ERROR_1("BasicError", "NULLPointerError", "_parser->getDocument()");
        }
    }
    catch(const xercesc::XMLException&)
    {}

    _filename = fileXML;

    MOUCA_POST_CONDITION(!isNull());
}

ResultUPtr XercesParser::getNode(const Core::String& strName) const
{
    MOUCA_PRE_CONDITION(!isNull());

    xercesc::DOMNodeList* nodeList=nullptr;
    if(_parseStack.empty())
    {
        // no need to free this pointer - owned by the parent parser object
        xercesc::DOMDocument* pXMLDoc = _parser->getDocument();
        if(pXMLDoc!=nullptr)
        {
            nodeList=pXMLDoc->getElementsByTagName(XercesString(strName).toXMLChar());
        }
        else
        {
            MOUCA_THROW_ERROR("BasicError", "NULLPointerError");
        }
    }
    else
    {
        MOUCA_ASSERT(_parseStack.top() != NULL);

        nodeList=_parseStack.top()->getElementsByTagName(XercesString(strName).toXMLChar());
    }

    if(nodeList==nullptr)
    {
        MOUCA_THROW_ERROR( "BasicError", "NULLPointerError" );
    }

    auto result = std::make_unique<XercesResult>();
    result->initialize( nullptr, nodeList );
    return result;
}

XercesParser::AutoPop XercesParser::autoPushNode(const Node& node)
{
    pushNode(node);
    return AutoPop(*this);
}

void XercesParser::pushNode(const Node& node)
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(dynamic_cast<const XercesNode*>(&node) !=  nullptr);
    const DOMElement* element = static_cast<const XercesNode*>(&node)->getNode();
    MOUCA_PRE_CONDITION(element != nullptr);

    _parseStack.push(element);
}

void XercesParser::popNode()
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(!_parseStack.empty());
    _parseStack.pop();
}

NodeUPtr XercesParser::searchNodeGeneric(const ResultUPtr& result, const Core::String& parameterLabel, const Core::String& value) const
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
            result->getNode(szSearch)->getAttribute(parameterLabel, read);

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
            MOUCA_THROW_ERROR_1("XMLError", "XMLMissingNodeError", parameterLabel);
        }
    }
    return node;
}

NodeUPtr XercesParser::searchNode(const Core::String& strNodeLabel, const Core::String& strParameterLabel, const Core::String& strValue) const
{
    MOUCA_PRE_CONDITION(!isNull());

    return searchNodeGeneric(getNode(strNodeLabel), strParameterLabel, strValue);
}

ResultUPtr XercesParser::getNodeFrom(const Node& node, const Core::String& strName) const
{
    MOUCA_PRE_CONDITION(!isNull());

    xercesc::DOMNodeList* nodeList = nullptr;
    if (node.isNull())
    {
        // no need to free this pointer - owned by the parent parser object
        xercesc::DOMDocument* pXMLDoc = _parser->getDocument();
        MOUCA_ASSERT(pXMLDoc != nullptr);
        nodeList = pXMLDoc->getElementsByTagName(XercesString(strName).toXMLChar());
    }
    else
    {
        nodeList = static_cast<const XercesNode*>(&node)->getNode()->getElementsByTagName(XercesString(strName).toXMLChar());
    }

    if (nodeList == nullptr)
    {
        MOUCA_THROW_ERROR("BasicError", "NULLPointerError");
    }

    auto result = std::make_unique<XercesResult>();
    result->initialize(nullptr, nodeList);
    return result;
}

NodeUPtr XercesParser::searchNodeFrom(const Node& node, const Core::String& nodeLabel, const Core::String& parameterLabel, const Core::String& value) const
{
    MOUCA_PRE_CONDITION(!isNull());

    return searchNodeGeneric(getNodeFrom(node, nodeLabel), parameterLabel, value);
}

}