#include "Dependencies.h"

#include <LibCore/include/CoreLocale.h>

#include <LibXML/include/XMLParser.h>

#include "MouCaCore/include/CoreSystem.h"

namespace MouCaCore
{

CoreSystem::CoreSystem():
_resourceManager(std::make_shared<ResourceManager>()), _locale(std::make_shared<Core::Locale>()),
_xmlPlatform(std::make_unique<XML::XercesPlatform>())
{
	// Configure
    _errorManager.setConfiguration(_locale);
}

void CoreSystem::printException(const Core::Exception& exception) const
{
	_errorManager.show(exception);
}

}