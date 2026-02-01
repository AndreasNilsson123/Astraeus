package com.astraeus.tools.inspector;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.PickResult;
import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.VBox;

/**
 * Inspector pane for displaying picking results.
 * Shows entity ID, depth, world position, and viewport context from pick operations.
 * 
 * <p>Features:</p>
 * <ul>
 *   <li>Real-time display of pick results</li>
 *   <li>Shows entity ID with validation</li>
 *   <li>Displays depth in normalized coordinates</li>
 *   <li>Shows world position (X, Y, Z)</li>
 *   <li>Viewport context (screen coordinates)</li>
 * </ul>
 * 
 * <p>Usage:</p>
 * <pre>{@code
 * PickInspectorPane pickPane = new PickInspectorPane();
 * 
 * // When user clicks viewport:
 * PickResult result = engine.pick(x, y);
 * pickPane.updatePickResult(result, x, y);
 * }</pre>
 */
public class PickInspectorPane extends VBox {
    
    private final Label statusLabel;
    private final TextField entityIdField;
    private final TextField depthField;
    private final TextField worldXField;
    private final TextField worldYField;
    private final TextField worldZField;
    private final TextField screenXField;
    private final TextField screenYField;
    
    public PickInspectorPane() {
        super(10);
        setPadding(new Insets(10));
        setMinWidth(250);
        setPrefWidth(300);
        setStyle("-fx-background-color: #f5f5f5;");
        
        // Title
        Label titleLabel = new Label("Pick Inspector");
        titleLabel.setStyle("-fx-font-size: 14; -fx-font-weight: bold;");
        
        // Status
        statusLabel = new Label("No pick result");
        statusLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #666666;");
        
        // Info section
        VBox infoSection = createSection("Pick Information");
        
        GridPane grid = new GridPane();
        grid.setHgap(10);
        grid.setVgap(8);
        grid.setPadding(new Insets(5));
        
        // Entity ID
        Label entityLabel = new Label("Entity ID:");
        entityLabel.setMinWidth(80);
        entityIdField = createReadOnlyField();
        grid.add(entityLabel, 0, 0);
        grid.add(entityIdField, 1, 0);
        
        // Depth
        Label depthLabel = new Label("Depth:");
        depthLabel.setMinWidth(80);
        depthField = createReadOnlyField();
        grid.add(depthLabel, 0, 1);
        grid.add(depthField, 1, 1);
        
        infoSection.getChildren().add(grid);
        
        // World Position section
        VBox worldSection = createSection("World Position");
        
        GridPane worldGrid = new GridPane();
        worldGrid.setHgap(10);
        worldGrid.setVgap(8);
        worldGrid.setPadding(new Insets(5));
        
        Label xLabel = new Label("X:");
        xLabel.setMinWidth(80);
        worldXField = createReadOnlyField();
        worldGrid.add(xLabel, 0, 0);
        worldGrid.add(worldXField, 1, 0);
        
        Label yLabel = new Label("Y:");
        yLabel.setMinWidth(80);
        worldYField = createReadOnlyField();
        worldGrid.add(yLabel, 0, 1);
        worldGrid.add(worldYField, 1, 1);
        
        Label zLabel = new Label("Z:");
        zLabel.setMinWidth(80);
        worldZField = createReadOnlyField();
        worldGrid.add(zLabel, 0, 2);
        worldGrid.add(worldZField, 1, 2);
        
        worldSection.getChildren().add(worldGrid);
        
        // Viewport Context section
        VBox viewportSection = createSection("Viewport Context");
        
        GridPane viewportGrid = new GridPane();
        viewportGrid.setHgap(10);
        viewportGrid.setVgap(8);
        viewportGrid.setPadding(new Insets(5));
        
        Label screenXLabel = new Label("Screen X:");
        screenXLabel.setMinWidth(80);
        screenXField = createReadOnlyField();
        viewportGrid.add(screenXLabel, 0, 0);
        viewportGrid.add(screenXField, 1, 0);
        
        Label screenYLabel = new Label("Screen Y:");
        screenYLabel.setMinWidth(80);
        screenYField = createReadOnlyField();
        viewportGrid.add(screenYLabel, 0, 1);
        viewportGrid.add(screenYField, 1, 1);
        
        viewportSection.getChildren().add(viewportGrid);
        
        // Add all sections
        getChildren().addAll(
            titleLabel,
            statusLabel,
            infoSection,
            worldSection,
            viewportSection
        );
    }
    
    /**
     * Create a section container.
     */
    private VBox createSection(String title) {
        VBox section = new VBox(5);
        section.setPadding(new Insets(10));
        section.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        Label titleLabel = new Label(title);
        titleLabel.setStyle("-fx-font-weight: bold; -fx-font-size: 11;");
        section.getChildren().add(titleLabel);
        
        return section;
    }
    
    /**
     * Create a read-only text field for displaying values.
     */
    private TextField createReadOnlyField() {
        TextField field = new TextField();
        field.setEditable(false);
        field.setStyle("-fx-background-color: #f9f9f9; -fx-font-family: monospace;");
        field.setPrefWidth(150);
        return field;
    }
    
    /**
     * Update the inspector with a new pick result.
     * 
     * @param result Pick result from engine (null-safe)
     * @param screenX Screen X coordinate where pick occurred
     * @param screenY Screen Y coordinate where pick occurred
     */
    public void updatePickResult(PickResult result, int screenX, int screenY) {
        if (result == null || !result.isHit()) {
            clearPickResult();
            statusLabel.setText("No hit");
            statusLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #999999;");
            return;
        }
        
        // Update status
        if (result.hasValidEntity()) {
            statusLabel.setText("Entity Selected");
            statusLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #00AA00;");
        } else {
            statusLabel.setText("Hit (no entity)");
            statusLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #FFA500;");
        }
        
        // Update fields
        entityIdField.setText(String.valueOf(result.getEntityId()));
        depthField.setText(String.format("%.4f", result.getDepth()));
        
        worldXField.setText(String.format("%.3f", result.getWorldX()));
        worldYField.setText(String.format("%.3f", result.getWorldY()));
        worldZField.setText(String.format("%.3f", result.getWorldZ()));
        
        screenXField.setText(String.valueOf(screenX));
        screenYField.setText(String.valueOf(screenY));
    }
    
    /**
     * Clear the inspector (no pick result).
     */
    public void clearPickResult() {
        statusLabel.setText("No pick result");
        statusLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #666666;");
        
        entityIdField.clear();
        depthField.clear();
        worldXField.clear();
        worldYField.clear();
        worldZField.clear();
        screenXField.clear();
        screenYField.clear();
    }
}
