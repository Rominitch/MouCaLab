/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibRT/include/RTBufferDescriptor.h>

namespace RT
{

ComponentDescriptor::ComponentDescriptor(const uint64_t nbComponents, const Type formatType, const ComponentUsage componentUsage, const bool bNormalized) :
_formatType(formatType), _nbComponents(nbComponents), _sizeInByte(0), _componentUsage(componentUsage), _isNormalized(bNormalized)
{
    //Check error
    MOUCA_PRE_CONDITION(nbComponents > 0);
    MOUCA_PRE_CONDITION(formatType < Type::NbTypes);
    MOUCA_PRE_CONDITION(componentUsage < ComponentUsage::NbUsages);

    //Compute Size of descriptor
    _sizeInByte = computeSizeOf(_formatType) * _nbComponents;
}

}