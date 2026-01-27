package com.astraeus.tools;

import com.astraeus.native_api.FrameStatsView;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;

/**
 * Detailed telemetry panel showing frame statistics in table format.
 * 
 * Design goals:
 * - Docking/layout-friendly structure
 * - Table view for metrics
 * - Per-pass breakdown (when available)
 * - No per-frame allocations (updates existing rows)
 */
public class TelemetryPane extends VBox {
    
    private final TableView<MetricRow> metricsTable;
    private final ObservableList<MetricRow> metrics;
    private final CheckBox enabledCheckBox;
    
    // Pre-allocated rows (no allocations per frame)
    private final MetricRow fpsRow;
    private final MetricRow cpuTimeRow;
    private final MetricRow gpuTimeRow;
    private final MetricRow drawCallsRow;
    private final MetricRow trianglesRow;
    private final MetricRow entitiesRow;
    
    private boolean enabled = true;
    
    public TelemetryPane() {
        super(10);
        setPadding(new Insets(10));
        setStyle("-fx-background-color: #f5f5f5; -fx-border-color: #cccccc; -fx-border-width: 1;");
        
        // Title bar with enable/disable checkbox
        HBox titleBar = new HBox(10);
        titleBar.setAlignment(javafx.geometry.Pos.CENTER_LEFT);
        
        Label titleLabel = new Label("Telemetry");
        titleLabel.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
        
        enabledCheckBox = new CheckBox("Enabled");
        enabledCheckBox.setSelected(true);
        enabledCheckBox.setOnAction(e -> setEnabled(enabledCheckBox.isSelected()));
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        titleBar.getChildren().addAll(titleLabel, spacer, enabledCheckBox);
        
        // Create metrics table
        metricsTable = new TableView<>();
        metricsTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        
        // Define columns
        TableColumn<MetricRow, String> nameColumn = new TableColumn<>("Metric");
        nameColumn.setCellValueFactory(data -> data.getValue().nameProperty());
        nameColumn.setPrefWidth(150);
        
        TableColumn<MetricRow, String> valueColumn = new TableColumn<>("Value");
        valueColumn.setCellValueFactory(data -> data.getValue().valueProperty());
        valueColumn.setPrefWidth(120);
        
        TableColumn<MetricRow, String> unitColumn = new TableColumn<>("Unit");
        unitColumn.setCellValueFactory(data -> data.getValue().unitProperty());
        unitColumn.setPrefWidth(80);
        
        metricsTable.getColumns().addAll(nameColumn, valueColumn, unitColumn);
        
        // Pre-allocate metric rows (no allocations per frame)
        fpsRow = new MetricRow("FPS", "--", "fps");
        cpuTimeRow = new MetricRow("CPU Time", "--", "ms");
        gpuTimeRow = new MetricRow("GPU Time", "--", "ms");
        drawCallsRow = new MetricRow("Draw Calls", "--", "");
        trianglesRow = new MetricRow("Triangles", "--", "");
        entitiesRow = new MetricRow("Entities", "--", "");
        
        metrics = FXCollections.observableArrayList(
            fpsRow,
            cpuTimeRow,
            gpuTimeRow,
            drawCallsRow,
            trianglesRow,
            entitiesRow
        );
        
        metricsTable.setItems(metrics);
        
        // Info label
        Label infoLabel = new Label("Updates at controlled rate (~30 Hz)");
        infoLabel.setStyle("-fx-font-size: 10; -fx-text-fill: #666666;");
        
        // Add all components
        getChildren().addAll(titleBar, metricsTable, infoLabel);
        
        // Set sizing
        setMinWidth(350);
        setPrefWidth(350);
        VBox.setVgrow(metricsTable, Priority.ALWAYS);
    }
    
    /**
     * Update telemetry panel with new frame statistics.
     * This method updates existing rows in-place, causing no allocations.
     * 
     * @param stats Current frame statistics
     */
    public void update(FrameStatsView stats) {
        if (!enabled) {
            return;
        }
        
        // Update rows in-place (no allocations)
        fpsRow.setValue(String.format("%.1f", stats.getFPS()));
        cpuTimeRow.setValue(String.format("%.2f", stats.getDeltaTimeMs()));
        gpuTimeRow.setValue(String.format("%.2f", stats.getRenderTimeMs()));
        drawCallsRow.setValue(String.format("%,d", stats.getDrawCalls()));
        trianglesRow.setValue(String.format("%,d", stats.getTriangleCount()));
        entitiesRow.setValue(String.format("%,d", stats.getEntityCount()));
    }
    
    /**
     * Enable or disable telemetry updates.
     */
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
        enabledCheckBox.setSelected(enabled);
    }
    
    /**
     * Check if telemetry is enabled.
     */
    public boolean isEnabled() {
        return enabled;
    }
    
    /**
     * Row data for metrics table.
     * Uses JavaFX properties for automatic UI updates.
     */
    public static class MetricRow {
        private final javafx.beans.property.SimpleStringProperty name;
        private final javafx.beans.property.SimpleStringProperty value;
        private final javafx.beans.property.SimpleStringProperty unit;
        
        public MetricRow(String name, String value, String unit) {
            this.name = new javafx.beans.property.SimpleStringProperty(name);
            this.value = new javafx.beans.property.SimpleStringProperty(value);
            this.unit = new javafx.beans.property.SimpleStringProperty(unit);
        }
        
        public javafx.beans.property.SimpleStringProperty nameProperty() { return name; }
        public javafx.beans.property.SimpleStringProperty valueProperty() { return value; }
        public javafx.beans.property.SimpleStringProperty unitProperty() { return unit; }
        
        public String getName() { return name.get(); }
        public String getValue() { return value.get(); }
        public String getUnit() { return unit.get(); }
        
        public void setName(String name) { this.name.set(name); }
        public void setValue(String value) { this.value.set(value); }
        public void setUnit(String unit) { this.unit.set(unit); }
    }
}
