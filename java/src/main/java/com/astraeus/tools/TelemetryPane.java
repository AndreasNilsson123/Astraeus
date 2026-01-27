package com.astraeus.tools;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.FrameStatsView;
import com.astraeus.native_api.PassTelemetryView;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.layout.*;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;

/**
 * Detailed telemetry panel showing per-pass performance breakdown.
 * Displays a table of all render passes with timing information.
 * 
 * Features:
 * - TableView with pass-by-pass breakdown
 * - Pass name, duration (ms), and percentage columns
 * - Auto-refresh when telemetry is enabled
 * - Summary row showing total frame time
 * - Enable/disable telemetry button
 * 
 * Usage:
 * <pre>
 *   TelemetryPane pane = new TelemetryPane(engine);
 *   
 *   // Each frame (or at regular intervals):
 *   pane.update();
 * </pre>
 */
public class TelemetryPane extends VBox {
    
    private final NativeEngine engine;
    private final TableView<PassTelemetryRow> table;
    private final ObservableList<PassTelemetryRow> tableData;
    
    private final Label totalLabel;
    private final Label passCountLabel;
    private final CheckBox enabledCheckBox;
    
    // Reusable views (avoid per-frame allocations)
    private final FrameStatsView frameStats;
    private final PassTelemetryView passTelemetry;
    
    /**
     * Create a new telemetry pane.
     * 
     * @param engine Native engine instance
     */
    public TelemetryPane(NativeEngine engine) {
        super(10);
        this.engine = engine;
        this.frameStats = new FrameStatsView();
        this.passTelemetry = new PassTelemetryView();
        this.tableData = FXCollections.observableArrayList();
        
        setPadding(new Insets(10));
        setStyle("-fx-background-color: #f5f5f5; -fx-border-color: #cccccc; -fx-border-width: 1;");
        
        // Title and controls
        HBox headerBox = new HBox(10);
        headerBox.setAlignment(javafx.geometry.Pos.CENTER_LEFT);
        
        Label titleLabel = new Label("Performance Telemetry");
        titleLabel.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        enabledCheckBox = new CheckBox("Enable Telemetry");
        enabledCheckBox.setSelected(engine.isTelemetryEnabled());
        enabledCheckBox.setOnAction(e -> {
            engine.setTelemetryEnabled(enabledCheckBox.isSelected());
            if (enabledCheckBox.isSelected()) {
                update();  // Refresh immediately
            } else {
                clearTable();
            }
        });
        
        headerBox.getChildren().addAll(titleLabel, spacer, enabledCheckBox);
        
        // Summary info
        HBox summaryBox = new HBox(20);
        summaryBox.setPadding(new Insets(5));
        summaryBox.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        passCountLabel = new Label("Passes: 0");
        passCountLabel.setFont(Font.font("System", FontWeight.BOLD, 12));
        
        totalLabel = new Label("Total: 0.00ms");
        totalLabel.setFont(Font.font("System", FontWeight.BOLD, 12));
        
        summaryBox.getChildren().addAll(passCountLabel, totalLabel);
        
        // Table
        table = createTable();
        VBox.setVgrow(table, Priority.ALWAYS);
        
        // Instructions
        Label instructionsLabel = new Label(
            "Shows timing breakdown for each render pass. " +
            "Enable telemetry to collect data (adds minor overhead)."
        );
        instructionsLabel.setStyle("-fx-font-size: 10; -fx-text-fill: #666666;");
        instructionsLabel.setWrapText(true);
        
        // Add all components
        getChildren().addAll(headerBox, summaryBox, table, instructionsLabel);
        
        // Set minimum size
        setMinWidth(400);
        setPrefWidth(500);
        setMinHeight(300);
    }
    
