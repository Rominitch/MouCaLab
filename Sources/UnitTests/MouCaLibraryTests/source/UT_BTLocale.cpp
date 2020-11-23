#include "Dependancies.h"

#include <LibCore/include/BTLocale.h>

TEST(BTLocale, defaultUse)
{
    Core::Locale locale;

    // Default must have something (depends on machine)
    EXPECT_FALSE(locale.getCountry().empty());
    EXPECT_FALSE(locale.getLanguage().empty());

    // Manual edition
    const Core::String english("en");
    const Core::String usa("US");
    ASSERT_NO_THROW(locale.setLanguage(english));
    ASSERT_NO_THROW(locale.setCountry(usa));

    EXPECT_EQ(english, locale.getLanguage());
    EXPECT_EQ(usa,     locale.getCountry());
}