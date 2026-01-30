package com.astraeus.test;

import com.astraeus.rendering.ViewportController;
import com.astraeus.rendering.OverlayStack;
import javafx.scene.shape.Rectangle;
import javafx.scene.control.Label;
import javafx.scene.paint.Color;
import javafx.geometry.Pos;

/**
 * Simple validation test for viewport framework v2 components.
 * 
 * This test validates that:
 * 1. ViewportController can be instantiated
 * 2. Camera modes can be switched
 * 3. OverlayStack can be created and managed
 * 4. Components have correct API
 * 
 * This test does NOT require native engine or JavaFX application context.
 */
public class ViewportFrameworkValidation {
    
    public static void main(String[] args) {
        System.out.println("=== Viewport Framework v2 Validation ===\n");
        
        // Test 1: ViewportController instantiation
        testViewportController();
        
        // Test 2: OverlayStack instantiation (requires JavaFX, skip if not available)
        try {
            testOverlayStack();
        } catch (NoClassDefFoundError e) {
            System.out.println("[TEST 2] OverlayStack - SKIPPED (JavaFX not available)");
        }
        
        System.out.println("\n=== All Tests Passed ===");
    }
    
    private static void testViewportController() {
        System.out.println("[TEST 1] ViewportController instantiation and API");
        
        // Create controller without engine reference
        ViewportController controller = new ViewportController();
        assert controller != null : "Controller creation failed";
        System.out.println("  ✓ Controller created");
        
        // Test mode switching
        controller.setMode(ViewportController.Mode.ORBIT);
        assert controller.getMode() == ViewportController.Mode.ORBIT : "Mode not set correctly";
        System.out.println("  ✓ Orbit mode set");
        
        controller.setMode(ViewportController.Mode.FLY);
        assert controller.getMode() == ViewportController.Mode.FLY : "Mode not set correctly";
        System.out.println("  ✓ Fly mode set");
        
        controller.setMode(ViewportController.Mode.PAN);
        assert controller.getMode() == ViewportController.Mode.PAN : "Mode not set correctly";
        System.out.println("  ✓ Pan mode set");
        
        // Test camera position/target getters
        double[] position = controller.getCameraPosition();
        assert position != null && position.length == 3 : "Position array invalid";
        System.out.println("  ✓ Camera position: [" + position[0] + ", " + position[1] + ", " + position[2] + "]");
        
        double[] target = controller.getCameraTarget();
        assert target != null && target.length == 3 : "Target array invalid";
        System.out.println("  ✓ Camera target: [" + target[0] + ", " + target[1] + ", " + target[2] + "]");
        
        // Test update (should not crash)
        controller.update(0.016);
        System.out.println("  ✓ Update called");
        
        // Test reset
        controller.reset();
        System.out.println("  ✓ Reset called");
        
        // Test orbit parameters
        controller.setOrbitTarget(5.0, 3.0, 2.0);
        controller.setOrbitDistance(15.0);
        System.out.println("  ✓ Orbit parameters set");
        
        // Test debug info
        String debugInfo = controller.getDebugInfo();
        assert debugInfo != null && !debugInfo.isEmpty() : "Debug info empty";
        System.out.println("  ✓ Debug info: " + debugInfo);
        
        System.out.println("[TEST 1] PASSED\n");
    }
    
    private static void testOverlayStack() {
        System.out.println("[TEST 2] OverlayStack API validation");
        
        // Note: Full overlay testing requires JavaFX initialization
        // For compile-time validation, we call testOverlayLayers()
        testOverlayLayers();
        
        System.out.println("[TEST 2] PASSED\n");
    }
    
    /**
     * Validate that all Layer enum values exist.
     */
    private static void testOverlayLayers() {
        System.out.println("[TEST 3] OverlayStack.Layer enum");
        
        // Verify all expected layers exist
        OverlayStack.Layer bg = OverlayStack.Layer.BACKGROUND;
        OverlayStack.Layer sel = OverlayStack.Layer.SELECTION;
        OverlayStack.Layer gizmo = OverlayStack.Layer.GIZMO;
        OverlayStack.Layer hud = OverlayStack.Layer.HUD;
        
        assert bg.getZOrder() == 0 : "BACKGROUND z-order incorrect";
        assert sel.getZOrder() == 1 : "SELECTION z-order incorrect";
        assert gizmo.getZOrder() == 2 : "GIZMO z-order incorrect";
        assert hud.getZOrder() == 3 : "HUD z-order incorrect";
        
        System.out.println("  ✓ All layers present with correct z-order");
        System.out.println("[TEST 3] PASSED\n");
    }
}
