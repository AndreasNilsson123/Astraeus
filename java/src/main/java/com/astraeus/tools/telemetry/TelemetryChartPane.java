package com.astraeus.tools.telemetry;

import com.astraeus.native_api.model.FrameStats;
import javafx.geometry.Insets;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Label;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

import java.util.List;

/**
 * Chart pane for visualizing telemetry history.
 * Displays line charts for FPS, frame time, draw calls, and triangles.
 * 
 * <p>Features:</p>
 * <ul>
 *   <li>Automatic scaling based on data range</li>
 *   <li>Grid lines for readability</li>
 *   <li>Multiple metrics in separate charts</li>
 *   <li>Efficient rendering using Canvas</li>
 * </ul>
 * 
 * <p>Performance: Redraws only when data changes. Update at 10-30 Hz.</p>
 * 
 * <p>Usage:</p>
 * <pre>{@code
 * TelemetryChartPane chartPane = new TelemetryChartPane();
 * 
 * // In update loop:
 * chartPane.updateChart(statsHistory);
 * }</pre>
 */
public class TelemetryChartPane extends BorderPane {
    
    private final Canvas fpsCanvas;
    private final Canvas frameTimeCanvas;
    private final Label fpsLabel;
    private final Label frameTimeLabel;
    
    private static final int CHART_HEIGHT = 100;
    private static final int CHART_PADDING = 30;
    private static final Color FPS_COLOR = Color.rgb(0, 150, 255);
    private static final Color FRAME_TIME_COLOR = Color.rgb(255, 100, 50);
    private static final Color GRID_COLOR = Color.rgb(200, 200, 200, 0.5);
    
    public TelemetryChartPane() {
        setPadding(new Insets(10));
        setStyle("-fx-background-color: white; -fx-border-color: #dddddd; -fx-border-width: 1;");
        
        VBox content = new VBox(10);
        
        // Title
        Label titleLabel = new Label("Performance History");
        titleLabel.setStyle("-fx-font-weight: bold; -fx-font-size: 12;");
        
        // FPS Chart
        fpsLabel = new Label("FPS (frames per second)");
        fpsLabel.setStyle("-fx-font-size: 10;");
        fpsCanvas = new Canvas();
        fpsCanvas.setHeight(CHART_HEIGHT);
        
        // Frame Time Chart
        frameTimeLabel = new Label("Frame Time (milliseconds)");
        frameTimeLabel.setStyle("-fx-font-size: 10;");
        frameTimeCanvas = new Canvas();
        frameTimeCanvas.setHeight(CHART_HEIGHT);
        
        content.getChildren().addAll(
            titleLabel,
            fpsLabel,
            fpsCanvas,
            frameTimeLabel,
            frameTimeCanvas
        );
        
        setCenter(content);
        
        // Handle resize
        widthProperty().addListener((obs, oldVal, newVal) -> {
            double width = newVal.doubleValue() - 20; // Account for padding
            fpsCanvas.setWidth(Math.max(200, width));
            frameTimeCanvas.setWidth(Math.max(200, width));
        });
        
        // Initial size
        fpsCanvas.setWidth(400);
        frameTimeCanvas.setWidth(400);
    }
    
    /**
     * Update charts with new telemetry history.
     * 
     * @param history Frame stats history (null-safe)
     */
    public void updateChart(FrameStatsHistory history) {
        if (history == null || history.isEmpty()) {
            clearCharts();
            return;
        }
        
        List<FrameStats> stats = history.getAll();
        if (stats.isEmpty()) {
            clearCharts();
            return;
        }
        
        drawFpsChart(stats);
        drawFrameTimeChart(stats);
    }
    
    /**
     * Clear all charts.
     */
    private void clearCharts() {
        GraphicsContext gc = fpsCanvas.getGraphicsContext2D();
        gc.clearRect(0, 0, fpsCanvas.getWidth(), fpsCanvas.getHeight());
        
        gc = frameTimeCanvas.getGraphicsContext2D();
        gc.clearRect(0, 0, frameTimeCanvas.getWidth(), frameTimeCanvas.getHeight());
    }
    
