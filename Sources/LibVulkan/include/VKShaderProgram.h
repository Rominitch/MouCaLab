/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    class File;
}

namespace Vulkan
{
    class Device;

    class ShaderSpecialization
    {
        public:
            using MapEntries = std::vector<VkSpecializationMapEntry>;
            //MOUCA_NOCOPY( ShaderSpecialization );

            ShaderSpecialization() :
                _pointer(nullptr),
                _sizeByte(0),
                _specializationInfo(
            {
                0,          // uint32_t                           mapEntryCount;
                nullptr,    // const VkSpecializationMapEntry*    pMapEntries;
                0,          // size_t                             dataSize;
                nullptr     // const void*                        pData;
            })
            {}

            bool isNull() const
            {
                return _specializationEntries.empty()
                    || _pointer == nullptr;
            }

            const VkSpecializationInfo& getInstance() const
            {
                return _specializationInfo;
            }

            void setMapEntries(MapEntries&& mapEntries)
            {
                _specializationEntries = std::move(mapEntries);

                // Update data
                _specializationInfo.mapEntryCount = static_cast<uint32_t>(_specializationEntries.size());
                _specializationInfo.pMapEntries = _specializationEntries.data();
            }

            void addMapEntry(VkSpecializationMapEntry&& mapEntry)
            {
                _specializationEntries.emplace_back(std::move(mapEntry));

                // Update data
                _specializationInfo.mapEntryCount = static_cast<uint32_t>(_specializationEntries.size());
                _specializationInfo.pMapEntries   = _specializationEntries.data();
            }

            /// Note: Can source of bugs if memory is not consistent.
            void addDataInfo(const void* pointer, const size_t sizeByte)
            {
                MOUCA_PRE_CONDITION(pointer != nullptr);   //Dev Issue: Impossible to send no data!
                MOUCA_PRE_CONDITION(sizeByte > 0);         //Dev Issue: Impossible to send no data!

                _pointer  = pointer;
                _sizeByte = sizeByte;
                _specializationInfo.dataSize = _sizeByte;
                _specializationInfo.pData    = _pointer;
            }
        private:
            std::vector<VkSpecializationMapEntry>   _specializationEntries;
            VkSpecializationInfo                    _specializationInfo;

            const void*   _pointer;
            size_t        _sizeByte;
    };

    class ShaderModule final
    {
        MOUCA_NOCOPY_NOMOVE(ShaderModule);

        public:
            static const std::array<RT::ShaderKind, static_cast<size_t>(RT::ShaderKind::NbShaders)> _allShaderType;

            ShaderModule();
            ~ShaderModule();

            void initialize(const Device& device, const Core::String& shaderSourceFile, const std::string& name, const VkShaderStageFlagBits stage);
            void release(const Device& device);

            bool isNull() const
            {
                return _shaderModule == VK_NULL_HANDLE;
            }

            const VkShaderModule& getInstance() const   { return _shaderModule; }
            VkShaderStageFlagBits getStage() const      { return _stage; }
            const std::string&    getName() const       { return _name;  }

        private:
            VkShaderModule          _shaderModule;         ///< Vulkan Shader instance.
            std::string             _name;
            VkShaderStageFlagBits   _stage;
    };

    using ShaderModuleSPtr = std::shared_ptr<ShaderModule>;
    using ShaderModuleWPtr = std::weak_ptr<ShaderModule>;

    class [[deprecated]] ShaderProgram
    {
        private:
            ShaderSpecialization _specialization;       ///< Specialization.
            VkShaderModule       _shaderModule;         ///< Vulkan Shader instance.
            std::string          _name;

            MOUCA_NOCOPY(ShaderProgram);

        public:
            static const std::array<RT::ShaderKind, static_cast<size_t>(RT::ShaderKind::NbShaders)> _allShaderType;

            ShaderProgram();
            ~ShaderProgram();

            void initialize(const Device& device, const Core::File& shaderSourceFile, const std::string& name);
            void release(const Device& device);

            bool isNull() const
            {
                return _shaderModule == VK_NULL_HANDLE;
            }

            const VkShaderModule& getInstance() const
            {
                return _shaderModule;
            }

            const std::string& getName() const
            {
                return _name;
            }

            const ShaderSpecialization& getSpecialization() const
            {
                return _specialization;
            }

            ShaderSpecialization& getEditSpecialization()
            {
                return _specialization;
            }

            void setSpecialization(ShaderSpecialization& specialization)
            {
                _specialization = std::move(specialization);
            }
    };
}