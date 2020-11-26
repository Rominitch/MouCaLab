/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibXML/include/XML.h>
#include <LibXML/include/XMLNode.h>

#define XERCES_SPEED_DEBUG
namespace XML
{
    class XercesPlatform final : public Platform
    {
        public:
            XercesPlatform();

            ~XercesPlatform() override;
    };

    class XercesResult final : public Result
    {
        public:
            XercesResult():
            _parent(NULL), _findNodesList(NULL)
            {}

            ~XercesResult() override = default;

            void initialize(xercesc::DOMElement* pParent, xercesc::DOMNodeList* pFindNodesList)
            {
                _parent			= pParent;
                _findNodesList	= pFindNodesList;
            }

            size_t getNbElements() const override
            {
                MOUCA_PRE_CONDITION(_findNodesList!=NULL);
                return _findNodesList->getLength();
            }

            NodeUPtr getNode(const size_t szNode) const override
            {
                MOUCA_PRE_CONDITION(_findNodesList!=NULL);
#ifndef XERCES_SPEED_DEBUG
                MOUCA_PRE_CONDITION(szNode < _findNodesList->getLength());
#endif
                xercesc::DOMNode* pNode = _findNodesList->item(szNode);
#ifndef XERCES_SPEED_DEBUG
                MOUCA_PRE_CONDITION(pNode != NULL && pNode->getNodeType() && pNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE);
#endif

                MOUCA_PRE_CONDITION(dynamic_cast<const DOMElement*>(pNode) != NULL);
                return std::make_unique<XercesNode>(static_cast<DOMElement*>(pNode));
            }
        private:
            DOMElement*		        _parent;
            xercesc::DOMNodeList*	_findNodesList;
            
            MOUCA_NOCOPY_NOMOVE(XercesResult);
    };

    //----------------------------------------------------------------------------
    /// \brief XML Parser based on DOM.
    /// \code{.cpp}
    ///     // Allocate platform: mandatory one time for all in program.
    ///     XML::Platform platform;
    ///
    ///     // Open parser
    ///     XercesParser parser;
    ///     parser.openXMLFile(L"MyFilePath");
    ///
    ///     // Make operation
    ///
    ///     // Clean object
    ///     parser.release();
    /// \endcode
    class XercesParser final : public Parser
    {
        public:
            /// Constructor
            XercesParser();
            /// Destructor
            ~XercesParser() override;

            //------------------------------------------------------------------------
            /// \brief  Check if current instance is clean or not.
            /// 
            /// \returns True if null, false otherwise.
            bool isNull() const override
            {
                return !isLoaded()
                    && _filename.empty();
            }

            bool isLoaded() const override
            {
                return !_parseStack.empty()
                    || _parser != nullptr;
            }
                        
            //------------------------------------------------------------------------
            /// \brief  Open parser with new file.
            /// 
            /// \param[in] strFilePath: filepath to XML file.
            /// \note  Need to release() instance if already use.
            void openXMLFile(const Core::Path& strFilePath = Core::Path()) override;

            //------------------------------------------------------------------------
            /// \brief  Release current parser to clean
            /// 
            /// \returns 
            void release() override;

        //-----------------------------------------------------------------------------------------
        //                                  Parsing with stack
        //-----------------------------------------------------------------------------------------
            ResultUPtr getNode(const Core::String& name) const override;

            AutoPop autoPushNode(const Node& node) override;

            void pushNode(const Node& node) override;
            void popNode() override;

            NodeUPtr searchNode(const Core::String& nodeLabel, const Core::String& parameterLabel, const Core::String& strValue) const override;

        //-----------------------------------------------------------------------------------------
        //                                 Parsing without stack
        //-----------------------------------------------------------------------------------------
            ResultUPtr  getNodeFrom(const Node& node, const Core::String& strName) const override;
            NodeUPtr    searchNodeFrom(const Node& node, const Core::String& nodeLabel, const Core::String& parameterLabel, const Core::String& strValue) const override;

        private:
            std::unique_ptr<xercesc::XercesDOMParser>	_parser;        ///< Parser data to read xml.

            std::stack<const xercesc::DOMElement*>	    _parseStack;    ///< Current stack of node parsing (managed with push/pop).

            NodeUPtr searchNodeGeneric(const ResultUPtr& result, const Core::String& parameterLabel, const Core::String& strValue) const;

            MOUCA_NOCOPY_NOMOVE(XercesParser);
    };
}