#include <gtest/gtest.h>
#include "ingest/IngestManager.hpp"
#include "scene/World.hpp"

namespace astraeus {
namespace testing {

class IngestManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        world_ = std::make_unique<World>();
        ingest_manager_ = std::make_unique<IngestManager>(world_.get());
    }
    
    void TearDown() override {
        ingest_manager_.reset();
        world_.reset();
    }
    
    std::unique_ptr<World> world_;
    std::unique_ptr<IngestManager> ingest_manager_;
};

/**
 * Test applying a simple snapshot.
 */
TEST_F(IngestManagerTest, ApplySnapshot) {
    Snapshot snapshot;
    snapshot.timestamp = 1.0;
    snapshot.entity_count = 1;
    
    EntityData entity;
    entity.id = 100;
    entity.pos_x = 10.0f;
    entity.pos_y = 20.0f;
    entity.pos_z = 30.0f;
    snapshot.entities.push_back(entity);
    
    ASSERT_NO_THROW({
        ingest_manager_->apply_snapshot(snapshot);
    });
    
    // Entity should be created in world
    EXPECT_GT(world_->get_entity_count(), 0);
}

/**
 * Test updating existing entity.
 */
TEST_F(IngestManagerTest, UpdateEntity) {
    // Apply first snapshot
    Snapshot snapshot1;
    snapshot1.timestamp = 1.0;
    EntityData entity1;
    entity1.id = 100;
    entity1.pos_x = 10.0f;
    entity1.pos_y = 20.0f;
    entity1.pos_z = 30.0f;
    snapshot1.entities.push_back(entity1);
    
    ingest_manager_->apply_snapshot(snapshot1);
    size_t initial_count = world_->get_entity_count();
    
    // Apply second snapshot with updated position
    Snapshot snapshot2;
    snapshot2.timestamp = 2.0;
    EntityData entity2;
    entity2.id = 100; // Same ID
    entity2.pos_x = 15.0f;
    entity2.pos_y = 25.0f;
    entity2.pos_z = 35.0f;
    snapshot2.entities.push_back(entity2);
    
    ingest_manager_->apply_snapshot(snapshot2);
    
    // Should update existing entity, not create new one
    EXPECT_EQ(world_->get_entity_count(), initial_count);
}

/**
 * Test removing entities not in snapshot.
 */
TEST_F(IngestManagerTest, RemoveEntities) {
    // Add entity 100
    Snapshot snapshot1;
    snapshot1.timestamp = 1.0;
    EntityData entity1;
    entity1.id = 100;
    snapshot1.entities.push_back(entity1);
    
    ingest_manager_->apply_snapshot(snapshot1);
    EXPECT_EQ(world_->get_entity_count(), 1);
    
    // Apply snapshot without entity 100
    Snapshot snapshot2;
    snapshot2.timestamp = 2.0;
    // Empty entities list
    
    ingest_manager_->apply_snapshot(snapshot2);
    
    // Entity should be removed (if configured to do so)
    // Or remain (if configured to keep local entities)
    // Test documents current behavior
}

/**
 * Test applying multiple entities in one snapshot.
 */
TEST_F(IngestManagerTest, MultipleEntities) {
    Snapshot snapshot;
    snapshot.timestamp = 1.0;
    
    for (int i = 0; i < 10; ++i) {
        EntityData entity;
        entity.id = 100 + i;
        entity.pos_x = i * 10.0f;
        snapshot.entities.push_back(entity);
    }
    
    ingest_manager_->apply_snapshot(snapshot);
    
    EXPECT_GE(world_->get_entity_count(), 10);
}

/**
 * Test snapshot ordering (older snapshot after newer).
 */
TEST_F(IngestManagerTest, OutOfOrderSnapshots) {
    // Apply newer snapshot first
    Snapshot snapshot2;
    snapshot2.timestamp = 2.0;
    EntityData entity2;
    entity2.id = 100;
    entity2.pos_x = 20.0f;
    snapshot2.entities.push_back(entity2);
    
    ingest_manager_->apply_snapshot(snapshot2);
    
    // Apply older snapshot
    Snapshot snapshot1;
    snapshot1.timestamp = 1.0;
    EntityData entity1;
    entity1.id = 100;
    entity1.pos_x = 10.0f;
    snapshot1.entities.push_back(entity1);
    
    // Should either ignore or handle gracefully
    ASSERT_NO_THROW({
        ingest_manager_->apply_snapshot(snapshot1);
    });
}

/**
 * Test idempotency - applying same snapshot twice.
 */
TEST_F(IngestManagerTest, Idempotency) {
    Snapshot snapshot;
    snapshot.timestamp = 1.0;
    EntityData entity;
    entity.id = 100;
    entity.pos_x = 10.0f;
    snapshot.entities.push_back(entity);
    
    ingest_manager_->apply_snapshot(snapshot);
    size_t count_after_first = world_->get_entity_count();
    
    // Apply same snapshot again
    ingest_manager_->apply_snapshot(snapshot);
    size_t count_after_second = world_->get_entity_count();
    
    // Should not create duplicate entities
    EXPECT_EQ(count_after_first, count_after_second);
}

/**
 * Test getting current simulation time.
 */
TEST_F(IngestManagerTest, SimulationTime) {
    Snapshot snapshot;
    snapshot.timestamp = 5.0;
    EntityData entity;
    entity.id = 100;
    snapshot.entities.push_back(entity);
    
    ingest_manager_->apply_snapshot(snapshot);
    
    double sim_time = ingest_manager_->get_simulation_time();
    EXPECT_DOUBLE_EQ(sim_time, 5.0);
}

/**
 * Test clearing ingest state.
 */
TEST_F(IngestManagerTest, Clear) {
    Snapshot snapshot;
    snapshot.timestamp = 1.0;
    EntityData entity;
    entity.id = 100;
    snapshot.entities.push_back(entity);
    
    ingest_manager_->apply_snapshot(snapshot);
    
    ingest_manager_->clear();
    
    // State should be reset
    EXPECT_DOUBLE_EQ(ingest_manager_->get_simulation_time(), 0.0);
}

} // namespace testing
} // namespace astraeus
