#pragma once

#include <LibBasicTools/include/BTObserver.h>
#include <LibBasicTools/include/BTResource.h>

namespace Vulkan
{
    class ShaderObserver
    {
        public:
            virtual ~ShaderObserver() = default;

            /// 
            virtual void disableRendering(ShaderFile&) = 0;

            virtual void needRefresh(ShaderFile&) = 0;
    };

    class ShaderFile : public BT::Resource,
                       public BT::Observable<ShaderObserver>
    {
        public:
            //------------------------------------------------------------------------
            /// \brief  Build a shader file with or without auto-reload system if can reload file.
            /// 
            /// \param[in] compiledPath: main file for shader (compiled in SPIR-V).
            /// \param[in] filepath: source file and compilable (not mandatory but need to reload shader).
            ShaderFile(const BT::StringOS& compiledPath, const BT::StringOS& filepath = BT::StringOS());

            /// Destructor
            ~ShaderFile() override = default;

            bool isNull() const override
            {
                return _filename.empty();
            }

            //------------------------------------------------------------------------
            /// \brief  
            /// 
            /// \throw BT::Exception when impossible to compile.
            ///                      when impossible to make operation (no file provide).
            void compile();

        private:
            void release() override
            {
                _filename.clear();
            }

            const BT::StringOS _sourceFilepath;      ///< Readable shader file.
    };

}