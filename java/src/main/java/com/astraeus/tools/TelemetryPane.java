package com.astraeus.tools;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.NativeEngine.TelemetryFrameStats;
import com.astraeus.native_api.NativeEngine.PassTiming;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;

import java.util.ArrayList;
import java.util.List;

/**
 * Detailed telemetry panel for docking/tooling window.
 * Displays comprehensive performance metrics and per-pass breakdown.
 * 
 * PERFORMANCE:
 * - NO allocations per frame (reuses TableView items)
 * - Efficient updates using ObservableList operations
 * - Throttled updates recommended (~30 Hz)
 * 
 * USAGE:
 * <pre>
 * TelemetryPane pane = new TelemetryPane(engine);
 * 
 * // In update loop (throttled to ~30 Hz):
 * pane.update();
 * </pre>
 */
public class TelemetryPane extends BorderPane {
    
    private final NativeEngine engine;
    
    // Top section: Overall stats
    private final Label frameLabel;
    private final Label fpsLabel;
    private final Label cpuTimeLabel;
    private final Label gpuTimeLabel;
    private final Label totalTimeLabel;
    private final Label drawCallsLabel;
    private final Label trianglesLabel;
    private final Label passCountLabel;
    
    // Bottom section: Per-pass breakdown
    private final TableView<PassTimingRow> passTable;
    private final ObservableList<PassTimingRow> passData;
    
    // Control: Enable/disable telemetry
    private final CheckBox enableTelemetryCheckBox;
    
    // Cached data to avoid unnecessary allocations
    private final List<PassTiming> cachedPassTimings = new ArrayList<>();
    
    public TelemetryPane(NativeEngine engine) {
        this.engine = engine;
        
        // === TOP SECTION: Overall Stats ===
        VBox topSection = new VBox(10);
        topSection.setPadding(new Insets(10));
        topSection.setStyle("-fx-background-color: #f5f5f5;");
        
        // Title
        Label titleLabel = new Label("Performance Telemetry");
        titleLabel.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
        
        // Enable/Disable toggle
        enableTelemetryCheckBox = new CheckBox("Enable Telemetry");
        enableTelemetryCheckBox.setSelected(engine.isTelemetryEnabled());
        enableTelemetryCheckBox.setOnAction(e -> {
            engine.enableTelemetry(enableTelemetryCheckBox.isSelected());
        });
        
        // Stats grid
        GridPane statsGrid = new GridPane();
        statsGrid.setHgap(15);
        statsGrid.setVgap(5);
        statsGrid.setPadding(new Insets(10));
        statsGrid.setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        // Create labels
        frameLabel = new Label("0");
        fpsLabel = new Label("0.0");
        cpuTimeLabel = new Label("0.00 ms");
        gpuTimeLabel = new Label("N/A");
        totalTimeLabel = new Label("0.00 ms");
        drawCallsLabel = new Label("0");
        trianglesLabel = new Label("0");
        passCountLabel = new Label("0");
        
        // Style value labels
        String valueStyle = "-fx-font-weight: bold;";
        frameLabel.setStyle(valueStyle);
        fpsLabel.setStyle(valueStyle);
        cpuTimeLabel.setStyle(valueStyle);
        gpuTimeLabel.setStyle(valueStyle);
        totalTimeLabel.setStyle(valueStyle);
        drawCallsLabel.setStyle(valueStyle);
        trianglesLabel.setStyle(valueStyle);
        passCountLabel.setStyle(valueStyle);
        
        // Add to grid
        int row = 0;
        statsGrid.add(new Label("Frame:"), 0, row);
        statsGrid.add(frameLabel, 1, row++);
        
        statsGrid.add(new Label("FPS:"), 0, row);
        statsGrid.add(fpsLabel, 1, row++);
        
        statsGrid.add(new Label("CPU Time:"), 0, row);
        statsGrid.add(cpuTimeLabel, 1, row++);
        
        statsGrid.add(new Label("GPU Time:"), 0, row);
        statsGrid.add(gpuTimeLabel, 1, row++);
        
        statsGrid.add(new Label("Total Time:"), 0, row);
        statsGrid.add(totalTimeLabel, 1, row++);
        
        statsGrid.add(new Label("Draw Calls:"), 0, row);
        statsGrid.add(drawCallsLabel, 1, row++);
        
        statsGrid.add(new Label("Triangles:"), 0, row);
        statsGrid.add(trianglesLabel, 1, row++);
        
        statsGrid.add(new Label("Passes:"), 0, row);
        statsGrid.add(passCountLabel, 1, row++);
        
        topSection.getChildren().addAll(titleLabel, enableTelemetryCheckBox, statsGrid);
        
        // === BOTTOM SECTION: Per-Pass Breakdown ===
        VBox bottomSection = new VBox(5);
        bottomSection.setPadding(new Insets(10));
        
        Label passLabel = new Label("Render Pass Breakdown");
        passLabel.setStyle("-fx-font-weight: bold;");
        
        // Table for pass timings
        passData = FXCollections.observableArrayList();
        passTable = new TableView<>(passData);
        passTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY);
        passTable.setPrefHeight(200);
        
