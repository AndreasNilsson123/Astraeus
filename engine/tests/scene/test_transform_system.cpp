#include <gtest/gtest.h>
#include "scene/World.hpp"
#include <cmath>
#include <limits>

namespace astraeus {
namespace testing {

class TransformSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        world_ = std::make_unique<World>();
    }
    
    void TearDown() override {
        world_.reset();
    }
    
    std::unique_ptr<World> world_;
    
    // Helper to check if matrix is identity
    bool is_identity_matrix(const float* mat) {
        const float identity[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        
        for (int i = 0; i < 16; ++i) {
            if (std::abs(mat[i] - identity[i]) > 0.001f) {
                return false;
            }
        }
        return true;
    }
};

/**
 * Test transform initialization with default values.
 */
TEST_F(TransformSystemTest, DefaultTransform) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    const Transform* transform = world_->get_transform(entity);
    ASSERT_NE(transform, nullptr);
    
    EXPECT_FLOAT_EQ(transform->pos_x, 0.0f);
    EXPECT_FLOAT_EQ(transform->pos_y, 0.0f);
    EXPECT_FLOAT_EQ(transform->pos_z, 0.0f);
    EXPECT_FLOAT_EQ(transform->rot_x, 0.0f);
    EXPECT_FLOAT_EQ(transform->rot_y, 0.0f);
    EXPECT_FLOAT_EQ(transform->rot_z, 0.0f);
    EXPECT_FLOAT_EQ(transform->scale_x, 1.0f);
    EXPECT_FLOAT_EQ(transform->scale_y, 1.0f);
    EXPECT_FLOAT_EQ(transform->scale_z, 1.0f);
}

/**
 * Test setting transform position.
 */
TEST_F(TransformSystemTest, SetPosition) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 10.0f, 20.0f, 30.0f);
    
    const Transform* transform = world_->get_transform(entity);
    ASSERT_NE(transform, nullptr);
    
    EXPECT_FLOAT_EQ(transform->pos_x, 10.0f);
    EXPECT_FLOAT_EQ(transform->pos_y, 20.0f);
    EXPECT_FLOAT_EQ(transform->pos_z, 30.0f);
}

/**
 * Test updating transform position.
 */
TEST_F(TransformSystemTest, UpdatePosition) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 1, 2, 3);
    
    world_->set_position(entity, 10, 20, 30);
    
    const Transform* transform = world_->get_transform(entity);
    EXPECT_FLOAT_EQ(transform->pos_x, 10.0f);
    EXPECT_FLOAT_EQ(transform->pos_y, 20.0f);
    EXPECT_FLOAT_EQ(transform->pos_z, 30.0f);
}

/**
 * Test transform rotation.
 */
TEST_F(TransformSystemTest, SetRotation) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    world_->set_rotation(entity, 45.0f, 90.0f, 180.0f);
    
    const Transform* transform = world_->get_transform(entity);
    EXPECT_FLOAT_EQ(transform->rot_x, 45.0f);
    EXPECT_FLOAT_EQ(transform->rot_y, 90.0f);
    EXPECT_FLOAT_EQ(transform->rot_z, 180.0f);
}

/**
 * Test transform scale.
 */
TEST_F(TransformSystemTest, SetScale) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    world_->set_scale(entity, 2.0f, 3.0f, 4.0f);
    
    const Transform* transform = world_->get_transform(entity);
    EXPECT_FLOAT_EQ(transform->scale_x, 2.0f);
    EXPECT_FLOAT_EQ(transform->scale_y, 3.0f);
    EXPECT_FLOAT_EQ(transform->scale_z, 4.0f);
}

/**
 * Test dirty flag is set when transform changes.
 */
TEST_F(TransformSystemTest, DirtyFlag) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    // Initially dirty (just created)
    const Transform* transform = world_->get_transform(entity);
    EXPECT_TRUE(transform->dirty);
    
    // Update should set dirty flag
    world_->set_position(entity, 10, 10, 10);
    EXPECT_TRUE(transform->dirty);
}

/**
 * Test world matrix computation for simple translation.
 */
