#include <gtest/gtest.h>
#include "core/CommandBuffer.hpp"
#include "core/EngineContext.hpp"

namespace astraeus {
namespace testing {

class CommandBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        // CommandBuffer requires EngineContext, so we'll test command structures
    }
    
    void TearDown() override {
    }
};

/**
 * Test CreateEntityCommand structure.
 */
TEST_F(CommandBufferTest, CreateEntityCommand) {
    CreateEntityCommand cmd;
    EXPECT_EQ(cmd.type, CommandType::CreateEntity);
    
    uint32_t entity_id = 0;
    cmd.out_entity_id = &entity_id;
    
    ASSERT_NE(cmd.out_entity_id, nullptr);
}

/**
 * Test DestroyEntityCommand structure.
 */
TEST_F(CommandBufferTest, DestroyEntityCommand) {
    DestroyEntityCommand cmd(42);
    EXPECT_EQ(cmd.type, CommandType::DestroyEntity);
    EXPECT_EQ(cmd.entity_id, 42);
}

/**
 * Test SetTransformCommand structure.
 */
TEST_F(CommandBufferTest, SetTransformCommand) {
    SetTransformCommand cmd(100, 1.0f, 2.0f, 3.0f, 0, 0, 0, 1, 1, 1);
    
    EXPECT_EQ(cmd.type, CommandType::SetTransform);
    EXPECT_EQ(cmd.entity_id, 100);
    EXPECT_FLOAT_EQ(cmd.pos_x, 1.0f);
    EXPECT_FLOAT_EQ(cmd.pos_y, 2.0f);
    EXPECT_FLOAT_EQ(cmd.pos_z, 3.0f);
}

/**
 * Test AssignMeshCommand structure.
 */
TEST_F(CommandBufferTest, AssignMeshCommand) {
    AssignMeshCommand cmd(10, 20);
    
    EXPECT_EQ(cmd.type, CommandType::AssignMesh);
    EXPECT_EQ(cmd.entity_id, 10);
    EXPECT_EQ(cmd.mesh_id, 20);
}

/**
 * Test AssignMaterialCommand structure.
 */
TEST_F(CommandBufferTest, AssignMaterialCommand) {
    AssignMaterialCommand cmd(10, 30);
    
    EXPECT_EQ(cmd.type, CommandType::AssignMaterial);
    EXPECT_EQ(cmd.entity_id, 10);
    EXPECT_EQ(cmd.material_id, 30);
}

/**
 * Test command type enum values.
 */
TEST_F(CommandBufferTest, CommandTypeValues) {
    EXPECT_EQ(static_cast<uint32_t>(CommandType::CreateEntity), 0);
    EXPECT_EQ(static_cast<uint32_t>(CommandType::DestroyEntity), 1);
    EXPECT_EQ(static_cast<uint32_t>(CommandType::SetTransform), 2);
    EXPECT_EQ(static_cast<uint32_t>(CommandType::AssignMesh), 3);
    EXPECT_EQ(static_cast<uint32_t>(CommandType::AssignMaterial), 4);
}

/**
 * Test polymorphic command base.
 */
TEST_F(CommandBufferTest, PolymorphicCommand) {
    Command* cmd1 = new CreateEntityCommand();
    EXPECT_EQ(cmd1->type, CommandType::CreateEntity);
    delete cmd1;
    
    Command* cmd2 = new DestroyEntityCommand(99);
    EXPECT_EQ(cmd2->type, CommandType::DestroyEntity);
    EXPECT_EQ(cmd2->entity_id, 99);
    delete cmd2;
}

} // namespace testing
} // namespace astraeus
