#include "Dependancies.h"

#include <LibCore/include/CoreLocale.h>

#include "MouCaCore/include/CoreSystem.h"

namespace MouCaCore
{

CoreSystem::CoreSystem():
_resourceManager(std::make_shared<ResourceManager>()), _locale(std::make_shared<Core::Locale>())
{
	// Configure
    _errorManager.setConfiguration(_locale);
}

void CoreSystem::printException(const Core::Exception& exception) const
{
	_errorManager.show(exception);
}

}