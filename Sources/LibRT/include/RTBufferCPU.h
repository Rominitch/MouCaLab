/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTCopy.h>
#include <LibRT/include/RTBuffer.h>

namespace RT
{
    class BufferCPUBase : public BufferBase
    {
        public:
            virtual ~BufferCPUBase() = default;

            virtual void release()=0;
            virtual const HandleBuffer lock()=0;
            virtual const void unlock()=0;
            virtual const HandleBuffer getData() const
            {
                return _handle;
            }

            virtual uint64_t getPitchRow() const
            {
                return 0;
            }

            bool isNull() const
            {
                return _handle == nullptr;
            }

        protected:
            BufferCPUBase(const Core::String& strLabel):
            BufferBase(strLabel)
            {}

        private:
            MOUCA_NOCOPY_NOMOVE(BufferCPUBase);
    };

    class BufferLinkedCPU : public BufferCPUBase
    {
    private:
        uint64_t _pitchRow;

    public:
        BufferLinkedCPU(const Core::String& strLabel = "CPU Linked Buffer"):
        BufferCPUBase(strLabel), _pitchRow(0)
        {}

        ~BufferLinkedCPU() override = default;

        void create(const RT::BufferDescriptor& descriptor, const size_t szNbElementBuffer, const HandleBuffer linkedPointer, const size_t pitch = 0)
        {
            MouCa::preCondition(_handle == nullptr);
            MouCa::preCondition(szNbElementBuffer > 0);
            MouCa::preCondition(linkedPointer != nullptr);
            MouCa::preCondition(descriptor.getByteSize() > 0);

            _handle     = linkedPointer;
            _nbElements = szNbElementBuffer;
            _descriptor = descriptor;
            _pitchRow   = pitch;

            MouCa::postCondition(_handle != nullptr);
            MouCa::postCondition(&static_cast<char*>(_handle)[(szNbElementBuffer-1) * descriptor.getByteSize()] != nullptr); //DEV Issue: access to last elements.
        }

        void release() override
        {
            _handle = nullptr;
        }

        const HandleBuffer lock() override
        {
            return _handle;
        }

        const void unlock() override
        {}

        uint64_t getPitchRow() const override
        {
            return _pitchRow;
        }

        template<typename DataType>
        DataType* lockData()
        {
            return reinterpret_cast<DataType*>(_handle);
        }
    };

    class BufferCPU : public BufferCPUBase
    {
        public:
            BufferCPU(const Core::String& strLabel = "CPU Buffer") :
            BufferCPUBase( strLabel )
            {}

            ~BufferCPU() override
            {
                release();
            }

            virtual HandleBuffer create(const RT::BufferDescriptor& descriptor, const size_t nbElementsBuffer)
            {
                release();

                MouCa::assertion( descriptor.getByteSize() > 0 );
                MouCa::assertion( nbElementsBuffer > 0 );

                _nbElements	= nbElementsBuffer;
                _descriptor	= descriptor;

                //Create buffer
                _handle = new uint8_t[getByteSize()];

                return _handle;
            }

            void release() override
            {
                delete[] _handle;
                _handle=nullptr;
            }

            const HandleBuffer getData() const override
            {
                return _handle;
            }

            const HandleBuffer lock() override
            {
                return _handle;
            }

            const void unlock() override
            {}

            template<typename DataType>
            DataType* lockData()
            {
                return reinterpret_cast<DataType*>(_handle);
            }
    };

    // Declaration
    using BufferCPUSPtr = std::shared_ptr<BufferCPU>;
    using BufferCPUWPtr = std::weak_ptr<BufferCPU>;

    using BufferLinkedCPUSPtr = std::shared_ptr<BufferLinkedCPU>;
    using BufferLinkedCPUWPtr = std::weak_ptr<BufferLinkedCPU>;

    using BufferCPUBaseSPtr = std::shared_ptr<BufferCPUBase>;
    using BufferCPUBaseWPtr = std::weak_ptr<BufferCPUBase>;
}