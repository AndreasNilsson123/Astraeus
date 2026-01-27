package com.astraeus.tools;

import com.astraeus.native_api.PickingView;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;

/**
 * Scene inspector tool for viewing entity hierarchy and properties.
 * Displays metadata for selected entities from picking operations.
 */
public class SceneInspector extends VBox {
    
    private int selectedEntityId = 0;
    private Label entityIdLabel;
    private Label worldPosLabel;
    private Label depthLabel;
    private TextArea metadataArea;
    
    public SceneInspector() {
        super(10);
        setPadding(new Insets(10));
        setStyle("-fx-background-color: #f5f5f5; -fx-border-color: #cccccc; -fx-border-width: 1;");
        
        // Title
        Label titleLabel = new Label("Entity Inspector");
        titleLabel.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
        
        // Entity info section
        VBox infoBox = new VBox(5);
        infoBox.setPadding(new Insets(10));
        infoBox.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        entityIdLabel = new Label("No entity selected");
        entityIdLabel.setStyle("-fx-font-weight: bold;");
        
        worldPosLabel = new Label("World Position: -");
        depthLabel = new Label("Depth: -");
        
        infoBox.getChildren().addAll(
            new Label("Selected Entity:"),
            entityIdLabel,
            new Separator(),
            worldPosLabel,
            depthLabel
        );
        
        // Metadata section
        Label metadataLabel = new Label("Metadata:");
        metadataLabel.setStyle("-fx-font-weight: bold;");
        
        metadataArea = new TextArea();
        metadataArea.setEditable(false);
        metadataArea.setPrefRowCount(8);
        metadataArea.setWrapText(true);
        metadataArea.setStyle("-fx-font-family: monospace; -fx-font-size: 11;");
        metadataArea.setText("Click an entity in the viewport to view its properties.");
        
        VBox metadataBox = new VBox(5);
        metadataBox.setPadding(new Insets(10));
        metadataBox.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        metadataBox.getChildren().addAll(metadataLabel, metadataArea);
        
        // Add all components
        getChildren().addAll(titleLabel, infoBox, metadataBox);
        
        // Set minimum width
        setMinWidth(250);
        setPrefWidth(300);
    }
    
    /**
     * Update inspector with picking result.
     * 
     * @param pickResult Result from picking operation
     */
    public void updateSelection(PickingView pickResult) {
        if (pickResult == null || !pickResult.hasValidEntity()) {
            clearSelection();
            return;
        }
        
        selectedEntityId = pickResult.getEntityId();
        
        // Update labels
        entityIdLabel.setText("Entity ID: " + selectedEntityId);
        worldPosLabel.setText(String.format("World Position: (%.2f, %.2f, %.2f)", 
                                           pickResult.getWorldX(),
                                           pickResult.getWorldY(),
                                           pickResult.getWorldZ()));
        depthLabel.setText(String.format("Depth: %.3f", pickResult.getDepth()));
        
        // Update metadata
        StringBuilder metadata = new StringBuilder();
        metadata.append("=== Entity ").append(selectedEntityId).append(" ===\n\n");
        metadata.append("Type: Unknown\n");
        metadata.append("Visible: Yes\n");
        metadata.append("Pickable: Yes\n\n");
        
        metadata.append("Transform:\n");
        metadata.append(String.format("  Position: (%.3f, %.3f, %.3f)\n",
                                     pickResult.getWorldX(),
                                     pickResult.getWorldY(),
                                     pickResult.getWorldZ()));
        metadata.append("  Rotation: (0.00, 0.00, 0.00)\n");
        metadata.append("  Scale: (1.00, 1.00, 1.00)\n\n");
        
        metadata.append("Rendering:\n");
        metadata.append(String.format("  Depth: %.3f\n", pickResult.getDepth()));
        metadata.append("  Color: Unknown\n");
        metadata.append("  Trail: Unknown\n\n");
        
        metadata.append("Selection:\n");
        metadata.append("  Click to deselect\n");
        
        metadataArea.setText(metadata.toString());
    }
    
    /**
     * Clear the current selection.
     */
    public void clearSelection() {
        selectedEntityId = 0;
        entityIdLabel.setText("No entity selected");
        worldPosLabel.setText("World Position: -");
        depthLabel.setText("Depth: -");
        metadataArea.setText("Click an entity in the viewport to view its properties.");
    }
    
    /**
     * Get selected entity ID.
     */
    public int getSelectedEntity() {
        return selectedEntityId;
    }
    
    /**
     * Set selected entity by ID (without pick data).
     */
    public void setSelectedEntity(int entityId) {
        if (entityId == 0) {
            clearSelection();
            return;
        }
        
        selectedEntityId = entityId;
        entityIdLabel.setText("Entity ID: " + entityId);
        metadataArea.setText("Entity selected by ID.\nClick entity in viewport for full details.");
    }
}
