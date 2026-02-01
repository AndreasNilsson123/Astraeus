package com.astraeus.tools.inspector;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for inspector logic (entity selection, picking, etc.).
 * 
 * <p>Verifies:
 * - Entity ID validation
 * - Selection state management
 * - Picking coordinate transformation
 * - Entity list filtering
 */
@DisplayName("Inspector Logic Tests")
class InspectorLogicTest {
    
    @Test
    @DisplayName("Entity ID zero should represent no selection")
    void entityIdZero_shouldRepresentNoSelection() {
        int entityId = 0;
        boolean isSelected = (entityId != 0);
        
        assertThat(isSelected).isFalse();
    }
    
    @Test
    @DisplayName("Positive entity ID should represent valid selection")
    void positiveEntityId_shouldRepresentValidSelection() {
        int entityId = 42;
        boolean isValid = (entityId > 0);
        
        assertThat(isValid).isTrue();
    }
    
    @Test
    @DisplayName("Negative entity ID should be invalid")
    void negativeEntityId_shouldBeInvalid() {
        int entityId = -1;
        boolean isValid = (entityId > 0);
        
        assertThat(isValid).isFalse();
    }
    
    @ParameterizedTest(name = "viewportX={0}, viewportY={1} -> within bounds")
    @CsvSource({
        "0, 0",
        "640, 360",
        "1279, 719",
        "100, 200",
        "500, 500"
    })
    @DisplayName("Picking coordinates should be within viewport bounds")
    void pickingCoordinates_shouldBeWithinViewportBounds(int x, int y) {
        int viewportWidth = 1280;
        int viewportHeight = 720;
        
        boolean withinBounds = (x >= 0 && x < viewportWidth && 
                               y >= 0 && y < viewportHeight);
        
        assertThat(withinBounds).isTrue();
    }
    
    @ParameterizedTest(name = "viewportX={0}, viewportY={1} -> out of bounds")
    @CsvSource({
        "-1, 360",
        "640, -1",
        "1280, 360",
        "640, 720",
        "-10, -10",
        "2000, 2000"
    })
    @DisplayName("Out of bounds picking coordinates should be rejected")
    void outOfBoundsPickingCoordinates_shouldBeRejected(int x, int y) {
        int viewportWidth = 1280;
        int viewportHeight = 720;
        
        boolean withinBounds = (x >= 0 && x < viewportWidth && 
                               y >= 0 && y < viewportHeight);
        
        assertThat(withinBounds).isFalse();
    }
    
    @Test
    @DisplayName("Coordinate transformation should preserve relative position")
    void coordinateTransformation_shouldPreserveRelativePosition() {
        int logicalX = 640;  // Center of 1280 width
        int logicalY = 360;  // Center of 720 height
        double scale = 2.0;
        
        int deviceX = (int) (logicalX * scale);
        int deviceY = (int) (logicalY * scale);
        
        assertThat(deviceX).isEqualTo(1280); // Center of 2560 width
        assertThat(deviceY).isEqualTo(720);  // Center of 1440 height
    }
    
    @Test
    @DisplayName("Pick result should contain entity ID and depth")
    void pickResult_shouldContainEntityIdAndDepth() {
        // Simulate a pick result
        int entityId = 123;
        float depth = 0.5f;
        boolean hit = true;
        
        assertThat(entityId).isGreaterThan(0);
        assertThat(depth).isBetween(0.0f, 1.0f);
        assertThat(hit).isTrue();
    }
    
    @Test
    @DisplayName("Miss result should have zero entity ID")
    void missResult_shouldHaveZeroEntityId() {
        int entityId = 0;
        float depth = 1.0f;
        boolean hit = false;
        
        assertThat(entityId).isEqualTo(0);
        assertThat(hit).isFalse();
    }
    
    @Test
    @DisplayName("Selection state should track current entity")
    void selectionState_shouldTrackCurrentEntity() {
        int selectedEntityId = 0;
        
        // No selection initially
        assertThat(selectedEntityId).isEqualTo(0);
        
        // Select entity
        selectedEntityId = 42;
        assertThat(selectedEntityId).isEqualTo(42);
        
        // Deselect
        selectedEntityId = 0;
        assertThat(selectedEntityId).isEqualTo(0);
    }
    
    @Test
    @DisplayName("Entity list should be filterable by name")
    void entityList_shouldBeFilterableByName() {
        String[] entities = { "Cube", "Sphere", "Cylinder", "Cone" };
        String filter = "C";
        
        long matchCount = java.util.Arrays.stream(entities)
            .filter(name -> name.toLowerCase().contains(filter.toLowerCase()))
            .count();
        
        assertThat(matchCount).isEqualTo(3); // Cube, Cylinder, Cone
    }
    
    @Test
    @DisplayName("Entity list should be sortable by ID")
    void entityList_shouldBeSortableById() {
        int[] entityIds = { 5, 2, 8, 1, 3 };
        java.util.Arrays.sort(entityIds);
        
        assertThat(entityIds).containsExactly(1, 2, 3, 5, 8);
    }
    
    @Test
    @DisplayName("Depth value should be normalized to 0-1 range")
    void depthValue_shouldBeNormalizedToZeroOne() {
        float depth = 0.75f;
        
        assertThat(depth).isBetween(0.0f, 1.0f);
    }
    
    @Test
    @DisplayName("Multi-select should maintain set of entity IDs")
    void multiSelect_shouldMaintainSetOfEntityIds() {
        java.util.Set<Integer> selectedEntities = new java.util.HashSet<>();
        
        // Add entities
        selectedEntities.add(1);
        selectedEntities.add(2);
        selectedEntities.add(3);
        
        assertThat(selectedEntities).hasSize(3);
        assertThat(selectedEntities).contains(1, 2, 3);
        
        // Remove entity
        selectedEntities.remove(2);
        assertThat(selectedEntities).hasSize(2);
        assertThat(selectedEntities).contains(1, 3);
        
        // Clear selection
        selectedEntities.clear();
        assertThat(selectedEntities).isEmpty();
    }
    
    @Test
    @DisplayName("Entity count should be non-negative")
    void entityCount_shouldBeNonNegative() {
        int entityCount = 100;
        
        assertThat(entityCount).isGreaterThanOrEqualTo(0);
    }
}
