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
                MOUCA_THROW_ERROR_2(u8"Engine3D", u8"XMLEmptyAttributeStringError", context.getFileName(), attribute);
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
                        MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLMissingGlobalData", context.getFileName());
                    }

                    // Search global data
                    auto result = context._parser.searchNodeFrom(*context._globalData, u8"Data", u8"name", value.substr(1));
                    // Make same job on another node
                    data = readValue(result, attribute, literalEnum, bitMode, context);
                }
                else
                {
                    std::vector<Core::String> results;
                    // Possibly | inside
                    if (bitMode)
                    {
                        boost::split(results, value, [](const auto& c) {return c == u8'|'; });
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
                            MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownAttributeStringError", context.getFileName(), attribute, part);
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
                    MOUCA_THROW_ERROR_1(u8"Engine3D", u8"XMLMissingGlobalData", context.getFileName());
                }

                // Search global data
                auto result = context._parser.searchNodeFrom(*context._globalData, u8"Data", u8"name", data.substr(1));
                // Make same job on another node
                result->getAttribute(attribute, value);
            }
            else
                node->getAttribute(attribute, value);
        }

        template<typename DataType>
        static void readData(const XML::NodeUPtr& node, DataType& value)
        {
            MOUCA_THROW_ERROR(u8"BasicError", u8"DevImplementationMissing");
        }

        template<>
        static void readData(const XML::NodeUPtr& node, VkExtent3D& extent)
        {
            node->getAttribute(u8"width", extent.width);
            node->getAttribute(u8"height", extent.height);
            node->getAttribute(u8"depth", extent.depth);
        }

        template<typename VulkanTypeWeak>
        static uint32_t getIdentifiant(const XML::NodeUPtr& node, const Core::String& nodeName, const std::map<uint32_t, VulkanTypeWeak>& container, Engine3DXMLLoader::ContextLoading& context, bool& existing)
        {
            uint32_t id = 0;
            if (node->hasAttribute(u8"externalId"))
            {
                existing = true;
                node->getAttribute(u8"externalId", id);
                if (container.find(id) == container.cend()) //DEV Issue: Must not exist
                {
                    MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownIdentifiantError", context.getFileName(), nodeName + u8":externalId", std::to_string(id));
                }
            }
            else
            {
                existing = false;
                node->getAttribute(u8"id", id);
                if (container.find(id) != container.cend()) //DEV Issue: Must not exist
                {
                    MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLAlreadyExistingIdError", context.getFileName(), std::to_string(id), u8"id");
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
                MOUCA_THROW_ERROR_3(u8"Engine3D", u8"XMLUnknownIdentifiantError", context.getFileName(), idName, std::to_string(id));
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