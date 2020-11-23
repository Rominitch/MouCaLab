#include "Dependancies.h"

#include <MouCa3DEngine/interface/IGraphicConfiguration.h>

TEST(IGraphicConfiguration, build)
{
    auto mode = RT::Window::InternalMode;

    // Configure object
    MouCa3DEngine::IGraphicConfiguration configuration;
    configuration.initialize(mode);
    EXPECT_TRUE(configuration.getWindowConfiguration().empty());
    EXPECT_EQ(mode, configuration.getMode());

    // Check add window
    MouCa3DEngine::IGraphicConfiguration::Window window;
    configuration.addWindow(window);
    EXPECT_EQ(1, configuration.getWindowConfiguration().size());

    // Check initialize clean all data
    mode = RT::Window::FullscreenWindowed;
    configuration.initialize(RT::Window::FullscreenWindowed);
    EXPECT_TRUE(configuration.getWindowConfiguration().empty());
    EXPECT_EQ(mode, configuration.getMode());
}