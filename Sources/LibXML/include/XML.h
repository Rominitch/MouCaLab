/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/CoreResource.h>

namespace XML
{
    class Platform
    {
        public:
            virtual ~Platform() = default;

        protected:
            Platform() = default;
    };

    using PlatformUPtr = std::unique_ptr<Platform>;

    class Node
    {
        public:
            Node() = default;
            virtual ~Node() = default;

            virtual void getValue(Core::String& value) const = 0;

            virtual bool isNull() const = 0;

            virtual bool hasAttribute(const Core::StringView& label) const = 0;

            virtual void getAttribute(const Core::StringView& label, bool& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, Core::String& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, uint32_t& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, int32_t& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, uint64_t& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, int64_t& value) const = 0;
            virtual void getAttribute(const Core::StringView& label, float& value) const = 0;
    };
    using NodeUPtr = std::unique_ptr<Node>;

    class Result
    {
        MOUCA_NOCOPY_NOMOVE(Result);
        public:
            Result() = default;
            virtual ~Result() = default;

            virtual size_t getNbElements() const = 0;

            virtual NodeUPtr getNode(const size_t szNode) const = 0;
    };
    using ResultUPtr = std::unique_ptr<Result>;

    //----------------------------------------------------------------------------
    /// \brief Abstract XML Parser based on DOM.
    class Parser : public Core::Resource
    {
        MOUCA_NOCOPY_NOMOVE(Parser);
        public:
            /// Auto clear parser: make pop when delete
            /// Simply exception error during parsing.
            struct AutoPop
            {
                /// Build auto pop.
                /// \note Prefer to use autoPushNode() to avoid create by yourself.
                AutoPop(Parser& parser):
                _parser(parser)
                {}
                /// Destructor
                ~AutoPop()
                {
                    _parser.popNode();
                }

                private:
                    MOUCA_NOCOPY_NOMOVE(AutoPop);
                    Parser&  _parser;   ///< Link to parser
            };

            /// Destructor
            ~Parser() override = default;

            virtual void openXMLFile(const Core::Path& filePath = Core::Path()) = 0;

        //-----------------------------------------------------------------------------------------
        //                                  Parsing with stack
        //-----------------------------------------------------------------------------------------
            virtual ResultUPtr getNode(const Core::StringView& strName) const = 0;

            virtual AutoPop autoPushNode(const Node& node) = 0;

            virtual void pushNode(const Node& node) = 0;
            virtual void popNode() = 0;

            virtual NodeUPtr searchNode(const Core::StringView& strNodeLabel, const Core::StringView& strParameterLabel, const Core::String& strValue) const = 0;
        //-----------------------------------------------------------------------------------------
        //                                 Parsing without stack
        //-----------------------------------------------------------------------------------------
            virtual ResultUPtr  getNodeFrom(const Node& node, const Core::String& strName) const = 0;
            virtual NodeUPtr    searchNodeFrom(const Node& node, const Core::String& nodeLabel, const Core::String& parameterLabel, const Core::String& strValue) const = 0;

        protected:
            /// Build empty parser
            Parser() = default;
    };

    using ParserSPtr = std::shared_ptr<Parser>;
}