    /**
     * Draw FPS chart.
     */
    private void drawFpsChart(List<FrameStats> stats) {
        GraphicsContext gc = fpsCanvas.getGraphicsContext2D();
        double width = fpsCanvas.getWidth();
        double height = fpsCanvas.getHeight();
        
        // Clear
        gc.clearRect(0, 0, width, height);
        
        // Find min/max FPS
        double minFps = Double.MAX_VALUE;
        double maxFps = 0;
        for (FrameStats stat : stats) {
            double fps = stat.getFPS();
            if (fps < minFps) minFps = fps;
            if (fps > maxFps) maxFps = fps;
        }
        
        // Add some padding to range
        double range = maxFps - minFps;
        if (range < 1) range = 1; // Avoid division by zero
        minFps = Math.max(0, minFps - range * 0.1);
        maxFps = maxFps + range * 0.1;
        
        // Draw grid and axes
        drawGrid(gc, width, height, minFps, maxFps);
        
        // Draw line
        gc.setStroke(FPS_COLOR);
        gc.setLineWidth(2);
        
        double xStep = (width - CHART_PADDING) / Math.max(1, stats.size() - 1);
        
        gc.beginPath();
        for (int i = 0; i < stats.size(); i++) {
            double fps = stats.get(i).getFPS();
            double x = i * xStep;
            double y = height - ((fps - minFps) / (maxFps - minFps) * (height - 10) + 5);
            
            if (i == 0) {
                gc.moveTo(x, y);
            } else {
                gc.lineTo(x, y);
            }
        }
        gc.stroke();
        
        // Draw labels
        gc.setFill(Color.BLACK);
        gc.setFont(javafx.scene.text.Font.font(9));
        gc.fillText(String.format("%.0f", maxFps), width - 25, 10);
        gc.fillText(String.format("%.0f", minFps), width - 25, height - 5);
    }
    
    /**
     * Draw frame time chart.
     */
    private void drawFrameTimeChart(List<FrameStats> stats) {
        GraphicsContext gc = frameTimeCanvas.getGraphicsContext2D();
        double width = frameTimeCanvas.getWidth();
        double height = frameTimeCanvas.getHeight();
        
        // Clear
        gc.clearRect(0, 0, width, height);
        
        // Find min/max frame time
        double minTime = Double.MAX_VALUE;
        double maxTime = 0;
        for (FrameStats stat : stats) {
            double time = stat.getTotalTimeMs();
            if (time < minTime) minTime = time;
            if (time > maxTime) maxTime = time;
        }
        
        // Add some padding to range
        double range = maxTime - minTime;
        if (range < 1) range = 1;
        minTime = Math.max(0, minTime - range * 0.1);
        maxTime = maxTime + range * 0.1;
        
        // Draw grid and axes
        drawGrid(gc, width, height, minTime, maxTime);
        
        // Draw line
        gc.setStroke(FRAME_TIME_COLOR);
        gc.setLineWidth(2);
        
        double xStep = (width - CHART_PADDING) / Math.max(1, stats.size() - 1);
        
        gc.beginPath();
        for (int i = 0; i < stats.size(); i++) {
            double time = stats.get(i).getTotalTimeMs();
            double x = i * xStep;
            double y = height - ((time - minTime) / (maxTime - minTime) * (height - 10) + 5);
            
            if (i == 0) {
                gc.moveTo(x, y);
            } else {
                gc.lineTo(x, y);
            }
        }
        gc.stroke();
        
        // Draw labels
        gc.setFill(Color.BLACK);
        gc.setFont(javafx.scene.text.Font.font(9));
        gc.fillText(String.format("%.1f ms", maxTime), width - 40, 10);
        gc.fillText(String.format("%.1f ms", minTime), width - 40, height - 5);
    }
    
    /**
     * Draw grid lines and background.
     */
    private void drawGrid(GraphicsContext gc, double width, double height, double minVal, double maxVal) {
        // Background
        gc.setFill(Color.rgb(250, 250, 250));
        gc.fillRect(0, 0, width, height);
        
        // Grid lines
        gc.setStroke(GRID_COLOR);
        gc.setLineWidth(1);
        
        // Horizontal grid lines
        int gridLines = 4;
        for (int i = 0; i <= gridLines; i++) {
            double y = i * (height / gridLines);
            gc.strokeLine(0, y, width - CHART_PADDING, y);
        }
        
        // Vertical grid lines (every 10th frame)
        int verticalLines = 5;
        for (int i = 0; i <= verticalLines; i++) {
            double x = i * ((width - CHART_PADDING) / verticalLines);
            gc.strokeLine(x, 0, x, height);
        }
    }
}
