#include <gtest/gtest.h>
#include "scene/spatial/SpatialIndex.hpp"

namespace astraeus {
namespace testing {

class SpatialIndexTest : public ::testing::Test {
protected:
    void SetUp() override {
        spatial_index_ = std::make_unique<SpatialIndex>();
    }
    
    void TearDown() override {
        spatial_index_.reset();
    }
    
    std::unique_ptr<SpatialIndex> spatial_index_;
};

/**
 * Test adding an object to the spatial index.
 */
TEST_F(SpatialIndexTest, AddObject) {
    AABB bounds;
    bounds.min_x = 0; bounds.min_y = 0; bounds.min_z = 0;
    bounds.max_x = 1; bounds.max_y = 1; bounds.max_z = 1;
    
    uint32_t id = spatial_index_->add_object(1, bounds);
    EXPECT_GT(id, 0);
}

/**
 * Test removing an object from the spatial index.
 */
TEST_F(SpatialIndexTest, RemoveObject) {
    AABB bounds;
    bounds.min_x = 0; bounds.min_y = 0; bounds.min_z = 0;
    bounds.max_x = 1; bounds.max_y = 1; bounds.max_z = 1;
    
    uint32_t id = spatial_index_->add_object(1, bounds);
    
    ASSERT_NO_THROW({
        spatial_index_->remove_object(id);
    });
}

/**
 * Test updating object bounds.
 */
TEST_F(SpatialIndexTest, UpdateObject) {
    AABB initial_bounds;
    initial_bounds.min_x = 0; initial_bounds.min_y = 0; initial_bounds.min_z = 0;
    initial_bounds.max_x = 1; initial_bounds.max_y = 1; initial_bounds.max_z = 1;
    
    uint32_t id = spatial_index_->add_object(1, initial_bounds);
    
    AABB new_bounds;
    new_bounds.min_x = 2; new_bounds.min_y = 2; new_bounds.min_z = 2;
    new_bounds.max_x = 3; new_bounds.max_y = 3; new_bounds.max_z = 3;
    
    ASSERT_NO_THROW({
        spatial_index_->update_object(id, new_bounds);
    });
}

/**
 * Test point query.
 */
TEST_F(SpatialIndexTest, PointQuery) {
    AABB bounds;
    bounds.min_x = 0; bounds.min_y = 0; bounds.min_z = 0;
    bounds.max_x = 10; bounds.max_y = 10; bounds.max_z = 10;
    
    spatial_index_->add_object(1, bounds);
    
    // Point inside bounds
    auto results_inside = spatial_index_->query_point(5, 5, 5);
    EXPECT_GT(results_inside.size(), 0);
    
    // Point outside bounds
    auto results_outside = spatial_index_->query_point(20, 20, 20);
    EXPECT_EQ(results_outside.size(), 0);
}

/**
 * Test AABB query (box intersection).
 */
TEST_F(SpatialIndexTest, AABBQuery) {
    AABB object_bounds;
    object_bounds.min_x = 0; object_bounds.min_y = 0; object_bounds.min_z = 0;
    object_bounds.max_x = 10; object_bounds.max_y = 10; object_bounds.max_z = 10;
    
    spatial_index_->add_object(1, object_bounds);
    
    // Overlapping query
    AABB query_overlap;
    query_overlap.min_x = 5; query_overlap.min_y = 5; query_overlap.min_z = 5;
    query_overlap.max_x = 15; query_overlap.max_y = 15; query_overlap.max_z = 15;
    
    auto results_overlap = spatial_index_->query_aabb(query_overlap);
    EXPECT_GT(results_overlap.size(), 0);
    
    // Non-overlapping query
    AABB query_no_overlap;
    query_no_overlap.min_x = 20; query_no_overlap.min_y = 20; query_no_overlap.min_z = 20;
    query_no_overlap.max_x = 30; query_no_overlap.max_y = 30; query_no_overlap.max_z = 30;
    
    auto results_no_overlap = spatial_index_->query_aabb(query_no_overlap);
    EXPECT_EQ(results_no_overlap.size(), 0);
}

/**
 * Test sphere query (radius query).
 */
TEST_F(SpatialIndexTest, SphereQuery) {
    AABB bounds;
    bounds.min_x = 0; bounds.min_y = 0; bounds.min_z = 0;
    bounds.max_x = 1; bounds.max_y = 1; bounds.max_z = 1;
    
    spatial_index_->add_object(1, bounds);
    
    // Query near the object
    auto results_near = spatial_index_->query_sphere(0.5f, 0.5f, 0.5f, 2.0f);
    EXPECT_GT(results_near.size(), 0);
    
    // Query far from the object
    auto results_far = spatial_index_->query_sphere(100, 100, 100, 1.0f);
    EXPECT_EQ(results_far.size(), 0);
}

/**
 * Test adding multiple objects.
 */
TEST_F(SpatialIndexTest, MultipleObjects) {
    const int count = 100;
    std::vector<uint32_t> ids;
    
    for (int i = 0; i < count; ++i) {
        AABB bounds;
        bounds.min_x = i * 10; bounds.min_y = 0; bounds.min_z = 0;
        bounds.max_x = i * 10 + 1; bounds.max_y = 1; bounds.max_z = 1;
        
        uint32_t id = spatial_index_->add_object(i, bounds);
        ids.push_back(id);
    }
    
    EXPECT_EQ(ids.size(), count);
}

/**
 * Test clearing the spatial index.
 */
TEST_F(SpatialIndexTest, Clear) {
    for (int i = 0; i < 10; ++i) {
        AABB bounds;
        bounds.min_x = i; bounds.min_y = i; bounds.min_z = i;
        bounds.max_x = i + 1; bounds.max_y = i + 1; bounds.max_z = i + 1;
        spatial_index_->add_object(i, bounds);
    }
    
    spatial_index_->clear();
    
    // After clear, query should return no results
    auto results = spatial_index_->query_sphere(5, 5, 5, 10);
    EXPECT_EQ(results.size(), 0);
}

/**
 * Test query with empty spatial index.
 */
TEST_F(SpatialIndexTest, QueryEmpty) {
    auto results = spatial_index_->query_point(0, 0, 0);
    EXPECT_EQ(results.size(), 0);
}

/**
 * Test overlapping objects query.
 */
TEST_F(SpatialIndexTest, OverlappingObjects) {
    // Add multiple overlapping objects
    AABB bounds1;
    bounds1.min_x = 0; bounds1.min_y = 0; bounds1.min_z = 0;
    bounds1.max_x = 10; bounds1.max_y = 10; bounds1.max_z = 10;
    
    AABB bounds2;
    bounds2.min_x = 5; bounds2.min_y = 5; bounds2.min_z = 5;
    bounds2.max_x = 15; bounds2.max_y = 15; bounds2.max_z = 15;
    
    spatial_index_->add_object(1, bounds1);
    spatial_index_->add_object(2, bounds2);
    
    // Query in overlapping region
    auto results = spatial_index_->query_point(7, 7, 7);
    EXPECT_GE(results.size(), 1); // Should find at least one, possibly both
}

} // namespace testing
} // namespace astraeus
