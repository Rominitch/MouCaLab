#include "Dependencies.h"

#include <LibRT/include/RTImage.h>

#include <MouCaCore/include/CoreSystem.h>
#include <MouCaCore/include/LoaderManager.h>
#include <MouCaCore/include/ResourceManager.h>

namespace Media
{

TEST(Image, compare)
{
    auto core = std::make_shared<MouCaCore::CoreSystem>();

    auto& resources = core->getResourceManager();

    const auto texturePath = MouCaEnvironment::getInputPath() / L"textures";

    auto imageRef       = resources.openImage(texturePath / L"image.png");
    auto image          = resources.openImage(texturePath / L"spir.png");
    auto imageDefault   = resources.openImage(texturePath / L"imageDefaut.png");
    
    // Load
    {
        auto& loader = core->getLoaderManager();
        loader.initialize();
        MouCaCore::LoadingItems items =
        {
            MouCaCore::LoadingItem(imageRef,        MouCaCore::LoadingItem::Deferred),
            MouCaCore::LoadingItem(image,           MouCaCore::LoadingItem::Deferred),
            MouCaCore::LoadingItem(imageDefault,    MouCaCore::LoadingItem::Deferred)
        };
        ASSERT_NO_THROW(loader.loadResources(items));

        ASSERT_NO_THROW(loader.synchronize());
        ASSERT_NO_THROW(loader.release());
    }

    EXPECT_FALSE(image->getImage().lock()->compare(*imageRef->getImage().lock(), 0, 0.0));

    EXPECT_TRUE(image->getImage().lock()->compare(*image->getImage().lock(), 0, 0.0));

    EXPECT_FALSE(imageRef->getImage().lock()->compare(*imageDefault->getImage().lock(), 0, 0.0));
    EXPECT_TRUE(imageRef->getImage().lock()->compare(*imageDefault->getImage().lock(),  0, 114.0));
    EXPECT_TRUE(imageRef->getImage().lock()->compare(*imageDefault->getImage().lock(), 474, 0.0));

    ASSERT_NO_THROW(resources.releaseResource(image));
    ASSERT_NO_THROW(resources.releaseResource(imageRef));
    ASSERT_NO_THROW(resources.releaseResource(imageDefault));
}

// cppcheck-suppress syntaxError
TEST(Image, open)
{
    auto core = std::make_shared<MouCaCore::CoreSystem>();

    auto& loader = core->getLoaderManager();
    loader.initialize();

    MouCaCore::ResourceManager* resources = nullptr;
    ASSERT_NO_THROW(resources = &core->getResourceManager());

    
    const auto texturePath = MouCaEnvironment::getInputPath() / L"textures";
    // Mode unknown: TXT
    {
        auto image = resources->openImage(texturePath / L"spir.txt");
        MouCaCore::LoadingItems items =
        {
            MouCaCore::LoadingItem(image,        MouCaCore::LoadingItem::Direct)
        };
        ASSERT_NO_THROW(loader.loadResources(items));

        EXPECT_TRUE(image->getImage().expired()); // No data loaded
        ASSERT_NO_THROW(resources->releaseResource(image));
    }

    // Mode libImage: PNG
    {
        auto image = resources->openImage(texturePath / L"spir.png");

        MouCaCore::LoadingItems items =
        {
            MouCaCore::LoadingItem(image,        MouCaCore::LoadingItem::Direct)
        };
        ASSERT_NO_THROW(loader.loadResources(items));
        EXPECT_FALSE(image->isNull());

        EXPECT_NO_THROW(image->release());
        EXPECT_TRUE(image->isNull());
        ASSERT_NO_THROW(resources->releaseResource(image));
    }

    // Mode OpenGL: KTX
    {
        auto image = resources->openImage(texturePath / L"spir.ktx");

        MouCaCore::LoadingItems items =
        {
            MouCaCore::LoadingItem(image,        MouCaCore::LoadingItem::Direct)
        };
        ASSERT_NO_THROW(loader.loadResources(items));
        EXPECT_FALSE(image->isNull());

        EXPECT_NO_THROW(image->release());
        EXPECT_TRUE(image->isNull());
        ASSERT_NO_THROW(resources->releaseResource(image));
    }

    ASSERT_NO_THROW(loader.release());
}

}