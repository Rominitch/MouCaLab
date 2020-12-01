#include "Dependencies.h"

#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTGeometry.h>
#include <LibRT/include/RTScene.h>

TEST(Scene, add)
{
    RT::Scene scene;

    EXPECT_TRUE(scene.getCameras().empty());
    EXPECT_TRUE(scene.getGeometries().empty());
    EXPECT_TRUE(scene.getLights().empty());

    RT::GeometrySPtr geometry = std::make_shared<RT::GeometryExternal>();
    scene.addGeometry(geometry);

    EXPECT_EQ(1, scene.getGeometries().size());
    EXPECT_TRUE(scene.getCameras().empty());
    EXPECT_TRUE(scene.getLights().empty());

    RT::LightSPtr light = std::make_shared<RT::Light>();
    scene.addLight(light);

    EXPECT_EQ(1, scene.getGeometries().size());
    EXPECT_TRUE(scene.getCameras().empty());
    EXPECT_EQ(1, scene.getLights().size());

    RT::CameraSPtr camera = std::make_shared<RT::Camera>();
    scene.addCamera(camera);

    EXPECT_EQ(1, scene.getGeometries().size());
    EXPECT_EQ(1, scene.getCameras().size());
    EXPECT_EQ(1, scene.getLights().size());

    ASSERT_NO_THROW(scene.release());
}

TEST(Scene, root)
{
    RT::Scene scene;
    const RT::Object& root = scene.getRoot();
    EXPECT_EQ(u8"SceneRoot", root.getLabel());
}