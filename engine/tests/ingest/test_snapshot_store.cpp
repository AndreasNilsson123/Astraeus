#include <gtest/gtest.h>
#include "ingest/SnapshotStore.hpp"

namespace astraeus {
namespace testing {

class SnapshotStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        snapshot_store_ = std::make_unique<SnapshotStore>();
    }
    
    void TearDown() override {
        snapshot_store_.reset();
    }
    
    std::unique_ptr<SnapshotStore> snapshot_store_;
};

/**
 * Test adding a snapshot to the store.
 */
TEST_F(SnapshotStoreTest, AddSnapshot) {
    Snapshot snapshot;
    snapshot.timestamp = 1.0;
    snapshot.entity_count = 10;
    
    ASSERT_NO_THROW({
        snapshot_store_->add_snapshot(snapshot);
    });
}

/**
 * Test retrieving a snapshot by timestamp.
 */
TEST_F(SnapshotStoreTest, GetSnapshotByTimestamp) {
    Snapshot snapshot1;
    snapshot1.timestamp = 1.0;
    snapshot1.entity_count = 10;
    
    Snapshot snapshot2;
    snapshot2.timestamp = 2.0;
    snapshot2.entity_count = 20;
    
    snapshot_store_->add_snapshot(snapshot1);
    snapshot_store_->add_snapshot(snapshot2);
    
    const Snapshot* retrieved = snapshot_store_->get_snapshot(1.0);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_DOUBLE_EQ(retrieved->timestamp, 1.0);
    EXPECT_EQ(retrieved->entity_count, 10);
}

/**
 * Test getting closest snapshot to a timestamp.
 */
TEST_F(SnapshotStoreTest, GetClosestSnapshot) {
    snapshot_store_->add_snapshot({0.0, 0});
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({2.0, 20});
    
    // Query between snapshots
    const Snapshot* closest = snapshot_store_->get_closest_snapshot(1.5);
    ASSERT_NE(closest, nullptr);
    // Should get either 1.0 or 2.0 depending on implementation
    EXPECT_TRUE(closest->timestamp == 1.0 || closest->timestamp == 2.0);
}

/**
 * Test getting snapshot range for interpolation.
 */
TEST_F(SnapshotStoreTest, GetSnapshotRange) {
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({2.0, 20});
    snapshot_store_->add_snapshot({3.0, 30});
    
    const Snapshot* before = nullptr;
    const Snapshot* after = nullptr;
    
    bool found = snapshot_store_->get_snapshot_range(2.5, &before, &after);
    
    if (found) {
        ASSERT_NE(before, nullptr);
        ASSERT_NE(after, nullptr);
        EXPECT_LE(before->timestamp, 2.5);
        EXPECT_GE(after->timestamp, 2.5);
    }
}

/**
 * Test removing old snapshots.
 */
TEST_F(SnapshotStoreTest, RemoveOldSnapshots) {
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({2.0, 20});
    snapshot_store_->add_snapshot({3.0, 30});
    snapshot_store_->add_snapshot({4.0, 40});
    
    // Remove snapshots older than 2.5
    snapshot_store_->remove_before(2.5);
    
    // Snapshots at 1.0 and 2.0 should be gone
    EXPECT_EQ(snapshot_store_->get_snapshot(1.0), nullptr);
    EXPECT_EQ(snapshot_store_->get_snapshot(2.0), nullptr);
    
    // Snapshots at 3.0 and 4.0 should still exist
    EXPECT_NE(snapshot_store_->get_snapshot(3.0), nullptr);
    EXPECT_NE(snapshot_store_->get_snapshot(4.0), nullptr);
}

/**
 * Test clearing all snapshots.
 */
TEST_F(SnapshotStoreTest, ClearAll) {
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({2.0, 20});
    snapshot_store_->add_snapshot({3.0, 30});
    
    snapshot_store_->clear();
    
    EXPECT_EQ(snapshot_store_->get_snapshot(1.0), nullptr);
    EXPECT_EQ(snapshot_store_->get_snapshot(2.0), nullptr);
    EXPECT_EQ(snapshot_store_->get_snapshot(3.0), nullptr);
}

/**
 * Test snapshot count tracking.
 */
TEST_F(SnapshotStoreTest, SnapshotCount) {
    EXPECT_EQ(snapshot_store_->get_count(), 0);
    
    snapshot_store_->add_snapshot({1.0, 10});
    EXPECT_EQ(snapshot_store_->get_count(), 1);
    
    snapshot_store_->add_snapshot({2.0, 20});
    EXPECT_EQ(snapshot_store_->get_count(), 2);
    
    snapshot_store_->clear();
    EXPECT_EQ(snapshot_store_->get_count(), 0);
}

/**
 * Test getting oldest and newest snapshots.
 */
TEST_F(SnapshotStoreTest, OldestAndNewest) {
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({3.0, 30});
    snapshot_store_->add_snapshot({2.0, 20});
    
    const Snapshot* oldest = snapshot_store_->get_oldest();
    ASSERT_NE(oldest, nullptr);
    EXPECT_DOUBLE_EQ(oldest->timestamp, 1.0);
    
    const Snapshot* newest = snapshot_store_->get_newest();
    ASSERT_NE(newest, nullptr);
    EXPECT_DOUBLE_EQ(newest->timestamp, 3.0);
}

/**
 * Test query with empty store.
 */
TEST_F(SnapshotStoreTest, QueryEmpty) {
    EXPECT_EQ(snapshot_store_->get_snapshot(1.0), nullptr);
    EXPECT_EQ(snapshot_store_->get_oldest(), nullptr);
    EXPECT_EQ(snapshot_store_->get_newest(), nullptr);
}

/**
 * Test adding snapshots out of order.
 */
TEST_F(SnapshotStoreTest, OutOfOrderAdd) {
    snapshot_store_->add_snapshot({3.0, 30});
    snapshot_store_->add_snapshot({1.0, 10});
    snapshot_store_->add_snapshot({2.0, 20});
    
    // Should handle out-of-order insertion
    EXPECT_NE(snapshot_store_->get_snapshot(1.0), nullptr);
    EXPECT_NE(snapshot_store_->get_snapshot(2.0), nullptr);
    EXPECT_NE(snapshot_store_->get_snapshot(3.0), nullptr);
    
    const Snapshot* oldest = snapshot_store_->get_oldest();
    EXPECT_DOUBLE_EQ(oldest->timestamp, 1.0);
}

} // namespace testing
} // namespace astraeus
