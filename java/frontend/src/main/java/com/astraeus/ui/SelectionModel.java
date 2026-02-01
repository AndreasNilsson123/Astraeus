package com.astraeus.ui;

import javafx.beans.property.ObjectProperty;
import javafx.beans.property.ReadOnlyObjectProperty;
import javafx.beans.property.SimpleObjectProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

/**
 * Selection model for scene entities.
 * Manages single and multi-selection state and notifies listeners of changes.
 * Can be shared between viewport, outliner, and inspector panes.
 */
public class SelectionModel {
    
    // Currently selected entity ID (0 = no selection)
    private final ObjectProperty<Integer> selectedEntityId;
    
    // List of selected entity IDs for multi-selection
    private final ObservableList<Integer> selectedEntities;
    
    // Selection change listeners
    private final ObservableList<SelectionChangeListener> listeners;
    
    public SelectionModel() {
        this.selectedEntityId = new SimpleObjectProperty<>(0);
        this.selectedEntities = FXCollections.observableArrayList();
        this.listeners = FXCollections.observableArrayList();
        
        // Automatically update single selection when list changes
        selectedEntities.addListener((javafx.collections.ListChangeListener.Change<? extends Integer> change) -> {
            if (selectedEntities.isEmpty()) {
                selectedEntityId.set(0);
            } else {
                selectedEntityId.set(selectedEntities.get(selectedEntities.size() - 1));
            }
            notifyListeners();
        });
    }
    
    /**
     * Select a single entity (clears previous selection).
     */
    public void select(int entityId) {
        if (entityId == 0) {
            clearSelection();
            return;
        }
        
        selectedEntities.clear();
        selectedEntities.add(entityId);
    }
    
    /**
     * Add an entity to the selection.
     */
    public void addToSelection(int entityId) {
        if (entityId == 0 || selectedEntities.contains(entityId)) {
            return;
        }
        selectedEntities.add(entityId);
    }
    
    /**
     * Remove an entity from the selection.
     */
    public void removeFromSelection(int entityId) {
        selectedEntities.remove(Integer.valueOf(entityId));
    }
    
    /**
     * Toggle entity selection state.
     */
    public void toggle(int entityId) {
        if (selectedEntities.contains(entityId)) {
            removeFromSelection(entityId);
        } else {
            addToSelection(entityId);
        }
    }
    
    /**
     * Clear all selections.
     */
    public void clearSelection() {
        selectedEntities.clear();
    }
    
    /**
     * Check if an entity is selected.
     */
    public boolean isSelected(int entityId) {
        return selectedEntities.contains(entityId);
    }
    
    /**
     * Get the primary selected entity ID (most recent).
     */
    public int getSelectedEntityId() {
        return selectedEntityId.get();
    }
    
    /**
     * Get the selected entity ID property (for binding).
     */
    public ReadOnlyObjectProperty<Integer> selectedEntityIdProperty() {
        return selectedEntityId;
    }
    
    /**
     * Get the list of all selected entity IDs.
     */
    public ObservableList<Integer> getSelectedEntities() {
        return FXCollections.unmodifiableObservableList(selectedEntities);
    }
    
    /**
     * Add a selection change listener.
     */
    public void addSelectionChangeListener(SelectionChangeListener listener) {
        listeners.add(listener);
    }
    
    /**
     * Remove a selection change listener.
     */
    public void removeSelectionChangeListener(SelectionChangeListener listener) {
        listeners.remove(listener);
    }
    
    /**
     * Notify all listeners of selection change.
     */
    private void notifyListeners() {
        for (SelectionChangeListener listener : listeners) {
            listener.onSelectionChanged(getSelectedEntityId(), getSelectedEntities());
        }
    }
    
    /**
     * Listener interface for selection changes.
     */
    @FunctionalInterface
    public interface SelectionChangeListener {
        void onSelectionChanged(int primaryEntityId, ObservableList<Integer> allSelectedEntities);
    }
}
