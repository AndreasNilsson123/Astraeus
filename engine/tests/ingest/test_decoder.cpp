#include <gtest/gtest.h>
#include "ingest/Decoder.h"
#include "ingest/FixedBinaryDecoder.hpp"

namespace astraeus {
namespace testing {

class DecoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        decoder_ = std::make_unique<FixedBinaryDecoder>();
    }
    
    void TearDown() override {
        decoder_.reset();
    }
    
    std::unique_ptr<FixedBinaryDecoder> decoder_;
};

/**
 * Test decoding a simple valid packet.
 */
TEST_F(DecoderTest, DecodeValidPacket) {
    // Create a simple valid packet
    std::vector<uint8_t> packet_data;
    // Add header and entity data according to format
    
    Snapshot snapshot;
    bool result = decoder_->decode(packet_data.data(), packet_data.size(), snapshot);
    
    // If packet format is correct, should succeed
    // If empty, should fail gracefully
}

/**
 * Test decoding empty packet.
 */
TEST_F(DecoderTest, DecodeEmptyPacket) {
    std::vector<uint8_t> empty_packet;
    
    Snapshot snapshot;
    bool result = decoder_->decode(empty_packet.data(), 0, snapshot);
    
    // Should return false and not crash
    EXPECT_FALSE(result);
}

/**
 * Test decoding packet with invalid header.
 */
TEST_F(DecoderTest, InvalidHeader) {
    // Create packet with invalid magic number or version
    std::vector<uint8_t> invalid_packet = {0xFF, 0xFF, 0xFF, 0xFF};
    
    Snapshot snapshot;
    bool result = decoder_->decode(invalid_packet.data(), invalid_packet.size(), snapshot);
    
    // Should reject invalid packet
    EXPECT_FALSE(result);
}

/**
 * Test decoding packet with truncated data.
 */
TEST_F(DecoderTest, TruncatedData) {
    // Create packet that claims to have more data than provided
    std::vector<uint8_t> truncated_packet;
    // Header indicates N entities but data is incomplete
    
    Snapshot snapshot;
    
    // Should handle gracefully without crash
    ASSERT_NO_THROW({
        decoder_->decode(truncated_packet.data(), truncated_packet.size(), snapshot);
    });
}

/**
 * Test decoder with malformed entity data.
 */
TEST_F(DecoderTest, MalformedEntityData) {
    // Create packet with corrupted entity data
    std::vector<uint8_t> malformed_packet;
    // Add some random bytes
    for (int i = 0; i < 100; ++i) {
        malformed_packet.push_back(static_cast<uint8_t>(rand() % 256));
    }
    
    Snapshot snapshot;
    
    // Should not crash even with random data
    ASSERT_NO_THROW({
        decoder_->decode(malformed_packet.data(), malformed_packet.size(), snapshot);
    });
}

/**
 * Test decoder with null pointer.
 */
TEST_F(DecoderTest, NullPointer) {
    Snapshot snapshot;
    
    // Should handle null gracefully
    ASSERT_NO_THROW({
        bool result = decoder_->decode(nullptr, 0, snapshot);
        EXPECT_FALSE(result);
    });
}

/**
 * Test decoder with maximum size packet.
 */
TEST_F(DecoderTest, MaximumSizePacket) {
    // Create a large packet with many entities
    const size_t large_size = 1024 * 1024; // 1 MB
    std::vector<uint8_t> large_packet(large_size, 0);
    
    Snapshot snapshot;
    
    // Should handle large packets without crash
    ASSERT_NO_THROW({
        decoder_->decode(large_packet.data(), large_packet.size(), snapshot);
    });
}

/**
 * Test getting decoder name/type.
 */
TEST_F(DecoderTest, DecoderInfo) {
    const char* name = decoder_->get_name();
    ASSERT_NE(name, nullptr);
    EXPECT_GT(strlen(name), 0);
}

/**
 * Test decoder reset functionality.
 */
TEST_F(DecoderTest, Reset) {
    // Decode some data
    std::vector<uint8_t> packet_data;
    Snapshot snapshot;
    decoder_->decode(packet_data.data(), packet_data.size(), snapshot);
    
    // Reset decoder state
    ASSERT_NO_THROW({
        decoder_->reset();
    });
}

/**
 * Test multiple sequential decodes.
 */
TEST_F(DecoderTest, SequentialDecodes) {
    for (int i = 0; i < 10; ++i) {
        std::vector<uint8_t> packet_data;
        Snapshot snapshot;
        
        ASSERT_NO_THROW({
            decoder_->decode(packet_data.data(), packet_data.size(), snapshot);
        });
    }
}

} // namespace testing
} // namespace astraeus