    /**
     * Create and configure the telemetry table.
     */
    @SuppressWarnings("unchecked")
    private TableView<PassTelemetryRow> createTable() {
        TableView<PassTelemetryRow> tableView = new TableView<>();
        tableView.setItems(tableData);
        tableView.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        
        // Pass Name column
        TableColumn<PassTelemetryRow, String> nameColumn = new TableColumn<>("Pass Name");
        nameColumn.setCellValueFactory(new PropertyValueFactory<>("passName"));
        nameColumn.setMinWidth(150);
        nameColumn.setPrefWidth(200);
        
        // Duration column
        TableColumn<PassTelemetryRow, Double> durationColumn = new TableColumn<>("Duration (ms)");
        durationColumn.setCellValueFactory(new PropertyValueFactory<>("durationMs"));
        durationColumn.setMinWidth(100);
        durationColumn.setPrefWidth(120);
        durationColumn.setStyle("-fx-alignment: CENTER-RIGHT;");
        
        // Format duration with 3 decimal places
        durationColumn.setCellFactory(col -> new TableCell<PassTelemetryRow, Double>() {
            @Override
            protected void updateItem(Double item, boolean empty) {
                super.updateItem(item, empty);
                if (empty || item == null) {
                    setText(null);
                } else {
                    setText(String.format("%.3f", item));
                }
            }
        });
        
        // Percentage column
        TableColumn<PassTelemetryRow, Double> percentColumn = new TableColumn<>("Percentage (%)");
        percentColumn.setCellValueFactory(new PropertyValueFactory<>("percentage"));
        percentColumn.setMinWidth(100);
        percentColumn.setPrefWidth(120);
        percentColumn.setStyle("-fx-alignment: CENTER-RIGHT;");
        
        // Format percentage with 1 decimal place
        percentColumn.setCellFactory(col -> new TableCell<PassTelemetryRow, Double>() {
            @Override
            protected void updateItem(Double item, boolean empty) {
                super.updateItem(item, empty);
                if (empty || item == null) {
                    setText(null);
                } else {
                    setText(String.format("%.1f%%", item));
                }
            }
        });
        
        tableView.getColumns().addAll(nameColumn, durationColumn, percentColumn);
        
        return tableView;
    }
    
    /**
     * Update telemetry data from engine.
     * Call this each frame or at a controlled rate (e.g., 10-30 Hz).
     */
    public void update() {
        if (!engine.isTelemetryEnabled()) {
            return;  // No data available
        }
        
        try {
            // Get frame stats for total time calculation
            engine.getFrameStats(frameStats);
            double totalFrameTime = frameStats.getRenderTimeMs();
            
            // Get pass count
            int passCount = engine.getPassCount();
            
            // Clear existing data
            tableData.clear();
            
            // Query each pass
            for (int i = 0; i < passCount; i++) {
                if (engine.getPassTelemetry(i, passTelemetry)) {
                    String passName = passTelemetry.getPassName();
                    double durationMs = passTelemetry.getDurationMs();
                    double percentage = passTelemetry.getPercentage(totalFrameTime);
                    
                    tableData.add(new PassTelemetryRow(passName, durationMs, percentage));
                }
            }
            
            // Update summary
            passCountLabel.setText(String.format("Passes: %d", passCount));
            totalLabel.setText(String.format("Total: %.2fms (%.1f FPS)", 
                                            totalFrameTime, 
                                            frameStats.getFPS()));
            
        } catch (Exception e) {
            System.err.println("[TelemetryPane] Error updating telemetry: " + e.getMessage());
        }
    }
    
    /**
     * Clear the table and reset summary.
     */
    private void clearTable() {
        tableData.clear();
        passCountLabel.setText("Passes: 0");
        totalLabel.setText("Total: 0.00ms");
    }
    
    /**
     * Enable or disable automatic telemetry collection.
     */
    public void setTelemetryEnabled(boolean enabled) {
        enabledCheckBox.setSelected(enabled);
        engine.setTelemetryEnabled(enabled);
        if (!enabled) {
            clearTable();
        }
    }
    
    /**
     * Check if telemetry is currently enabled.
     */
    public boolean isTelemetryEnabled() {
        return engine.isTelemetryEnabled();
    }
    
    /**
     * Data model for table rows.
     * Uses JavaFX properties for binding.
     */
    public static class PassTelemetryRow {
        private final SimpleStringProperty passName;
        private final SimpleDoubleProperty durationMs;
        private final SimpleDoubleProperty percentage;
        
        public PassTelemetryRow(String passName, double durationMs, double percentage) {
            this.passName = new SimpleStringProperty(passName);
            this.durationMs = new SimpleDoubleProperty(durationMs);
            this.percentage = new SimpleDoubleProperty(percentage);
        }
        
        public String getPassName() {
            return passName.get();
        }
        
        public SimpleStringProperty passNameProperty() {
            return passName;
        }
        
        public double getDurationMs() {
            return durationMs.get();
        }
        
        public SimpleDoubleProperty durationMsProperty() {
            return durationMs;
        }
        
        public double getPercentage() {
            return percentage.get();
        }
        
        public SimpleDoubleProperty percentageProperty() {
            return percentage;
        }
    }
}
