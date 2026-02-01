package com.astraeus.native_api.layout;

import com.astraeus.generated.StructLayouts;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for validating generated constants match native API.
 * 
 * <p>Verifies:
 * - Schema version and hash
 * - Generated code availability
 * - Consistency with native definitions
 */
@DisplayName("Generated Code Validation Tests")
class ConstantsTest {
    
    @Test
    @DisplayName("Schema version should be defined")
    void schemaVersion_shouldBeDefined() {
        assertThat(StructLayouts.SCHEMA_VERSION)
            .as("Schema version should be defined")
            .isNotNull()
            .isNotEmpty();
    }
    
    @Test
    @DisplayName("Schema hash should be defined")
    void schemaHash_shouldBeDefined() {
        assertThat(StructLayouts.SCHEMA_HASH)
            .as("Schema hash should be defined")
            .isNotNull()
            .isNotEmpty();
    }
    
    @Test
    @DisplayName("Generation timestamp should be defined")
    void generationTimestamp_shouldBeDefined() {
        assertThat(StructLayouts.GENERATION_TIMESTAMP)
            .as("Generation timestamp should be defined")
            .isNotNull()
            .isNotEmpty();
    }
    
    @Test
    @DisplayName("StructLayouts class should be in correct package")
    void structLayoutsClass_shouldBeInCorrectPackage() {
        assertThat(StructLayouts.class.getPackageName())
            .isEqualTo("com.astraeus.generated");
    }
    
    // Add specific constant tests based on your native API
    // Example:
    // @Test
    // @DisplayName("RenderPass enum values should be valid")
    // void renderPassEnums_shouldBeValid() {
    //     assertThat(Constants.PASS_TYPE_OPAQUE).isEqualTo(0);
    //     assertThat(Constants.PASS_TYPE_TRANSPARENT).isEqualTo(1);
    // }
}
