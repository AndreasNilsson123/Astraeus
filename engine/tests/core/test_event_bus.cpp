#include <gtest/gtest.h>
#include "core/EventBus.hpp"

namespace astraeus {
namespace testing {

struct TestEvent {
    int value;
    std::string message;
};

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        event_bus_ = std::make_unique<EventBus>();
    }
    
    void TearDown() override {
        event_bus_.reset();
    }
    
    std::unique_ptr<EventBus> event_bus_;
};

/**
 * Test basic event subscription and publishing.
 */
TEST_F(EventBusTest, SubscribeAndPublish) {
    int received_count = 0;
    TestEvent received_event{0, ""};
    
    event_bus_->subscribe<TestEvent>([&](const TestEvent& event) {
        received_count++;
        received_event = event;
    });
    
    TestEvent sent_event{42, "test message"};
    event_bus_->publish(sent_event);
    
    EXPECT_EQ(received_count, 1);
    EXPECT_EQ(received_event.value, 42);
    EXPECT_EQ(received_event.message, "test message");
}

/**
 * Test multiple subscribers for the same event type.
 */
TEST_F(EventBusTest, MultipleSubscribers) {
    int count1 = 0, count2 = 0, count3 = 0;
    
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { count1++; });
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { count2++; });
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { count3++; });
    
    event_bus_->publish(TestEvent{1, "test"});
    
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(count3, 1);
}

/**
 * Test publishing multiple events.
 */
TEST_F(EventBusTest, MultipleEvents) {
    int sum = 0;
    
    event_bus_->subscribe<TestEvent>([&](const TestEvent& event) {
        sum += event.value;
    });
    
    event_bus_->publish(TestEvent{10, "first"});
    event_bus_->publish(TestEvent{20, "second"});
    event_bus_->publish(TestEvent{30, "third"});
    
    EXPECT_EQ(sum, 60);
}

/**
 * Test unsubscribing from events.
 */
TEST_F(EventBusTest, Unsubscribe) {
    int count = 0;
    
    auto handle = event_bus_->subscribe<TestEvent>([&](const TestEvent&) {
        count++;
    });
    
    event_bus_->publish(TestEvent{1, "test1"});
    EXPECT_EQ(count, 1);
    
    event_bus_->unsubscribe(handle);
    
    event_bus_->publish(TestEvent{2, "test2"});
    EXPECT_EQ(count, 1); // Should not increment
}

/**
 * Test that events are delivered in subscription order.
 */
TEST_F(EventBusTest, DeliveryOrder) {
    std::vector<int> order;
    
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { order.push_back(1); });
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { order.push_back(2); });
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { order.push_back(3); });
    
    event_bus_->publish(TestEvent{0, "test"});
    
    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

/**
 * Test publishing with no subscribers doesn't crash.
 */
TEST_F(EventBusTest, PublishWithNoSubscribers) {
    ASSERT_NO_THROW({
        event_bus_->publish(TestEvent{42, "test"});
    });
}

/**
 * Test clearing all subscribers.
 */
TEST_F(EventBusTest, ClearAllSubscribers) {
    int count = 0;
    
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { count++; });
    event_bus_->subscribe<TestEvent>([&](const TestEvent&) { count++; });
    
    event_bus_->publish(TestEvent{1, "test1"});
    EXPECT_EQ(count, 2);
    
    event_bus_->clear();
    
    event_bus_->publish(TestEvent{2, "test2"});
    EXPECT_EQ(count, 2); // Should not increment
}

} // namespace testing
} // namespace astraeus