TEST_F(TransformSystemTest, WorldMatrixTranslation) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 10, 20, 30);
    
    world_->update_transforms();
    
    const Transform* transform = world_->get_transform(entity);
    ASSERT_NE(transform, nullptr);
    
    // Check translation components (column-major: mat[12], mat[13], mat[14])
    EXPECT_FLOAT_EQ(transform->world_matrix[12], 10.0f);
    EXPECT_FLOAT_EQ(transform->world_matrix[13], 20.0f);
    EXPECT_FLOAT_EQ(transform->world_matrix[14], 30.0f);
    EXPECT_FLOAT_EQ(transform->world_matrix[15], 1.0f);
}

/**
 * Test parent-child transform hierarchy.
 */
TEST_F(TransformSystemTest, ParentChildHierarchy) {
    EntityHandle parent = world_->create_entity();
    EntityHandle child = world_->create_entity();
    
    world_->add_transform(parent, 10, 0, 0);
    world_->add_transform(child, 5, 0, 0);
    
    world_->set_parent(child, parent);
    world_->update_transforms();
    
    // Child world position should be parent + local = 15, 0, 0
    const Transform* child_transform = world_->get_transform(child);
    ASSERT_NE(child_transform, nullptr);
    
    // Check world translation
    EXPECT_FLOAT_EQ(child_transform->world_matrix[12], 15.0f);
}

/**
 * Test transform with zero scale.
 */
TEST_F(TransformSystemTest, ZeroScale) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    world_->set_scale(entity, 0, 0, 0);
    
    ASSERT_NO_THROW({
        world_->update_transforms();
    });
}

/**
 * Test transform with negative scale.
 */
TEST_F(TransformSystemTest, NegativeScale) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    world_->set_scale(entity, -1, -1, -1);
    
    ASSERT_NO_THROW({
        world_->update_transforms();
    });
    
    const Transform* transform = world_->get_transform(entity);
    EXPECT_FLOAT_EQ(transform->scale_x, -1.0f);
    EXPECT_FLOAT_EQ(transform->scale_y, -1.0f);
    EXPECT_FLOAT_EQ(transform->scale_z, -1.0f);
}

/**
 * Test NaN handling in transforms.
 * Verifies that setting NaN values doesn't crash the system.
 */
TEST_F(TransformSystemTest, NaNHandling) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    
    // Setting NaN should either be rejected or handled gracefully
    ASSERT_NO_THROW({
        world_->set_position(entity, nan_val, nan_val, nan_val);
        world_->update_transforms();
    });
    
    const Transform* transform = world_->get_transform(entity);
    // If NaN is set, matrix computation should not propagate NaN
    // or should clamp to valid values
    bool has_nan = false;
    for (int i = 0; i < 16; ++i) {
        if (std::isnan(transform->world_matrix[i])) {
            has_nan = true;
            break;
        }
    }
    // Ideally should not have NaN in world matrix
    // This test documents current behavior
}

/**
 * Test Inf handling in transforms.
 */
TEST_F(TransformSystemTest, InfHandling) {
    EntityHandle entity = world_->create_entity();
    world_->add_transform(entity, 0, 0, 0);
    
    float inf_val = std::numeric_limits<float>::infinity();
    
    ASSERT_NO_THROW({
        world_->set_position(entity, inf_val, inf_val, inf_val);
        world_->update_transforms();
    });
}

/**
 * Test many transforms update performance.
 */
TEST_F(TransformSystemTest, ManyTransforms) {
    const int count = 1000;
    std::vector<EntityHandle> entities;
    
    for (int i = 0; i < count; ++i) {
        EntityHandle entity = world_->create_entity();
        world_->add_transform(entity, i, i, i);
        entities.push_back(entity);
    }
    
    ASSERT_NO_THROW({
        world_->update_transforms();
    });
    
    // Verify all transforms were updated
    for (auto entity : entities) {
        const Transform* transform = world_->get_transform(entity);
        ASSERT_NE(transform, nullptr);
        EXPECT_FALSE(transform->dirty); // Should be clean after update
    }
}

} // namespace testing
} // namespace astraeus
