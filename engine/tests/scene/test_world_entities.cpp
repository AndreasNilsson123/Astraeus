#include <gtest/gtest.h>
#include "scene/World.hpp"

namespace astraeus {
namespace testing {

class WorldEntitiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        world_ = std::make_unique<World>();
    }
    
    void TearDown() override {
        world_.reset();
    }
    
    std::unique_ptr<World> world_;
};

/**
 * Test basic entity creation.
 */
TEST_F(WorldEntitiesTest, CreateEntity) {
    EntityHandle entity = world_->create_entity();
    EXPECT_NE(entity, INVALID_ENTITY);
    EXPECT_TRUE(world_->is_valid(entity));
}

/**
 * Test multiple entity creation.
 */
TEST_F(WorldEntitiesTest, CreateMultipleEntities) {
    const int count = 100;
    std::vector<EntityHandle> entities;
    
    for (int i = 0; i < count; ++i) {
        EntityHandle entity = world_->create_entity();
        EXPECT_NE(entity, INVALID_ENTITY);
        entities.push_back(entity);
    }
    
    // All entities should be valid
    for (auto entity : entities) {
        EXPECT_TRUE(world_->is_valid(entity));
    }
}

/**
 * Test entity destruction.
 */
TEST_F(WorldEntitiesTest, DestroyEntity) {
    EntityHandle entity = world_->create_entity();
    EXPECT_TRUE(world_->is_valid(entity));
    
    world_->destroy_entity(entity);
    EXPECT_FALSE(world_->is_valid(entity));
}

/**
 * Test entity handle reuse after destruction.
 */
TEST_F(WorldEntitiesTest, HandleReuse) {
    // Create and destroy an entity
    EntityHandle entity1 = world_->create_entity();
    world_->destroy_entity(entity1);
    
    // Create another entity - may reuse the slot
    EntityHandle entity2 = world_->create_entity();
    EXPECT_NE(entity2, INVALID_ENTITY);
    EXPECT_TRUE(world_->is_valid(entity2));
    
    // Old handle should still be invalid
    EXPECT_FALSE(world_->is_valid(entity1));
}

/**
 * Test adding transform component.
 */
TEST_F(WorldEntitiesTest, AddTransformComponent) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_transform(entity, 1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(world_->has_transform(entity));
    
    const Transform* transform = world_->get_transform(entity);
    ASSERT_NE(transform, nullptr);
    EXPECT_FLOAT_EQ(transform->pos_x, 1.0f);
    EXPECT_FLOAT_EQ(transform->pos_y, 2.0f);
    EXPECT_FLOAT_EQ(transform->pos_z, 3.0f);
}

/**
 * Test adding renderable component.
 */
TEST_F(WorldEntitiesTest, AddRenderableComponent) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_renderable(entity);
    EXPECT_TRUE(world_->has_renderable(entity));
    
    const Renderable* renderable = world_->get_renderable(entity);
    ASSERT_NE(renderable, nullptr);
    EXPECT_TRUE(renderable->visible);
}

/**
 * Test adding color component.
 */
TEST_F(WorldEntitiesTest, AddColorComponent) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_color(entity, 1.0f, 0.5f, 0.0f, 1.0f);
    EXPECT_TRUE(world_->has_color(entity));
    
    const Color* color = world_->get_color(entity);
    ASSERT_NE(color, nullptr);
    EXPECT_FLOAT_EQ(color->r, 1.0f);
    EXPECT_FLOAT_EQ(color->g, 0.5f);
    EXPECT_FLOAT_EQ(color->b, 0.0f);
    EXPECT_FLOAT_EQ(color->a, 1.0f);
}

/**
 * Test removing components.
 */
TEST_F(WorldEntitiesTest, RemoveComponent) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_transform(entity, 0, 0, 0);
    EXPECT_TRUE(world_->has_transform(entity));
    
    world_->remove_transform(entity);
    EXPECT_FALSE(world_->has_transform(entity));
}

/**
 * Test entity with multiple components.
 */
TEST_F(WorldEntitiesTest, MultipleComponents) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_transform(entity, 1, 2, 3);
    world_->add_renderable(entity);
    world_->add_color(entity, 0.5f, 0.5f, 0.5f, 1.0f);
    
    EXPECT_TRUE(world_->has_transform(entity));
    EXPECT_TRUE(world_->has_renderable(entity));
    EXPECT_TRUE(world_->has_color(entity));
}

/**
 * Test getting component from entity without it returns nullptr.
 */
TEST_F(WorldEntitiesTest, GetNonExistentComponent) {
    EntityHandle entity = world_->create_entity();
    
    const Transform* transform = world_->get_transform(entity);
    EXPECT_EQ(transform, nullptr);
}

/**
 * Test destroying entity with components.
 */
TEST_F(WorldEntitiesTest, DestroyEntityWithComponents) {
    EntityHandle entity = world_->create_entity();
    
    world_->add_transform(entity, 1, 2, 3);
    world_->add_renderable(entity);
    world_->add_color(entity, 1, 1, 1, 1);
    
    ASSERT_NO_THROW({
        world_->destroy_entity(entity);
    });
    
    EXPECT_FALSE(world_->is_valid(entity));
}

/**
 * Test entity count tracking.
 */
TEST_F(WorldEntitiesTest, EntityCount) {
    size_t initial_count = world_->get_entity_count();
    
    EntityHandle e1 = world_->create_entity();
    EXPECT_EQ(world_->get_entity_count(), initial_count + 1);
    
    EntityHandle e2 = world_->create_entity();
    EXPECT_EQ(world_->get_entity_count(), initial_count + 2);
    
    world_->destroy_entity(e1);
    EXPECT_EQ(world_->get_entity_count(), initial_count + 1);
    
    world_->destroy_entity(e2);
    EXPECT_EQ(world_->get_entity_count(), initial_count);
}

/**
 * Test clearing all entities.
 */
TEST_F(WorldEntitiesTest, ClearAll) {
    for (int i = 0; i < 10; ++i) {
        EntityHandle entity = world_->create_entity();
        world_->add_transform(entity, i, i, i);
    }
    
    EXPECT_GT(world_->get_entity_count(), 0);
    
    world_->clear_all();
    
    EXPECT_EQ(world_->get_entity_count(), 0);
}

} // namespace testing
} // namespace astraeus
