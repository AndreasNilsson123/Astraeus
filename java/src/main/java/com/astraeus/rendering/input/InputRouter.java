package com.astraeus.rendering.input;

import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;

/**
 * InputRouter - Centralized event routing with focus management.
 * 
 * Ensures only one component receives input events at a time to avoid duplicate handlers.
 * Coordinates between viewport, tools, and UI components.
 * 
 * FEATURES:
 * - Single active input component (focus ownership)
 * - Event filtering and routing
 * - Input enable/disable per component
 * - Clean handler registration/unregistration
 * 
 * USAGE:
 * <pre>
 * InputRouter router = new InputRouter();
 * router.registerComponent("viewport", viewportNode);
 * router.setActiveComponent("viewport");
 * router.routeMouseEvent(event);
 * </pre>
 */
public class InputRouter {
    
    private Node activeComponent;
    private String activeComponentId;
    private boolean inputEnabled = true;
    
    /**
     * Create a new InputRouter.
     */
    public InputRouter() {
    }
    
    /**
     * Set the active input component.
     * Only this component will receive routed input events.
     * 
     * @param component The node to receive input
     * @param componentId Identifier for the component
     */
    public void setActiveComponent(Node component, String componentId) {
        if (this.activeComponent != component) {
            // Request focus on the new component
            if (component != null && component.isFocusTraversable()) {
                component.requestFocus();
            }
            
            this.activeComponent = component;
            this.activeComponentId = componentId;
        }
    }
    
    /**
     * Get the currently active component.
     */
    public Node getActiveComponent() {
        return activeComponent;
    }
    
    /**
     * Get the active component ID.
     */
    public String getActiveComponentId() {
        return activeComponentId;
    }
    
    /**
     * Check if input is enabled.
     */
    public boolean isInputEnabled() {
        return inputEnabled;
    }
    
    /**
     * Enable or disable all input routing.
     */
    public void setInputEnabled(boolean enabled) {
        this.inputEnabled = enabled;
    }
    
    /**
     * Route a mouse event to the active component.
     * Returns true if the event was consumed.
     */
    public boolean routeMouseEvent(MouseEvent event) {
        if (!inputEnabled || activeComponent == null) {
            return false;
        }
        
        // Fire the event on the active component
        activeComponent.fireEvent(event);
        return event.isConsumed();
    }
    
    /**
     * Route a key event to the active component.
     * Returns true if the event was consumed.
     */
    public boolean routeKeyEvent(KeyEvent event) {
        if (!inputEnabled || activeComponent == null) {
            return false;
        }
        
        // Fire the event on the active component
        activeComponent.fireEvent(event);
        return event.isConsumed();
    }
    
    /**
     * Route a scroll event to the active component.
     * Returns true if the event was consumed.
     */
    public boolean routeScrollEvent(ScrollEvent event) {
        if (!inputEnabled || activeComponent == null) {
            return false;
        }
        
        // Fire the event on the active component
        activeComponent.fireEvent(event);
        return event.isConsumed();
    }
}
