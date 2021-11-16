/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibXML/include/XML.h>

namespace MouCaGraphic
{
    class LoaderHelper
    {
    public:
        // Convert string to enum based on real integer or custom string or many using |
        // Supported Format:
        //   Integer number: aspectMask = "6"
        //   LiteralFormat:  aspectMask = "VK_IMAGE_ASPECT_DEPTH_BIT"
        //         or with | aspectMask = "VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT"
        template<typename VulkanEnum>
        static VulkanEnum readValue(XML::NodeUPtr& node, const Core::String& attribute, const std::map<Core::String, VulkanEnum>& literalEnum, bool bitMode, Engine3DXMLLoader::ContextLoading& context)
        {
            Core::String value;
            node->getAttribute(attribute, value);

            if (value.empty())
            {
                throw Core::Exception(Core::ErrorData("Engine3D", "XMLEmptyAttributeStringError") << context.getFileName() << attribute);
            }

            auto data = static_cast<VulkanEnum>(0); // Set to 0 to make | operator
            try
            {
                // Here we don't verify if integer exists: can be a combination or future value.
                // Note: Vulkan enum class would be fine (when possible).
                data = static_cast<VulkanEnum>(std::stoll(value));
            }
            catch (...)
            {
                // Search if variable
                if (value[0] == '$')
                {
                    if (context._globalData == nullptr)
                    {
                        throw Core::Exception(Core::ErrorData("Engine3D", "XMLMissingGlobalData") << context.getFileName());
                    }

                    // Search global data
                    auto result = context._parser.searchNodeFrom(*context._globalData, "Data", "name", value.substr(1));
                    // Make same job on another node
                    data = readValue(result, attribute, literalEnum, bitMode, context);
                }
                else
                {
                    std::vector<Core::String> results;
                    // Possibly | inside
                    if (bitMode)
                    {
                        boost::split(results, value, [](const auto& c) {return c == '|'; });
                    }
                    else
                    {
                        results.emplace_back(value);
                    }
                    for (const auto& part : results)
                    {
                        auto itLiteral = literalEnum.find(part);
                        if (itLiteral == literalEnum.cend())
                        {
                            throw Core::Exception(Core::ErrorData("Engine3D", "XMLUnknownAttributeStringError") << context.getFileName() << attribute << part);
                        }
                        data = static_cast<VulkanEnum>(data | itLiteral->second);
                    }
                }
            }
            return data;
        }

        template<typename DataType>
        static void readAttribute(const XML::NodeUPtr& node, const Core::String& attribute, DataType& value, Engine3DXMLLoader::ContextLoading& context)
        {
            Core::String data;
            node->getAttribute(attribute, data);

            if (data[0] == '$')
            {
                if (context._globalData == nullptr)
                {
                    throw Core::Exception(Core::ErrorData("Engine3D", "XMLMissingGlobalData") << context.getFileName());
                }

                // Search global data
                auto result = context._parser.searchNodeFrom(*context._globalData, "Data", "name", data.substr(1));
                // Make same job on another node
                result->getAttribute(attribute, value);
            }
            else
                node->getAttribute(attribute, value);
        }

        template<typename DataType>
        static void readData(const XML::NodeUPtr& node, DataType& value)
        {
            throw Core::Exception(Core::ErrorData("BasicError", "DevImplementationMissing"));
        }

        template<typename VulkanTypeWeak>
        static uint32_t getIdentifiant(const XML::NodeUPtr& node, const Core::String& nodeName, const std::map<uint32_t, VulkanTypeWeak>& container, Engine3DXMLLoader::ContextLoading& context, bool& existing)
        {
            uint32_t id = 0;
            if (node->hasAttribute("externalId"))
            {
                existing = true;
                node->getAttribute("externalId", id);
                if (container.find(id) == container.cend()) //DEV Issue: Must not exist
                {
                    throw Core::Exception(Core::ErrorData("Engine3D", "XMLUnknownIdentifiantError") << context.getFileName() << nodeName << ":externalId" << std::to_string(id));
                }
            }
            else
            {
                existing = false;
                node->getAttribute("id", id);
                if (container.find(id) != container.cend()) //DEV Issue: Must not exist
                {
                    throw Core::Exception(Core::ErrorData("Engine3D", "XMLAlreadyExistingIdError") << context.getFileName() << std::to_string(id) << "id");
                }
            }
            return id;
        }

        template<typename VulkanTypeWeak>
        static uint32_t getLinkedIdentifiant(const XML::NodeUPtr& node, const Core::String& idName, const std::map<uint32_t, VulkanTypeWeak>& container, Engine3DXMLLoader::ContextLoading& context)
        {
            uint32_t id = 0;
            node->getAttribute(idName, id);
            if (container.find(id) == container.cend()) //DEV Issue: Must exist
            {
                throw Core::Exception(Core::ErrorData("Engine3D", "XMLUnknownIdentifiantError") << context.getFileName() << idName << std::to_string(id));
            }
            return id;
        }

        static void createCharExtensions(std::vector<Core::String>& extensions, std::vector<const char*>& cExtensions)
        {
            // Need to convert list (MUST keep old to get valid memory)
            cExtensions.resize(extensions.size());
            auto cExt = cExtensions.begin();
            for (auto& ext : extensions)
            {
                *cExt = ext.c_str();
                ++cExt;
            }
        }
    };
}