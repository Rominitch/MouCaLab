#include "Dependencies.h"

#include <LibXML/include/XMLNode.h>

//Memory Leak Linker
#ifndef NDEBUG
#   ifdef DEBUG_NEW
#       undef DEBUG_NEW
#   endif
#   define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#   define new DEBUG_NEW
#endif

namespace XML
{

bool XercesNode::hasAttribute(const Core::String& label) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    return (attribute != nullptr);
}

void XercesNode::getAttribute(const Core::String& label, bool& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if (attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }

    Core::String sValue = XercesString::toString(attribute->getValue());
    // Set to lower
    std::transform(sValue.begin(), sValue.end(), sValue.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });
    if(sValue == "true")
    {
        value = true;
    }
    else if(sValue == "false")
    {
        value = false;
    }
    else
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLBooleanBadParameterError") << label);
    }
}

void XercesNode::getAttribute(const Core::String& label, Core::String& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    
    value = XercesString::toString(attribute->getValue());
}

void XercesNode::getAttribute(const Core::String& label, uint32_t& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    value = std::stoul(XercesString::toString(attribute->getValue()));
}

void XercesNode::getAttribute(const Core::String& label, int32_t& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    value = std::stol(XercesString::toString(attribute->getValue()));
}

void XercesNode::getAttribute(const Core::String& label, uint64_t& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    value = std::stoull(XercesString::toString(attribute->getValue()));
}

void XercesNode::getAttribute(const Core::String& label, int64_t&  value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    value = std::stoll(XercesString::toString(attribute->getValue()));
}

void XercesNode::getAttribute(const Core::String& label, float& value) const
{
    MouCa::preCondition(!isNull());

    const xercesc::DOMAttr* attribute = _node->getAttributeNode(XercesString(label).toXMLChar());
    if(attribute == nullptr)
    {
        throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingParameterError") << label);
    }
    value = std::stof(XercesString::toString(attribute->getValue()));
}

}