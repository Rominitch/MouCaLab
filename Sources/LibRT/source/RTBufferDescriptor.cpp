/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibRT/include/RTBufferDescriptor.h>

namespace RT
{

ComponentDescriptor::ComponentDescriptor(const uint64_t nbComponents, const Type formatType, const ComponentUsage componentUsage, const bool bNormalized) :
_formatType(formatType), _nbComponents(nbComponents), _sizeInByte(0), _componentUsage(componentUsage), _isNormalized(bNormalized)
{
    //Check error
    MouCa::preCondition(nbComponents > 0);
    MouCa::preCondition(formatType < Type::NbTypes);
    MouCa::preCondition(componentUsage < ComponentUsage::NbUsages);

    //Compute Size of descriptor
    _sizeInByte = computeSizeOf(_formatType) * _nbComponents;
}

}