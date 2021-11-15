/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include <LibGLFW/include/GLFWWindow.h>

#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKContextWindow.h>

#include <LibRT/include/RTImage.h>
#include <LibRT/include/RTRenderDialog.h>

MouCaLabTest::MouCaLabTest()
{
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getWorkingPath(), MouCaCore::ResourceManager::Executable);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getWorkingPath() / ".." / ".." / "MouCaLab" / "UnitTests" / "GLSL",      MouCaCore::ResourceManager::ShadersSource);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getWorkingPath() / ".." / ".." / "MouCaLab" / "UnitTests" / "Renderer",  MouCaCore::ResourceManager::Renderer);

    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / "Configuration", MouCaCore::ResourceManager::Configuration);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / "fonts",         MouCaCore::ResourceManager::Fonts);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / "mesh",          MouCaCore::ResourceManager::Meshes);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / "textures",      MouCaCore::ResourceManager::Textures);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / ".." / "SpirV",  MouCaCore::ResourceManager::Shaders);
    _core.getResourceManager().addResourceFolder(MouCaEnvironment::getInputPath() / "references",    MouCaCore::ResourceManager::References);
}

void MouCaLabTest::SetUp()
{
    _core.getLoaderManager().initialize();

    _graphic.initialize();
}

void MouCaLabTest::TearDown()
{
    _graphic.release();

    _core.getLoaderManager().release();
}

void MouCaLabTest::configureEventManager()
{
    _eventManager = std::make_shared<EventManager3D>();
}

void MouCaLabTest::releaseEventManager()
{
    _eventManager.reset();
}

void MouCaLabTest::loadEngine(MouCaGraphic::VulkanManager& manager, const Core::String& fileName)
{
    // Build path
    Core::Path pathFile = _core.getResourceManager().getResourceFolder(MouCaCore::ResourceManager::Renderer) / fileName;

    // Read resource
    XML::ParserSPtr xmlFile = _core.getResourceManager().openXML(pathFile);
    xmlFile->openXMLFile();
    ASSERT_TRUE(xmlFile->isLoaded()) << "Impossible to read XML";

    // Let go !!
    MouCaGraphic::Engine3DXMLLoader loader(manager);
    MouCaGraphic::Engine3DXMLLoader::ContextLoading context(_graphic, *xmlFile, _core.getResourceManager());
    ASSERT_NO_THROW(loader.load(context));

    // Release resource (no needed anymore)
    _core.getResourceManager().releaseResource(std::move(xmlFile));
}

void MouCaLabTest::loadEngine(MouCaGraphic::Engine3DXMLLoader& loader, const Core::String& fileName)
{
    // Build path
    Core::Path pathFile = _core.getResourceManager().getResourceFolder(MouCaCore::ResourceManager::Renderer) / fileName;

    // Read resource
    XML::ParserSPtr xmlFile;
    ASSERT_NO_THROW(xmlFile = _core.getResourceManager().openXML(pathFile));
    ASSERT_NO_THROW(xmlFile->openXMLFile());
    ASSERT_TRUE(xmlFile->isLoaded()) << u8"Impossible to read XML";

    // Let go !!
    MouCaGraphic::Engine3DXMLLoader::ContextLoading context(_graphic, *xmlFile, _core.getResourceManager());
    try
    {
        loader.load(context);
        // Release resource (no needed anymore)
        _core.getResourceManager().releaseResource(std::move(xmlFile));
    }
    catch (const Core::Exception& exception)
    {
        // Merge all messages
        Core::String message;
        for (size_t id = 0; id < exception.getNbErrors(); ++id)
        {
            const auto& error = exception.read(id);
            message += error.getLibraryLabel() + u8" " + error.getErrorLabel() + u8":\n"
                + _core.getExceptionManager().getError(error) + u8"\n\n";
        }

        // Release resource (no needed anymore)
        _core.getResourceManager().releaseResource(std::move(xmlFile));

        FAIL() << u8"XML loading error:\n" << message << u8"\nXML file: " << pathFile;
    }
    catch (...)
    {
        // Release resource (no needed anymore)
        _core.getResourceManager().releaseResource(std::move(xmlFile));
        FAIL() << u8"XML loading error - Unknown error - XML file: " << pathFile;
    }
}

void MouCaLabTest::mainLoop(MouCaGraphic::VulkanManager& manager, const Core::String& title, const std::function<void(const double timer)>& afterRender)
{
    // Enable tracking
    enableFileTracking(manager);

    // Execute draw into another thread
    uint32_t lastFPS = 0;

    // Frame counter to display fps
    uint32_t frameCounter = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;

    // Force to show dialog
    {
        manager.getSurfaces().at(0)->_linkWindow.lock()->setVisibility(true);
    }

    double timer = 0.0;
    // Execute main event loop
    while (_graphic.getRTPlatform().isWindowsActive())
    {
        const auto tStart = std::chrono::high_resolution_clock::now();
        // Draw Frame
        {
            manager.execute(0, 0, false);
        }

        // Show FPS
        frameCounter++;
        const auto tEnd = std::chrono::high_resolution_clock::now();
        const auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        timer += tDiff / 1000.0;
        afterRender(timer);

        // Avoid to change FPS quickly (bad effect on window reactivity)
        const double fpsTimer = std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count();
        if (fpsTimer > 1000.0)
        {
            lastFPS = static_cast<uint32_t>((double)frameCounter * (1000.0 / fpsTimer));
            frameCounter = 0;
            lastTimestamp = tEnd;

            // Use FPS on Dialog title (can be dangerous if too quick)
            auto window = manager.getSurfaces().at(0)->_linkWindow.lock();
            if (window->getStateSize() == RT::Window::Normal)
            {
                std::stringstream ss;
                ss << title << lastFPS << u8" FPS";
                window->setWindowTitle(ss.str());
            }
        }

        // Pool now
        _graphic.getRTPlatform().pollEvents();
    }

    // Enable tracking
    disableFileTracking(manager);
}

