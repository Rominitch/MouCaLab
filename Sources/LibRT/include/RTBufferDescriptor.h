/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

#include <LibRT/include/RTEnum.h>

namespace RT
{
    //----------------------------------------------------------------------------
    /// \brief Describe one component of buffer internal structure.
    /// \code{.cpp}
    ///     ComponentDescriptor component(3, RT::TypeFloat, RT::BufferVertices);
    /// \endcode
    /// \see BufferDescriptor
    class ComponentDescriptor final
    {
        public:
            /// Constructor: produce invalid data.
            ComponentDescriptor():
            _formatType(Type::Unknown), _nbComponents(0), _sizeInByte(0), _componentUsage(ComponentUsage::Anonyme), _isNormalized(false)
            {}
            
            //------------------------------------------------------------------------
            /// \brief  Constructor with initialization. Descriptor will be valid after this.
            /// 
            /// \param[in] nbComponents:    Number of components.
            /// \param[in] formatType:      Format type of data.
            /// \param[in] componentUsage:  Component usage. Default is BufferDefault.
            /// \param[in] bNormalized:     is data will be normalized (floating-point only). Default is false.
            ComponentDescriptor(const uint64_t nbComponents, const Type formatType, const ComponentUsage componentUsage = ComponentUsage::Anonyme, const bool bNormalized = false);

            /// Copy constructor
            ComponentDescriptor(const ComponentDescriptor& copy) = default;

            ~ComponentDescriptor() = default;

            //------------------------------------------------------------------------
            /// \brief  Get memory size in byte of this descriptor.
            /// 
            /// \returns Memory size in byte.
            uint64_t getSizeInByte() const
            {
                return _sizeInByte;
            }

            //------------------------------------------------------------------------
            /// \brief  Get number of components setup in this descriptor.
            /// 
            /// \returns Number of component (typically between 1 to 4).
            uint64_t getNbComponents() const
            {
                return _nbComponents;
            }

            //------------------------------------------------------------------------
            /// \brief  Get format type of data like (UnsignedInt, char, ...).
            /// 
            /// \returns RT format type.
            /// \see RT::FormatType
            Type getFormatType() const
            {
                return _formatType;
            }

            //------------------------------------------------------------------------
            /// \brief  Get component usage (vertex, color, ... data).
            /// 
            /// \returns Component usage
            /// \see RT::ComponentUsage
            ComponentUsage getComponentUsage() const
            {
                return _componentUsage;
            }

            //------------------------------------------------------------------------
            /// \brief  Is current data must be normalized (floating-point ONLY).
            /// 
            /// \returns True if data are normalized, otherwise false.
            bool isNormalized() const
            {
                return _isNormalized;
            }

        private:
            uint64_t		_nbComponents;          ///< Number of data which compose component.
            Type		_formatType;            ///< Format of descriptor.
            ComponentUsage	_componentUsage;        ///< Usage of this component (vertex, color, texCoord data).
            uint64_t		_sizeInByte;            ///< [COMPUTED] Size in byte of this descriptor.
            bool			_isNormalized;          ///< Is component normalize: only for float -> must be between 0 and 1
    };

    //----------------------------------------------------------------------------
    /// \brief Complex descriptor in order to describe internal buffer structure for rendering pipeline.
    /// \code{.cpp}
    ///     const std::vector<RT::ComponentDescriptor> descriptors =
    ///     {
    ///         { 3, RT::TypeFloat, RT::BufferVertices  },
    ///         { 1, RT::TypeFloat, RT::BufferTexCoord0 }
    ///     };
    ///     RT::BufferDescriptor bufferDescriptor;
    ///     bufferDescriptor.initialize(descriptors);
    /// \endcode
    /// \see ComponentDescriptor
    class BufferDescriptor final
    {
        using DescriptorArray = std::vector< ComponentDescriptor >;

