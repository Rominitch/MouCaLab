#include "Dependencies.h"

#include <LibError/include/Error.h>
#include <LibError/include/ErrorPrinter.h>

#include <MouCaCore/include/CoreSystem.h>
//#include <MouCaCore/interface/LoaderManager.h>
//#include <MouCaCore/interface/ResourceManager.h>
//#include <MouCaCore/interface/ThreadPools.h>

TEST(MouCaCore, getManager)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();

    ASSERT_NO_THROW(core->getResourceManager());

    ASSERT_NO_THROW(core->getLoaderManager());

    ASSERT_NO_THROW(core->getThreadPools());

    ASSERT_NO_THROW(core->getPlugInManager());

    ASSERT_NO_THROW(core->getExceptionManager());
}

class Output : public Core::ErrorPrinter
{
    public:
        Output(void) = default;
        ~Output(void) override = default;

        void print(const Core::ErrorData& Error, const Core::ErrorLibrary*) const override
        {
            std::cout << Error.getErrorLabel() <<std::endl;
        }
};

TEST(MouCaCore, printError)
{
    auto output = std::make_shared<Output>();

    auto core = std::make_unique<MouCaCore::CoreSystem>();
    ASSERT_NO_THROW(core->getExceptionManager().setPrinter(output));

    ASSERT_NO_THROW(core->printException(Core::Exception(Core::ErrorData("Test", "Test", __FILE__, __LINE__))));
}