        // Columns
        TableColumn<PassTimingRow, String> nameCol = new TableColumn<>("Pass Name");
        nameCol.setCellValueFactory(cellData -> cellData.getValue().nameProperty());
        nameCol.setPrefWidth(200);
        
        TableColumn<PassTimingRow, String> timeCol = new TableColumn<>("Time (ms)");
        timeCol.setCellValueFactory(cellData -> cellData.getValue().timeMsProperty());
        timeCol.setPrefWidth(80);
        
        TableColumn<PassTimingRow, String> percentCol = new TableColumn<>("Percentage");
        percentCol.setCellValueFactory(cellData -> cellData.getValue().percentageProperty());
        percentCol.setPrefWidth(80);
        
        passTable.getColumns().addAll(nameCol, timeCol, percentCol);
        
        bottomSection.getChildren().addAll(passLabel, passTable);
        
        // === LAYOUT ===
        setTop(topSection);
        setCenter(bottomSection);
        
        // Set minimum size
        setMinWidth(300);
        setPrefWidth(350);
    }
    
    /**
     * Update telemetry display with latest data from engine.
     * Call this method at a throttled rate (~30 Hz) to avoid UI overhead.
     * 
     * PERFORMANCE: Reuses existing TableView rows when possible.
     */
    public void update() {
        if (!engine.isTelemetryEnabled()) {
            return;
        }
        
        try {
            // Get current frame stats
            TelemetryFrameStats stats = engine.getTelemetryStats();
            updateOverallStats(stats);
            
            // Get per-pass timings
            int passCount = engine.getPassCount();
            cachedPassTimings.clear();
            
            double totalPassTime = 0.0;
            for (int i = 0; i < passCount; i++) {
                PassTiming timing = engine.getPassTiming(i);
                if (timing != null) {
                    cachedPassTimings.add(timing);
                    totalPassTime += timing.getTimeMs();
                }
            }
            
            updatePassBreakdown(cachedPassTimings, totalPassTime);
            
        } catch (Exception e) {
            System.err.println("[TelemetryPane] Error updating: " + e.getMessage());
        }
    }
    
    /**
     * Update overall statistics labels.
     */
    private void updateOverallStats(TelemetryFrameStats stats) {
        frameLabel.setText(String.valueOf(stats.getFrameNumber()));
        fpsLabel.setText(String.format("%.1f", stats.getFPS()));
        cpuTimeLabel.setText(String.format("%.2f ms", stats.getCpuTimeMs()));
        
        // GPU time: show "N/A" if not available
        if (stats.getGpuTimeMs() > 0.001) {
            gpuTimeLabel.setText(String.format("%.2f ms", stats.getGpuTimeMs()));
        } else {
            gpuTimeLabel.setText("N/A");
        }
        
        totalTimeLabel.setText(String.format("%.2f ms", stats.getTotalTimeMs()));
        drawCallsLabel.setText(String.format("%,d", stats.getDrawCalls()));
        trianglesLabel.setText(String.format("%,d", stats.getTriangleCount()));
        passCountLabel.setText(String.valueOf(stats.getPassCount()));
    }
    
    /**
     * Update per-pass breakdown table.
     * Efficiently reuses existing rows when possible.
     */
    private void updatePassBreakdown(List<PassTiming> timings, double totalTime) {
        // Sort by time (descending)
        timings.sort((a, b) -> Double.compare(b.getTimeMs(), a.getTimeMs()));
        
        // Update or create rows
        int rowCount = Math.max(passData.size(), timings.size());
        
        for (int i = 0; i < rowCount; i++) {
            if (i < timings.size()) {
                PassTiming timing = timings.get(i);
                double percentage = (totalTime > 0) ? (timing.getTimeMs() / totalTime * 100.0) : 0.0;
                
                if (i < passData.size()) {
                    // Reuse existing row
                    PassTimingRow row = passData.get(i);
                    row.update(timing.getName(), timing.getTimeMs(), percentage);
                } else {
                    // Add new row
                    passData.add(new PassTimingRow(timing.getName(), timing.getTimeMs(), percentage));
                }
            } else {
                // Remove excess rows
                passData.remove(i);
                rowCount--; // Adjust count after removal
                i--; // Re-check this index
            }
        }
    }
    
    /**
     * TableView row for pass timing data.
     * Uses JavaFX properties for efficient updates.
     */
    public static class PassTimingRow {
        private final SimpleStringProperty name;
        private final SimpleStringProperty timeMs;
        private final SimpleStringProperty percentage;
        
        public PassTimingRow(String name, double timeMs, double percentage) {
            this.name = new SimpleStringProperty(name);
            this.timeMs = new SimpleStringProperty(String.format("%.3f", timeMs));
            this.percentage = new SimpleStringProperty(String.format("%.1f%%", percentage));
        }
        
        public void update(String name, double timeMs, double percentage) {
            this.name.set(name);
            this.timeMs.set(String.format("%.3f", timeMs));
            this.percentage.set(String.format("%.1f%%", percentage));
        }
        
        public SimpleStringProperty nameProperty() { return name; }
        public SimpleStringProperty timeMsProperty() { return timeMs; }
        public SimpleStringProperty percentageProperty() { return percentage; }
    }
}
