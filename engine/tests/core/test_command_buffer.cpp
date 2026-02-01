#include <gtest/gtest.h>
#include "core/CommandBuffer.hpp"

namespace astraeus {
namespace testing {

class CommandBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        command_buffer_ = std::make_unique<CommandBuffer>();
    }
    
    void TearDown() override {
        command_buffer_.reset();
    }
    
    std::unique_ptr<CommandBuffer> command_buffer_;
};

/**
 * Test basic command submission and execution.
 */
TEST_F(CommandBufferTest, SubmitAndExecute) {
    int execute_count = 0;
    
    command_buffer_->submit([&]() {
        execute_count++;
    });
    
    EXPECT_EQ(execute_count, 0); // Not executed yet
    
    command_buffer_->execute_all();
    
    EXPECT_EQ(execute_count, 1);
}

/**
 * Test multiple commands are executed in order.
 */
TEST_F(CommandBufferTest, ExecutionOrder) {
    std::vector<int> execution_order;
    
    command_buffer_->submit([&]() { execution_order.push_back(1); });
    command_buffer_->submit([&]() { execution_order.push_back(2); });
    command_buffer_->submit([&]() { execution_order.push_back(3); });
    
    command_buffer_->execute_all();
    
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

/**
 * Test clearing command buffer.
 */
TEST_F(CommandBufferTest, Clear) {
    int execute_count = 0;
    
    command_buffer_->submit([&]() { execute_count++; });
    command_buffer_->submit([&]() { execute_count++; });
    
    command_buffer_->clear();
    command_buffer_->execute_all();
    
    EXPECT_EQ(execute_count, 0); // Commands were cleared
}

/**
 * Test command buffer size tracking.
 */
TEST_F(CommandBufferTest, SizeTracking) {
    EXPECT_EQ(command_buffer_->size(), 0);
    
    command_buffer_->submit([]() {});
    EXPECT_EQ(command_buffer_->size(), 1);
    
    command_buffer_->submit([]() {});
    EXPECT_EQ(command_buffer_->size(), 2);
    
    command_buffer_->execute_all();
    EXPECT_EQ(command_buffer_->size(), 0); // Cleared after execution
}

/**
 * Test empty command buffer execution doesn't crash.
 */
TEST_F(CommandBufferTest, ExecuteEmpty) {
    ASSERT_NO_THROW({
        command_buffer_->execute_all();
    });
}

/**
 * Test command buffer reuse after execution.
 */
TEST_F(CommandBufferTest, ReuseAfterExecution) {
    int count = 0;
    
    command_buffer_->submit([&]() { count++; });
    command_buffer_->execute_all();
    EXPECT_EQ(count, 1);
    
    command_buffer_->submit([&]() { count++; });
    command_buffer_->execute_all();
    EXPECT_EQ(count, 2);
}

/**
 * Test commands with captured state.
 */
TEST_F(CommandBufferTest, CapturedState) {
    std::string result;
    std::string message = "captured";
    
    command_buffer_->submit([&result, message]() {
        result = message;
    });
    
    command_buffer_->execute_all();
    
    EXPECT_EQ(result, "captured");
}

/**
 * Test large number of commands.
 */
TEST_F(CommandBufferTest, ManyCommands) {
    int sum = 0;
    const int num_commands = 1000;
    
    for (int i = 0; i < num_commands; ++i) {
        command_buffer_->submit([&sum, i]() {
            sum += i;
        });
    }
    
    command_buffer_->execute_all();
    
    int expected_sum = (num_commands * (num_commands - 1)) / 2;
    EXPECT_EQ(sum, expected_sum);
}

} // namespace testing
} // namespace astraeus
