#include "Dependancies.h"

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMesh.h>

#include <MouCaCore/include/CoreSystem.h>
#include <MouCaCore/include/LoaderManager.h>
#include <MouCaCore/include/ResourceManager.h>

#include <MouCaCore/include/LoaderManager.h>

namespace MouCaCore
{

TEST(LoaderManager, SynchonizeData)
{
    LoaderManager::SynchonizeData synchronize;
    
    ASSERT_NO_THROW(synchronize.initialize(6));

    ASSERT_NO_THROW(synchronize.threadWorking(0));
    ASSERT_NO_THROW(synchronize.threadWorking(1));
    ASSERT_NO_THROW(synchronize.threadWorking(5));

    ASSERT_NO_THROW(synchronize.threadReady(0));
    ASSERT_NO_THROW(synchronize.threadReady(1));
    ASSERT_NO_THROW(synchronize.threadReady(2));
    ASSERT_NO_THROW(synchronize.threadReady(5));

    ASSERT_NO_THROW(synchronize.synchronize());
}

// cppcheck-suppress syntaxError
TEST(LoaderManager, create)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();
    
    MouCaCore::LoaderManager& loader = core->getLoaderManager();
    // Prepare manager
    ASSERT_NO_THROW(loader.initialize());

    const auto texturePath = MouCaEnvironment::getInputPath() / L"textures";
    const auto filename0 = texturePath / L"img0.png";
    const auto filename1 = texturePath / L"img1.png";
    const auto filename2 = texturePath / L"img2.png";
    const auto filename3 = texturePath / L"img3.png";
    const auto filename4 = texturePath / L"img4.png";

    // Job to do
    LoadingItems items = 
    {   
        LoadingItem(core->getResourceManager().openImage(filename0), LoadingItem::Direct,   LoadingItem::Low),
        LoadingItem(core->getResourceManager().openImage(filename1), LoadingItem::Direct,   LoadingItem::Urgent),
        LoadingItem(core->getResourceManager().openImage(filename2), LoadingItem::Deferred, LoadingItem::Priority),
        LoadingItem(core->getResourceManager().openImage(filename3), LoadingItem::Deferred, LoadingItem::Normal),
        LoadingItem(core->getResourceManager().openImage(filename4), LoadingItem::Deferred, LoadingItem::Urgent)
    };

    // Loading
    ASSERT_NO_THROW(loader.loadResources(items));

    ASSERT_NO_THROW(loader.synchronize());

    // Release manager
    ASSERT_NO_THROW(loader.release());

    // Clean all resources in manager
    ASSERT_NO_THROW(core->getResourceManager().releaseResources());
}

// DisableCodeCoverage

class LoaderManagerTest : public testing::Test
{
    public:
        static const uint32_t _nbJobs = 1000;
        LoadingItems items;
        MouCaCore::CoreSystemUPtr core;

        void generateJob()
        {
            std::random_device r;
            std::default_random_engine e1(r());

            const auto filename = MouCaEnvironment::getInputPath() / L"mesh" / L"armor"/ L"normalmap.ktx";

            // Generate list of data
            for(uint32_t job = 0; job < _nbJobs; ++job)
            {
                const auto imageName = Core::StringOS(L"img") + Core::convertToOS(std::to_string(job)) + Core::StringOS(L".ktx");
                // Duplicate data
                const auto dest = MouCaEnvironment::getOutputPath() / imageName;
                ASSERT_TRUE( std::filesystem::copy_file( filename, dest, std::filesystem::copy_options::overwrite_existing ) );

                std::uniform_int_distribution<int> modeDistribution(0, 60);
                int mode = modeDistribution(e1);
                std::uniform_int_distribution<int> orderDistribution(0, 4);
                int order = orderDistribution(e1);
                if(mode != 0)
                    mode = 1;

                auto image = MouCaEnvironment::getOutputPath() / imageName;
                items.emplace_back(core->getResourceManager().openImage( image ), static_cast<LoadingItem::State>(mode), static_cast<LoadingItem::Order>(order));
            }
        }

        void SetUp() override
        {
            MOUCA_PRE_CONDITION(items.empty());

            core = std::make_unique<MouCaCore::CoreSystem>();
            generateJob();

            MOUCA_POST_CONDITION(!items.empty());
        }

        void TearDown() override
        {
            MOUCA_PRE_CONDITION(items.empty());

            // Control resource loading
            MOUCA_PRE_CONDITION(core->getResourceManager().getResources().size() == _nbJobs);
            for(const auto& resource : core->getResourceManager().getResources())
            {
                ASSERT_FALSE(resource->isNull());
            }

            // Clean all resources in manager
            ASSERT_NO_THROW(core->getResourceManager().releaseResources());
        }
};

TEST_F(LoaderManagerTest, PERFORMANCE_highList)
{
    MouCaCore::LoaderManager& loader = core->getLoaderManager();
    // Prepare manager
    ASSERT_NO_THROW(loader.initialize());

    // Loading
    ASSERT_NO_THROW(loader.loadResources(items));
    EXPECT_TRUE(items.empty()); // Here we release list

    ASSERT_NO_THROW(loader.synchronize());

    // Release manager
    ASSERT_NO_THROW(loader.release());
}

// EnableCodeCoverage

}