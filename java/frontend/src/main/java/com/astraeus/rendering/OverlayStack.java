package com.astraeus.rendering;

import javafx.scene.layout.Pane;
import javafx.scene.layout.StackPane;
import javafx.scene.Node;
import javafx.geometry.Pos;

import java.util.HashMap;
import java.util.Map;

/**
 * OverlayStack manages layered UI overlays for a viewport.
 * 
 * PERFORMANCE:
 * - NO per-frame allocations
 * - Efficient layer management with fixed structure
 * - Minimal layout overhead
 * 
 * FEATURES:
 * - Named overlay layers with Z-order
 * - HUD text layer
 * - Selection/gizmo layers
 * - Telemetry overlay integration
 * - Independent visibility control per layer
 * 
 * USAGE:
 * <pre>
 * OverlayStack overlayStack = new OverlayStack();
 * 
 * // Add overlay layers
 * overlayStack.addOverlay("telemetry", telemetryOverlay, OverlayStack.Layer.HUD);
 * overlayStack.addOverlay("selection", selectionBox, OverlayStack.Layer.SELECTION);
 * 
 * // Toggle visibility
 * overlayStack.setOverlayVisible("telemetry", true);
 * 
 * // Add to viewport
 * viewport.getChildren().add(overlayStack);
 * </pre>
 */
public class OverlayStack extends StackPane {
    
    /**
     * Pre-defined overlay layers with Z-order (bottom to top).
     */
    public enum Layer {
        /** Background layer (lowest, behind viewport image) */
        BACKGROUND(0),
        /** Selection indicators and highlights */
        SELECTION(1),
        /** Gizmo layer for 3D manipulation widgets */
        GIZMO(2),
        /** HUD text and telemetry overlays (highest) */
        HUD(3);
        
        private final int zOrder;
        
        Layer(int zOrder) {
            this.zOrder = zOrder;
        }
        
        public int getZOrder() {
            return zOrder;
        }
    }
    
    /**
     * Internal container for each layer.
     */
    private static class LayerContainer extends Pane {
        private final Layer layer;
        
        LayerContainer(Layer layer) {
            this.layer = layer;
            setMouseTransparent(true); // By default, don't block input
            setPickOnBounds(false);    // Only pick on actual content
        }
        
        Layer getLayer() {
            return layer;
        }
    }
    
    // Layer containers (created once, never reallocated)
    private final Map<Layer, LayerContainer> layerContainers;
    
    // Named overlays for easy access
    private final Map<String, Node> namedOverlays;
    
    /**
     * Create a new OverlayStack.
     */
    public OverlayStack() {
        layerContainers = new HashMap<>();
        namedOverlays = new HashMap<>();
        
        // Create all layer containers upfront
        for (Layer layer : Layer.values()) {
            LayerContainer container = new LayerContainer(layer);
            layerContainers.put(layer, container);
            getChildren().add(container);
        }
        
        // Configure stack pane
        setPickOnBounds(false);  // Only pick on actual overlay content
        
        System.out.println("[OverlayStack] Created with " + Layer.values().length + " layers");
    }
    
    /**
     * Add an overlay to a specific layer.
     * 
     * @param name Unique name for this overlay
     * @param overlay The JavaFX node to add
     * @param layer The layer to add it to
     */
    public void addOverlay(String name, Node overlay, Layer layer) {
        if (namedOverlays.containsKey(name)) {
            System.err.println("[OverlayStack] Warning: Overlay '" + name + "' already exists, replacing");
            removeOverlay(name);
        }
        
        LayerContainer container = layerContainers.get(layer);
        container.getChildren().add(overlay);
        namedOverlays.put(name, overlay);
        
        System.out.println("[OverlayStack] Added overlay '" + name + "' to " + layer + " layer");
    }
    
    /**
     * Add an overlay with default positioning (centered).
     */
    public void addOverlay(String name, Node overlay, Layer layer, Pos alignment) {
        addOverlay(name, overlay, layer);
        StackPane.setAlignment(overlay, alignment);
    }
    
