#pragma once

#include <LibXML/include/XML.h>

namespace XML
{
    using DOMElement = xercesc::DOMElement;

    class XercesString
    {
        public:
            explicit XercesString(const Core::StringView& unicode) :
            _xmlChar(xercesc::XMLString::transcode(unicode.data()))
            {
                MouCa::preCondition(_xmlChar != NULL);
            }

            ~XercesString()
            {
                xercesc::XMLString::release(&_xmlChar);
            }

            Core::String toString() const
            {
                MouCa::preCondition(_xmlChar!=NULL);
                return Core::String(xercesc::XMLString::transcode(_xmlChar));
            }

            const XMLCh* toXMLChar() const
            {
                MouCa::preCondition(_xmlChar!=NULL);
                return _xmlChar;
            }

            static Core::String toString(const XMLCh* xmlChar)
            {
                return Core::String(xercesc::XMLString::transcode(xmlChar));
            }

        private:
            XMLCh* _xmlChar;
    };

    class XercesNode final : public Node
    {
        public:
            explicit XercesNode(DOMElement* node=nullptr):
            Node(), _node(node)
            {}

            ~XercesNode() override = default;

            bool isNull() const override
            {
                return _node == nullptr;
            }

            DOMElement* getNode() const
            {
                return _node;
            }

            void getValue(Core::String& value) const override
            {
                MouCa::preCondition(!isNull());
                value = Core::String(xercesc::XMLString::transcode(_node->getFirstChild()->getNodeValue()));
            }

            bool hasAttribute(const Core::StringView& label) const override;

            void getAttribute(const Core::StringView& label, bool& value) const override;
            void getAttribute(const Core::StringView& label, Core::String& value) const override;
            void getAttribute(const Core::StringView& label, uint32_t& value) const override;
            void getAttribute(const Core::StringView& label, int32_t&  value) const override;
            void getAttribute(const Core::StringView& label, uint64_t& value) const override;
            void getAttribute(const Core::StringView& label, int64_t&  value) const override;
            void getAttribute(const Core::StringView& label, float&    value) const override;

        private:
            DOMElement* _node;
    };
}