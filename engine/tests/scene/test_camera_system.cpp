#include <gtest/gtest.h>
#include "scene/World.hpp"
#include "scene/CameraSystem.hpp"

namespace astraeus {
namespace testing {

class CameraSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        world_ = std::make_unique<World>();
        camera_system_ = std::make_unique<CameraSystem>(world_.get());
    }
    
    void TearDown() override {
        camera_system_.reset();
        world_.reset();
    }
    
    std::unique_ptr<World> world_;
    std::unique_ptr<CameraSystem> camera_system_;
};

/**
 * Test creating a camera entity.
 */
TEST_F(CameraSystemTest, CreateCamera) {
    EntityHandle camera_entity = camera_system_->create_camera();
    
    EXPECT_NE(camera_entity, INVALID_ENTITY);
    EXPECT_TRUE(world_->is_valid(camera_entity));
    EXPECT_TRUE(world_->has_camera_component(camera_entity));
}

/**
 * Test camera has transform component.
 */
TEST_F(CameraSystemTest, CameraHasTransform) {
    EntityHandle camera_entity = camera_system_->create_camera();
    
    EXPECT_TRUE(world_->has_transform(camera_entity));
}

/**
 * Test setting camera as active.
 */
TEST_F(CameraSystemTest, SetActiveCamera) {
    EntityHandle camera1 = camera_system_->create_camera();
    EntityHandle camera2 = camera_system_->create_camera();
    
    camera_system_->set_active_camera(camera1);
    EXPECT_EQ(camera_system_->get_active_camera(), camera1);
    
    camera_system_->set_active_camera(camera2);
    EXPECT_EQ(camera_system_->get_active_camera(), camera2);
}

/**
 * Test camera projection matrix setup.
 */
TEST_F(CameraSystemTest, ProjectionMatrix) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    camera_system_->set_perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    
    const float* proj_matrix = camera_system_->get_projection_matrix();
    ASSERT_NE(proj_matrix, nullptr);
    
    // Basic sanity check: projection matrix should not be all zeros
    bool has_nonzero = false;
    for (int i = 0; i < 16; ++i) {
        if (proj_matrix[i] != 0.0f) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

/**
 * Test camera view matrix computation.
 */
TEST_F(CameraSystemTest, ViewMatrix) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    world_->set_position(camera, 0, 0, 10);
    camera_system_->update();
    
    const float* view_matrix = camera_system_->get_view_matrix();
    ASSERT_NE(view_matrix, nullptr);
}

/**
 * Test orthographic projection.
 */
TEST_F(CameraSystemTest, OrthographicProjection) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    camera_system_->set_orthographic(-10, 10, -10, 10, 0.1f, 100.0f);
    
    const float* proj_matrix = camera_system_->get_projection_matrix();
    ASSERT_NE(proj_matrix, nullptr);
}

/**
 * Test camera frustum extraction.
 */
TEST_F(CameraSystemTest, FrustumExtraction) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    camera_system_->set_perspective(60.0f, 1.0f, 1.0f, 100.0f);
    world_->set_position(camera, 0, 0, 0);
    camera_system_->update();
    
    // Should be able to extract frustum planes
    const Frustum* frustum = camera_system_->get_frustum();
    ASSERT_NE(frustum, nullptr);
}

/**
 * Test camera look-at functionality.
 */
TEST_F(CameraSystemTest, LookAt) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    world_->set_position(camera, 0, 0, 10);
    camera_system_->look_at(0, 0, 0, 0, 1, 0);
    camera_system_->update();
    
    const float* view_matrix = camera_system_->get_view_matrix();
    ASSERT_NE(view_matrix, nullptr);
}

/**
 * Test multiple cameras don't interfere.
 */
TEST_F(CameraSystemTest, MultipleCameras) {
    EntityHandle camera1 = camera_system_->create_camera();
    EntityHandle camera2 = camera_system_->create_camera();
    EntityHandle camera3 = camera_system_->create_camera();
    
    world_->set_position(camera1, 10, 0, 0);
    world_->set_position(camera2, 0, 10, 0);
    world_->set_position(camera3, 0, 0, 10);
    
    camera_system_->set_active_camera(camera1);
    camera_system_->update();
    
    // All cameras should still be valid
    EXPECT_TRUE(world_->is_valid(camera1));
    EXPECT_TRUE(world_->is_valid(camera2));
    EXPECT_TRUE(world_->is_valid(camera3));
}

/**
 * Test destroying active camera.
 */
TEST_F(CameraSystemTest, DestroyActiveCamera) {
    EntityHandle camera = camera_system_->create_camera();
    camera_system_->set_active_camera(camera);
    
    world_->destroy_entity(camera);
    
    // Active camera should become invalid
    EXPECT_EQ(camera_system_->get_active_camera(), INVALID_ENTITY);
}

} // namespace testing
} // namespace astraeus
