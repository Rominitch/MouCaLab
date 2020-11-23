/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTBufferDescriptor.h>

namespace RT
{
    class ContextExecution;

    class MemoryAreaBase
    {
        public:
            using HandleBuffer = void*;

            MemoryAreaBase(const Core::String& label=u8"Default Buffer"):
            _handle(nullptr), _nbElements(0), _label(label)
            {}

            virtual ~MemoryAreaBase() = default;

            void setLabel(const Core::String& label)
            {
                _label = label;
            }

            size_t getNbElements() const
            {
                return _nbElements;
            }

        protected:
            HandleBuffer    _handle;			    ///< 64bits Memory to store ID, real pointer, ...
            size_t		    _nbElements;		    ///< Number of item inside handleBuffer. Not real buffer size.
            Core::String	_label;			        ///< Name of data.

        private:
            MOUCA_NOCOPY_NOMOVE(MemoryAreaBase);
    };

    class BufferBase : public MemoryAreaBase
    {
        public:
            BufferBase(const Core::String& label):
            MemoryAreaBase(label)
            {}

            ~BufferBase() override = default;

            const BufferDescriptor& getDescriptor() const
            {
                return _descriptor;
            }

            size_t getByteSize() const
            {
                return _descriptor.getByteSize() * _nbElements;
            }

        protected:
            BufferDescriptor _descriptor;
    };
}