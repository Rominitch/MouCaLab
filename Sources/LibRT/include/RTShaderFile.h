/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibCore/include/BTFile.h>

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Shader SPIR-V resource with possibility to attached real source for automatic update.
    ///
    /// \see   Core::File
    class ShaderFile : public Core::File
    {
        public:
            //------------------------------------------------------------------------
            /// \brief  Build a shader file with or without auto-reload system if can reload file.
            /// 
            /// \param[in] compiledPath: main file for shader (compiled in SPIR-V).
            /// \param[in] filepath: source file and compilable (not mandatory but need to reload shader).
            ShaderFile(const Core::Path& compiledPath, const ShaderKind kind, const Core::Path& filepath = Core::Path());

            /// Destructor
            ~ShaderFile() override;

            //------------------------------------------------------------------------
            /// \brief  Get kind of shader (vertex fragment, ...)
            /// 
            /// \see ShaderKind  
            /// \returns Valid kind of shader
            const ShaderKind getKind() const { return _kind; }

            //------------------------------------------------------------------------
            /// \brief  
            /// 
            /// \throw Core::Exception when impossible to compile.
            ///                      when impossible to make operation (no file provide).
            void compile();

            //------------------------------------------------------------------------
            /// \brief  Get current filename for tracking.
            /// 
            /// \returns Current filename for tracking.
            const Core::Path& getTrackedFilename() const override
            {
                return _sourceFilepath.empty() ? _filename : _sourceFilepath;
            }

        private:
            const ShaderKind _kind;
            const Core::Path _sourceFilepath;      ///< Readable shader file.
    };

}