    /**
     * Remove an overlay by name.
     * 
     * @param name Name of overlay to remove
     * @return true if overlay was found and removed
     */
    public boolean removeOverlay(String name) {
        Node overlay = namedOverlays.remove(name);
        if (overlay != null) {
            // Find and remove from parent container
            for (LayerContainer container : layerContainers.values()) {
                if (container.getChildren().remove(overlay)) {
                    System.out.println("[OverlayStack] Removed overlay '" + name + "'");
                    return true;
                }
            }
        }
        return false;
    }
    
    /**
     * Get an overlay by name.
     * 
     * @param name Overlay name
     * @return The overlay node, or null if not found
     */
    public Node getOverlay(String name) {
        return namedOverlays.get(name);
    }
    
    /**
     * Check if an overlay exists.
     * 
     * @param name Overlay name
     * @return true if overlay exists
     */
    public boolean hasOverlay(String name) {
        return namedOverlays.containsKey(name);
    }
    
    /**
     * Set overlay visibility by name.
     * 
     * @param name Overlay name
     * @param visible Visibility state
     */
    public void setOverlayVisible(String name, boolean visible) {
        Node overlay = namedOverlays.get(name);
        if (overlay != null) {
            overlay.setVisible(visible);
        } else {
            System.err.println("[OverlayStack] Warning: Overlay '" + name + "' not found");
        }
    }
    
    /**
     * Toggle overlay visibility by name.
     * 
     * @param name Overlay name
     */
    public void toggleOverlay(String name) {
        Node overlay = namedOverlays.get(name);
        if (overlay != null) {
            overlay.setVisible(!overlay.isVisible());
        }
    }
    
    /**
     * Check if an overlay is visible.
     * 
     * @param name Overlay name
     * @return true if visible, false if hidden or not found
     */
    public boolean isOverlayVisible(String name) {
        Node overlay = namedOverlays.get(name);
        return overlay != null && overlay.isVisible();
    }
    
    /**
     * Set visibility for entire layer.
     * 
     * @param layer Layer to show/hide
     * @param visible Visibility state
     */
    public void setLayerVisible(Layer layer, boolean visible) {
        LayerContainer container = layerContainers.get(layer);
        if (container != null) {
            container.setVisible(visible);
        }
    }
    
    /**
     * Get layer container for direct manipulation.
     * Use with caution - prefer named overlay methods.
     * 
     * @param layer Layer to get
     * @return Layer container pane
     */
    public Pane getLayerContainer(Layer layer) {
        return layerContainers.get(layer);
    }
    
    /**
     * Clear all overlays from a specific layer.
     * 
     * @param layer Layer to clear
     */
    public void clearLayer(Layer layer) {
        LayerContainer container = layerContainers.get(layer);
        if (container != null) {
            // Remove from named overlays map
            namedOverlays.entrySet().removeIf(entry -> 
                container.getChildren().contains(entry.getValue())
            );
            
            container.getChildren().clear();
            System.out.println("[OverlayStack] Cleared " + layer + " layer");
        }
    }
    
    /**
     * Clear all overlays from all layers.
     */
    public void clearAll() {
        for (Layer layer : Layer.values()) {
            clearLayer(layer);
        }
        namedOverlays.clear();
        System.out.println("[OverlayStack] Cleared all overlays");
    }
    
    /**
     * Get number of overlays currently displayed.
     */
    public int getOverlayCount() {
        return namedOverlays.size();
    }
    
    /**
     * Get debug information about current overlays.
     */
    public String getDebugInfo() {
        StringBuilder sb = new StringBuilder();
        sb.append("OverlayStack: ").append(namedOverlays.size()).append(" overlays\n");
        
        for (Layer layer : Layer.values()) {
            LayerContainer container = layerContainers.get(layer);
            int count = container.getChildren().size();
            if (count > 0) {
                sb.append("  ").append(layer).append(": ").append(count).append(" items\n");
            }
        }
        
        return sb.toString();
    }
}
