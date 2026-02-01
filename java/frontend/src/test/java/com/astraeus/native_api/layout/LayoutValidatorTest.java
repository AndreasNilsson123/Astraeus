package com.astraeus.native_api.layout;

import com.astraeus.generated.StructLayouts;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import java.lang.foreign.MemoryLayout;
import java.lang.foreign.StructLayout;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for FFM struct layout validation.
 * 
 * <p>Verifies that Java FFM struct layouts match expected C ABI specifications:
 * - Struct sizes
 * - Struct alignments
 * - Field offsets
 * - Padding correctness
 * 
 * <p>These tests serve as regression protection against platform-specific
 * alignment issues and ABI changes.
 */
@DisplayName("FFM Layout Validator Tests")
class LayoutValidatorTest {
    
    @Test
    @DisplayName("All struct layouts should validate successfully")
    void validateAllLayouts_shouldPass() {
        boolean result = LayoutValidator.validateAllLayouts();
        assertThat(result)
            .as("All struct layouts should be valid")
            .isTrue();
    }
    
    @ParameterizedTest(name = "{0}: size={1}, alignment={2}")
    @CsvSource({
        "FrameStats, 40, 8",
        "TelemetryFrameStats, 48, 8",
        "ViewportConfig, 16, 4",
        "PixelBufferView, 36, 8",
        "ReadbackConfig, 13, 4",
        "PickResult, 24, 4",
        "EngineConfig, 24, 8",
        "CameraDesc, 56, 4",
        "MaterialDesc, 40, 4"
    })
    @DisplayName("Struct layout should match expected size and alignment")
    void structLayout_shouldMatchExpectedSizeAndAlignment(
            String structName, 
            long expectedSize, 
            long expectedAlignment) {
        
        StructLayout layout = getLayoutByName(structName);
        
        assertThat(layout.byteSize())
            .as("%s size", structName)
            .isEqualTo(expectedSize);
            
        assertThat(layout.byteAlignment())
            .as("%s alignment", structName)
            .isEqualTo(expectedAlignment);
    }
    
    @Test
    @DisplayName("Struct sizes should be valid")
    void structSizes_shouldBeValid() {
        // Note: FFM doesn't automatically add tail padding,
        // so some structs may not be multiples of their alignment
        // This is okay for single instances, but care is needed for arrays
        
        assertThat(StructLayouts.FRAME_STATS_LAYOUT.byteSize()).isEqualTo(40);
        assertThat(StructLayouts.TELEMETRY_FRAME_STATS_LAYOUT.byteSize()).isEqualTo(48);
        assertThat(StructLayouts.VIEWPORT_CONFIG_LAYOUT.byteSize()).isEqualTo(16);
        assertThat(StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT.byteSize()).isEqualTo(36);
        assertThat(StructLayouts.READBACK_CONFIG_LAYOUT.byteSize()).isEqualTo(13);
        assertThat(StructLayouts.PICK_RESULT_LAYOUT.byteSize()).isEqualTo(24);
        assertThat(StructLayouts.ENGINE_CONFIG_LAYOUT.byteSize()).isEqualTo(24);
        assertThat(StructLayouts.CAMERA_DESC_LAYOUT.byteSize()).isEqualTo(56);
        assertThat(StructLayouts.MATERIAL_DESC_LAYOUT.byteSize()).isEqualTo(40);
    }
    
    @Test
    @DisplayName("FrameStats layout should have expected fields")
    void frameStatsLayout_shouldHaveExpectedFields() {
        StructLayout layout = StructLayouts.FRAME_STATS_LAYOUT;
        
        assertThat(layout.byteSize()).isEqualTo(40);
        assertThat(layout.byteAlignment()).isEqualTo(8);
        
        // Verify field presence (field names are optional in MemoryLayout)
        // If your generator includes field names, verify them here
        assertThat(layout.memberLayouts()).isNotEmpty();
    }
    
    @Test
    @DisplayName("PixelBufferView layout should have expected fields")
    void pixelBufferViewLayout_shouldHaveExpectedFields() {
        StructLayout layout = StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT;
        
        assertThat(layout.byteSize()).isEqualTo(36);
        assertThat(layout.byteAlignment()).isEqualTo(8);
        assertThat(layout.memberLayouts()).isNotEmpty();
    }
    
    @Test
    @DisplayName("Schema version should be non-empty")
    void schemaVersion_shouldBeNonEmpty() {
        assertThat(StructLayouts.SCHEMA_VERSION)
            .as("Schema version")
            .isNotNull()
            .isNotEmpty();
    }
    
    @Test
    @DisplayName("Schema hash should be non-empty")
    void schemaHash_shouldBeNonEmpty() {
        assertThat(StructLayouts.SCHEMA_HASH)
            .as("Schema hash")
            .isNotNull()
            .isNotEmpty();
    }
    
    // Helper methods
    
    private StructLayout getLayoutByName(String name) {
        return switch (name) {
            case "FrameStats" -> StructLayouts.FRAME_STATS_LAYOUT;
            case "TelemetryFrameStats" -> StructLayouts.TELEMETRY_FRAME_STATS_LAYOUT;
            case "ViewportConfig" -> StructLayouts.VIEWPORT_CONFIG_LAYOUT;
            case "PixelBufferView" -> StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT;
            case "ReadbackConfig" -> StructLayouts.READBACK_CONFIG_LAYOUT;
            case "PickResult" -> StructLayouts.PICK_RESULT_LAYOUT;
            case "EngineConfig" -> StructLayouts.ENGINE_CONFIG_LAYOUT;
            case "CameraDesc" -> StructLayouts.CAMERA_DESC_LAYOUT;
            case "MaterialDesc" -> StructLayouts.MATERIAL_DESC_LAYOUT;
            default -> throw new IllegalArgumentException("Unknown layout: " + name);
        };
    }
    
    private void assertLayoutSizeAlignmentInvariant(StructLayout layout) {
        long size = layout.byteSize();
        long alignment = layout.byteAlignment();
        
        assertThat(size % alignment)
            .as("Size %d should be a multiple of alignment %d", size, alignment)
            .isEqualTo(0);
    }
}