void MouCaLabTest::takeScreenshot(MouCaGraphic::VulkanManager& manager, const Core::Path& imageFile, const size_t nbMaxDefectPixels, const double maxDistance4D)
{
    // Make screenshot
    const Core::Path path = MouCaEnvironment::getOutputPath() / imageFile;
    RT::ImageImportSPtr diskImage = _core.getResourceManager().createImage(path);
    ASSERT_NO_THROW(manager.takeScreenshot(path, diskImage, 0));

    auto& resourceManager = _core.getResourceManager();

    // Read reference (first time top because don't exist)
    const Core::Path reference = resourceManager.getResourceFolder(MouCaCore::ResourceManager::References) / imageFile;
    EXPECT_TRUE(std::filesystem::exists(reference)) << "Missing: " << reference; // DEV Issue: no reference existing !
    auto refImage = resourceManager.openImage(reference);
    {
        auto& loaderManager = _core.getLoaderManager();
        MouCaCore::LoadingItems loadingItems =
        {
            MouCaCore::LoadingItem(refImage, MouCaCore::LoadingItem::Direct),

        };
        loaderManager.loadResources(loadingItems);
    }

    // Compare images
    size_t nbDefect;
    double maxFoundDistance;
    const bool compare = diskImage->getImage().lock()->compare(*refImage->getImage().lock(), nbMaxDefectPixels, maxDistance4D, &nbDefect, &maxFoundDistance);
    if (!compare) // Have you update the reference ? Or bug ?
    {
        const Core::Path sourceFile(MouCaEnvironment::getOutputPath() / imageFile);
        const Core::Path targetParent(MouCaEnvironment::getOutputPath() / L".." / L".." / L".." / L"Report");
        const Core::Path targetFile(targetParent / imageFile);

        // Duplicate result into failure folder
        if (!std::filesystem::exists(targetParent))
            ASSERT_NO_THROW(std::filesystem::create_directories(targetParent)); // Recursively create target directory if not existing.
        ASSERT_NO_THROW(std::filesystem::copy_file(sourceFile, targetFile, std::filesystem::copy_options::overwrite_existing));

        EXPECT_TRUE(compare) << u8"Image comparison failed: " << imageFile << u8"\n"
            << u8"Defect pixel: " << nbDefect << " > " << nbMaxDefectPixels << u8"\n"
            << u8"With tolerance of " << maxDistance4D << u8"(max found: " << maxFoundDistance << u8")";
    }

    _core.getResourceManager().releaseResource(std::move(diskImage));
    _core.getResourceManager().releaseResource(std::move(refImage));
}

void MouCaLabTest::enableFileTracking(MouCaGraphic::VulkanManager& manager)
{
    auto& fileTracker = _core.getResourceManager().getTracker();
    MOUCA_PRE_CONDITION(!fileTracker.signalFileChanged().isConnected(&manager));

    fileTracker.signalFileChanged().connectMember(&manager, &MouCaGraphic::VulkanManager::afterShaderEdition);

    fileTracker.startTracking();
}

void MouCaLabTest::disableFileTracking(MouCaGraphic::VulkanManager& manager)
{
    auto& fileTracker = _core.getResourceManager().getTracker();
    MOUCA_PRE_CONDITION(fileTracker.signalFileChanged().isConnected(&manager));

    fileTracker.stopTracking();

    fileTracker.signalFileChanged().disconnect(&manager);
}

void MouCaLabTest::clearDialog(MouCaGraphic::VulkanManager& manager)
{
    // Clean allocate dialog
    bool empty = manager.getSurfaces().empty();
    while (!empty)
    {
        auto& surface = *manager.getSurfaces().begin();

        {
            GLFW::WindowWPtr windowWeak = std::dynamic_pointer_cast<GLFW::Window>(surface->_linkWindow.lock());
            _graphic.getRTPlatform().releaseWindow(windowWeak);
        }
        empty = manager.getSurfaces().empty();
    }
}
void MouCaLabTest::updateCommandBuffers(MouCaGraphic::Engine3DXMLLoader& loader, const VkCommandBufferResetFlags reset)
{
    // Execute Command
    for(auto cmdBuffer : loader._commandBuffers)
    {
        cmdBuffer.second.lock()->execute(reset);
    }
}

void MouCaLabTest::updateCommandBuffersSurface(MouCaGraphic::Engine3DXMLLoader& loader, const VkCommandBufferResetFlags reset)
{
    // Execute commands
    loader._surfaces[0].lock()->updateCommandBuffer(reset);
}