        public:
            /// Default constructor: need to call initialize after.
            BufferDescriptor():
            _totalSizeInByte(0)
            {}

            BufferDescriptor(const BufferDescriptor& copy):
            _descriptors(copy._descriptors), _totalSizeInByte(copy._totalSizeInByte)
            {}

            /// Void memory descriptor
            BufferDescriptor(const size_t memorySize) :
            _descriptors({ ComponentDescriptor(memorySize, RT::Type::Char) }), _totalSizeInByte(memorySize)
            {
                MouCa::preCondition(_totalSizeInByte > 0);
            }

            /// Void memory descriptor
            BufferDescriptor(const ComponentDescriptor& descriptor) :
            _descriptors({ descriptor }), _totalSizeInByte(descriptor.getSizeInByte())
            {
                MouCa::preCondition(_totalSizeInByte > 0);
            }

            ~BufferDescriptor() = default;

            void initialize(const DescriptorArray& descriptorArray)
            {
                MouCa::preCondition(!descriptorArray.empty());
                release();

                //Copy
                _descriptors = descriptorArray;

                for(const auto& descriptor : _descriptors)
                {
                    _totalSizeInByte += descriptor.getSizeInByte();
                }
                MouCa::postCondition(!_descriptors.empty());
                MouCa::postCondition(_totalSizeInByte > 0);
            }

            void initialize(const size_t szNbDescriptors, const size_t* pNbComponentsArray, const Type* pFormatTypeArray, const ComponentUsage* pComponentTypeArray, const bool* pNormalized)
            {
                MouCa::preCondition(pNbComponentsArray != nullptr);
                MouCa::preCondition(pFormatTypeArray != nullptr);
                MouCa::preCondition(pComponentTypeArray != nullptr);
                MouCa::preCondition(szNbDescriptors > 0);

                release();

                //Reserve memory
                _descriptors.reserve(szNbDescriptors);

                //Copy data
                for(size_t szDescriptor = 0; szDescriptor < szNbDescriptors; ++szDescriptor)
                {
                    addDescriptor(ComponentDescriptor(pNbComponentsArray[szDescriptor], pFormatTypeArray[szDescriptor], pComponentTypeArray[szDescriptor], pNormalized[szDescriptor]));
                }

                MouCa::postCondition(!_descriptors.empty());
                MouCa::postCondition(_totalSizeInByte > 0);
            }

            void addDescriptor(const ComponentDescriptor& descriptor)
            {
                _descriptors.push_back(descriptor);

                _totalSizeInByte += descriptor.getSizeInByte();
            }

            uint64_t getByteSize() const
            {
                return _totalSizeInByte;
            }

            void release()
            {
                _descriptors.clear();
                _totalSizeInByte = 0;
            }

            const DescriptorArray& getDescriptors() const { return _descriptors; }

            const ComponentDescriptor& getComponentDescriptor(const size_t szDescriptor) const
            {
                MouCa::assertion(szDescriptor < _descriptors.size());
                return _descriptors[szDescriptor];
            }

            uint64_t getNbDescriptors() const
            {
                return _descriptors.size();
            }

            //------------------------------------------------------------------------
            /// \brief  Search if component usage is present inside BufferDescriptor.
            /// 
            /// \param[in] usage: usage to search.
            /// \returns True if usage exists, otherwise false.
            bool hasComponentUsage(const ComponentUsage usage) const
            {
                return std::find_if(_descriptors.cbegin(), _descriptors.cend(),
                                    [&](const ComponentDescriptor& component)
                                    {
                                        return component.getComponentUsage() == usage;
                                    }) != _descriptors.cend();
            }

            void operator<<(const ComponentDescriptor& descriptor)
            {
                addDescriptor(descriptor);
            }

        protected:
            DescriptorArray _descriptors;       ///< All descriptor for one vertex.
            uint64_t		_totalSizeInByte;   ///< Total size of one vertex in byte.
    };
}