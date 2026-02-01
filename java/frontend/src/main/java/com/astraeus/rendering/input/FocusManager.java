package com.astraeus.rendering.input;

import javafx.scene.Node;

import java.util.HashMap;
import java.util.Map;

/**
 * FocusManager - Manages input focus for components.
 * 
 * Ensures only one component owns input focus at a time.
 * Prevents duplicate event handlers and input conflicts.
 * 
 * FEATURES:
 * - Component registration by ID
 * - Focus acquisition/release
 * - Focus change notifications
 * - Automatic focus request
 * 
 * USAGE:
 * <pre>
 * FocusManager manager = new FocusManager();
 * manager.registerComponent("viewport", viewportNode);
 * manager.requestFocus("viewport");
 * 
 * if (manager.hasFocus("viewport")) {
 *     // Handle input
 * }
 * </pre>
 */
public class FocusManager {
    
    private final Map<String, Node> components = new HashMap<>();
    private String focusedComponentId;
    private Node focusedComponent;
    
    /**
     * Create a new FocusManager.
     */
    public FocusManager() {
    }
    
    /**
     * Register a component that can receive focus.
     * 
     * @param componentId Unique identifier for the component
     * @param component The node that can receive focus
     */
    public void registerComponent(String componentId, Node component) {
        if (componentId == null || component == null) {
            throw new IllegalArgumentException("Component ID and node cannot be null");
        }
        
        components.put(componentId, component);
        
        // Ensure the component is focusable
        if (!component.isFocusTraversable()) {
            component.setFocusTraversable(true);
        }
    }
    
    /**
     * Unregister a component.
     * If the component currently has focus, focus is released.
     * 
     * @param componentId The component ID to unregister
     */
    public void unregisterComponent(String componentId) {
        if (componentId == null) {
            return;
        }
        
        components.remove(componentId);
        
        // Release focus if this component had it
        if (componentId.equals(focusedComponentId)) {
            releaseFocus();
        }
    }
    
    /**
     * Request focus for a component.
     * The component must be registered first.
     * 
     * @param componentId The component ID to focus
     * @return true if focus was granted, false otherwise
     */
    public boolean requestFocus(String componentId) {
        if (componentId == null) {
            return false;
        }
        
        Node component = components.get(componentId);
        if (component == null) {
            System.err.println("[FocusManager] Component not registered: " + componentId);
            return false;
        }
        
        // Release previous focus
        if (focusedComponentId != null && !focusedComponentId.equals(componentId)) {
            releaseFocus();
        }
        
        // Grant new focus
        focusedComponentId = componentId;
        focusedComponent = component;
        component.requestFocus();
        
        return true;
    }
    
    /**
     * Release focus from the currently focused component.
     */
    public void releaseFocus() {
        focusedComponentId = null;
        focusedComponent = null;
    }
    
    /**
     * Check if a component has focus.
     * 
     * @param componentId The component ID to check
     * @return true if the component has focus
     */
    public boolean hasFocus(String componentId) {
        return componentId != null && componentId.equals(focusedComponentId);
    }
    
    /**
     * Get the ID of the currently focused component.
     * 
     * @return The focused component ID, or null if no component has focus
     */
    public String getFocusedComponentId() {
        return focusedComponentId;
    }
    
    /**
     * Get the currently focused component node.
     * 
     * @return The focused node, or null if no component has focus
     */
    public Node getFocusedComponent() {
        return focusedComponent;
    }
    
    /**
     * Check if any component has focus.
     * 
     * @return true if a component is focused
     */
    public boolean isAnyFocused() {
        return focusedComponentId != null;
    }
}
