#include "Dependencies.h"

#include <LibCore/include/CoreFile.h>
#include <LibCore/include/CoreResource.h>

#include <LibRT/include/RTAnimationBones.h>
#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTShaderFile.h>

#include <LibXML/include/XML.h>

#include <LibVulkan/interface/VKDefines.h> // Need to remove !!! Don't use Vulkan item only generic Lib

#include <MouCaCore/include/ResourceManager.h>
#include <MouCaCore/include/CoreSystem.h>

TEST(ResourceManager, resourceFolder)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();

    MouCaCore::ResourceManager* resources = NULL;
    ASSERT_NO_THROW(resources = &core->getResourceManager());

    const size_t outputID = 0;
    const Core::Path outputPath(MouCaEnvironment::getOutputPath());

    const size_t inputID = 1;
    const Core::Path inputPath(MouCaEnvironment::getInputPath());
    ASSERT_NO_THROW(resources->addResourceFolder(outputPath, outputID));
    ASSERT_NO_THROW(resources->addResourceFolder(inputPath, inputID));

    EXPECT_EQ(inputPath,  resources->getResourceFolder(inputID));
    EXPECT_EQ(outputPath, resources->getResourceFolder(outputID));
}

TEST(ResourceManager, resource)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();

    MouCaCore::ResourceManager* resources = NULL;
    ASSERT_NO_THROW(resources = &core->getResourceManager());

    Core::ResourceSPtr myFile(new Core::File());
    Core::ResourceSPtr myFile2(new Core::File());

    ASSERT_NO_THROW(resources->registerResource(myFile));
    ASSERT_NO_THROW(resources->registerResource(myFile2));

    ASSERT_NO_THROW(resources->unregisterResource(myFile2.get()));
    ASSERT_NO_THROW(resources->unregisterResource(myFile.get()));
}

TEST(ResourceManager, create)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();

    MouCaCore::ResourceManager* resources = NULL;
    ASSERT_NO_THROW(resources = &core->getResourceManager());

    // Create
    auto image = resources->createImage();
    EXPECT_EQ(1, resources->getNbResources());

    auto xml = resources->createXML(Core::StringOS(L"file.xml"));
    EXPECT_EQ(2, resources->getNbResources());

    auto file = resources->createFile(Core::StringOS(L"file.txt"));
    EXPECT_EQ(3, resources->getNbResources());

    // Release
    ASSERT_NO_THROW(resources->releaseResource(std::move(image)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(xml)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(file)));
    EXPECT_EQ(0, resources->getNbResources());
}

TEST(ResourceManager, readExisting)
{
    auto core = std::make_unique<MouCaCore::CoreSystem>();

    MouCaCore::ResourceManager* resources = NULL;
    ASSERT_NO_THROW(resources = &core->getResourceManager());

    const Core::Path imageFile     = Core::Path(u8"textures")  / u8"image.png";
    const Core::Path meshFile      = Core::Path(u8"mesh")      / u8"armor"   / u8"armor.dae";
    const Core::Path xmlFile       = Core::Path(u8"libraries") / u8"ErrorXML"/ u8"DemoLibrary.xml";
    const Core::Path randomFile    = Core::Path(u8"libraries") / u8"MyString.txt";
    const Core::Path shaderFile    = Core::Path(u8"libraries") / u8"vertex.spv";
    const Core::Path animationFile = Core::Path(u8"mesh")      / u8"goblin.dae";

    // Open
    auto image = resources->openImage(MouCaEnvironment::getInputPath() / imageFile);
    EXPECT_EQ(1, resources->getNbResources());

    auto mesh = resources->openMeshImport(MouCaEnvironment::getInputPath() / meshFile, Vulkan::getMeshDescriptor(), RT::MeshImport::ComputeAllInvert);
    EXPECT_EQ(2, resources->getNbResources());

    auto xml = resources->openXML(MouCaEnvironment::getInputPath() / xmlFile);
    EXPECT_EQ(3, resources->getNbResources());

    auto file = resources->openFile(MouCaEnvironment::getInputPath() / randomFile);
    EXPECT_EQ(4, resources->getNbResources());

    auto shader = resources->openShader( MouCaEnvironment::getInputPath() / shaderFile, RT::ShaderKind::Fragment );
    EXPECT_EQ(5, resources->getNbResources());

    auto animation = resources->openAnimation(MouCaEnvironment::getInputPath() / animationFile);
    const size_t maxResources = resources->getNbResources();
    EXPECT_EQ(6, maxResources);

    // Reopen
    {
        image = resources->openImage(MouCaEnvironment::getInputPath() / imageFile);
        EXPECT_EQ(maxResources, resources->getNbResources());

        mesh = resources->openMeshImport(MouCaEnvironment::getInputPath() / meshFile, Vulkan::getMeshDescriptor(), RT::MeshImport::ComputeAllInvert);
        EXPECT_EQ(maxResources, resources->getNbResources());

        xml = resources->openXML(MouCaEnvironment::getInputPath() / xmlFile);
        EXPECT_EQ(maxResources, resources->getNbResources());

        file = resources->openFile(MouCaEnvironment::getInputPath() / randomFile);
        EXPECT_EQ(maxResources, resources->getNbResources());

        shader = resources->openShader(MouCaEnvironment::getInputPath() / shaderFile, RT::ShaderKind::Fragment );
        EXPECT_EQ(maxResources, resources->getNbResources());

        animation = resources->openAnimation(MouCaEnvironment::getInputPath() / animationFile);
        EXPECT_EQ(maxResources, resources->getNbResources());
    }

    // Multi-File: Since animation: resource can be multi-type (Animation and mesh).
    //             We don't load data so it's impossible to prove file is corresponding to data.
    //             Before we check just resource format is similar.
    {
        // Invalid format but possible in API.
        XML::ParserSPtr parserInv;
        ASSERT_NO_THROW(parserInv = resources->openXML(MouCaEnvironment::getInputPath() / imageFile));
        ASSERT_NO_THROW(resources->releaseResource(std::move(parserInv)));

        RT::MeshImportSPtr meshInv;
        ASSERT_NO_THROW(meshInv = resources->openMeshImport(MouCaEnvironment::getInputPath() / xmlFile));
        ASSERT_NO_THROW(resources->releaseResource(std::move(meshInv)));

        RT::ShaderFileSPtr shaderInv;
        ASSERT_NO_THROW(shaderInv = resources->openShader(MouCaEnvironment::getInputPath() / imageFile, RT::ShaderKind::Fragment));
        ASSERT_NO_THROW(resources->releaseResource(std::move(shaderInv)));

        RT::ImageImportSPtr imageInv;
        ASSERT_NO_THROW(imageInv = resources->openImage(MouCaEnvironment::getInputPath() / xmlFile));
        ASSERT_NO_THROW(resources->releaseResource(std::move(imageInv)));

        RT::AnimationImporterSPtr animationInv;
        ASSERT_NO_THROW(animationInv = resources->openAnimation(MouCaEnvironment::getInputPath() / xmlFile));
        ASSERT_NO_THROW(resources->releaseResource(std::move(animationInv)));
        
        EXPECT_EQ(maxResources, resources->getNbResources());
    }

    // Release
    ASSERT_NO_THROW(resources->releaseResource(std::move(image)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(mesh)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(xml)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(file)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(shader)));
    ASSERT_NO_THROW(resources->releaseResource(std::move(animation)));
    EXPECT_EQ(0, resources->getNbResources());
}