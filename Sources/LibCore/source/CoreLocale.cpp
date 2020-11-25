/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibCore/include/CoreLocale.h>

namespace Core
{
    const String Locale::defaultLanguage(u8"fr");
	const String Locale::defaultCountry(u8"FR");

Locale::Locale():
_language(defaultLanguage), _country(defaultCountry)
{
#if defined MOUCA_OS_WINDOWS

	Core::StringOS localeName;
	localeName.resize(LOCALE_NAME_MAX_LENGTH);
	
    if (GetUserDefaultLocaleName(localeName.data(), static_cast<int>(localeName.size())) > 0)
	{
		std::vector<Core::String> results;
		boost::split(results, Core::convertToU8(localeName), [](const auto& c) {return c == u8'-'; });
		BT_ASSERT(results.size() >= 2);

		_language = results[0];
		_country  = results[1];
    }
#else
/*
// OSX
CFLocaleRef cflocale = CFLocaleCopyCurrent();
CFStringRef value = (CFStringRef)CFLocaleGetValue(cflocale, kCFLocaleLanguageCode);
std::string str(CFStringGetCStringPtr(value, kCFStringEncodingUTF8));
CFRelease(cflocale);

*/
#	error	Need to implement Read OS language
#endif
}

}