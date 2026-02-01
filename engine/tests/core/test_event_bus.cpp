#include <gtest/gtest.h>
#include "core/EventBus.hpp"

namespace astraeus {
namespace testing {

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        event_bus_ = std::make_unique<EventBus>();
    }
    
    void TearDown() override {
        // Clean up any remaining events
        event_bus_->clear();
        event_bus_.reset();
    }
    
    std::unique_ptr<EventBus> event_bus_;
};

/**
 * Test basic event posting and polling.
 */
TEST_F(EventBusTest, PostAndPoll) {
    auto* event = new SelectionChangedEvent(42, 1.0f, 2.0f, 3.0f);
    
    event_bus_->post(event);
    
    EXPECT_EQ(event_bus_->pending_count(), 1);
    
    Event* polled = event_bus_->poll();
    ASSERT_NE(polled, nullptr);
    EXPECT_EQ(polled->type, EventType::SelectionChanged);
    
    auto* selection = static_cast<SelectionChangedEvent*>(polled);
    EXPECT_EQ(selection->entity_id, 42);
    EXPECT_FLOAT_EQ(selection->world_x, 1.0f);
    
    delete polled;
}

/**
 * Test multiple events in order.
 */
TEST_F(EventBusTest, MultipleEventsInOrder) {
    event_bus_->post(new EntityCreatedEvent(1));
    event_bus_->post(new EntityCreatedEvent(2));
    event_bus_->post(new EntityCreatedEvent(3));
    
    EXPECT_EQ(event_bus_->pending_count(), 3);
    
    Event* e1 = event_bus_->poll();
    ASSERT_NE(e1, nullptr);
    EXPECT_EQ(static_cast<EntityCreatedEvent*>(e1)->entity_id, 1);
    delete e1;
    
    Event* e2 = event_bus_->poll();
    ASSERT_NE(e2, nullptr);
    EXPECT_EQ(static_cast<EntityCreatedEvent*>(e2)->entity_id, 2);
    delete e2;
    
    Event* e3 = event_bus_->poll();
    ASSERT_NE(e3, nullptr);
    EXPECT_EQ(static_cast<EntityCreatedEvent*>(e3)->entity_id, 3);
    delete e3;
}

/**
 * Test polling empty queue.
 */
TEST_F(EventBusTest, PollEmpty) {
    Event* event = event_bus_->poll();
    EXPECT_EQ(event, nullptr);
}

/**
 * Test peek without removing.
 */
TEST_F(EventBusTest, Peek) {
    event_bus_->post(new EntityCreatedEvent(100));
    
    const Event* peeked1 = event_bus_->peek();
    ASSERT_NE(peeked1, nullptr);
    EXPECT_EQ(peeked1->type, EventType::EntityCreated);
    
    // Peeking shouldn't remove it
    EXPECT_EQ(event_bus_->pending_count(), 1);
    
    const Event* peeked2 = event_bus_->peek();
    EXPECT_EQ(peeked1, peeked2); // Should be same event
    
    // Now poll it
    Event* polled = event_bus_->poll();
    ASSERT_NE(polled, nullptr);
    delete polled;
    
    // Queue should be empty now
    EXPECT_EQ(event_bus_->pending_count(), 0);
}

/**
 * Test clearing events.
 */
TEST_F(EventBusTest, Clear) {
    event_bus_->post(new EntityCreatedEvent(1));
    event_bus_->post(new EntityCreatedEvent(2));
    event_bus_->post(new EntityCreatedEvent(3));
    
    EXPECT_EQ(event_bus_->pending_count(), 3);
    
    event_bus_->clear();
    
    EXPECT_EQ(event_bus_->pending_count(), 0);
    EXPECT_EQ(event_bus_->poll(), nullptr);
}

/**
 * Test different event types.
 */
TEST_F(EventBusTest, DifferentEventTypes) {
    event_bus_->post(new SelectionChangedEvent(10, 1, 2, 3));
    event_bus_->post(new EntityCreatedEvent(20));
    event_bus_->post(new IngestStartedEvent(1, 1000));
    
    Event* e1 = event_bus_->poll();
    EXPECT_EQ(e1->type, EventType::SelectionChanged);
    delete e1;
    
    Event* e2 = event_bus_->poll();
    EXPECT_EQ(e2->type, EventType::EntityCreated);
    delete e2;
    
    Event* e3 = event_bus_->poll();
    EXPECT_EQ(e3->type, EventType::IngestStarted);
    delete e3;
}

/**
 * Test AssetLoadedEvent.
 */
TEST_F(EventBusTest, AssetLoadedEvent) {
    auto* event = new AssetLoadedEvent(123, 0, "test_mesh.obj");
    event_bus_->post(event);
    
    Event* polled = event_bus_->poll();
    ASSERT_NE(polled, nullptr);
    EXPECT_EQ(polled->type, EventType::AssetLoaded);
    
    auto* asset_event = static_cast<AssetLoadedEvent*>(polled);
    EXPECT_EQ(asset_event->asset_id, 123);
    EXPECT_EQ(asset_event->asset_type, 0);
    EXPECT_STREQ(asset_event->asset_name, "test_mesh.obj");
    
    delete polled;
}

/**
 * Test IngestProgressEvent.
 */
TEST_F(EventBusTest, IngestProgressEvent) {
    auto* event = new IngestProgressEvent(500, 1000);
    event_bus_->post(event);
    
    Event* polled = event_bus_->poll();
    ASSERT_NE(polled, nullptr);
    
    auto* progress = static_cast<IngestProgressEvent*>(polled);
    EXPECT_EQ(progress->bytes_processed, 500);
    EXPECT_EQ(progress->total_bytes, 1000);
    EXPECT_FLOAT_EQ(progress->progress, 0.5f);
    
    delete polled;
}

/**
 * Test pending count tracking.
 */
TEST_F(EventBusTest, PendingCount) {
    EXPECT_EQ(event_bus_->pending_count(), 0);
    
    event_bus_->post(new EntityCreatedEvent(1));
    EXPECT_EQ(event_bus_->pending_count(), 1);
    
    event_bus_->post(new EntityCreatedEvent(2));
    EXPECT_EQ(event_bus_->pending_count(), 2);
    
    Event* e = event_bus_->poll();
    delete e;
    EXPECT_EQ(event_bus_->pending_count(), 1);
    
    e = event_bus_->poll();
    delete e;
    EXPECT_EQ(event_bus_->pending_count(), 0);
}

} // namespace testing
} // namespace astraeus
