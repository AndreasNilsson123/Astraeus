package com.astraeus.tools;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.util.MathUtils;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;

import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

/**
 * Timeline panel for displaying render time, simulation time, and playback controls.
 * 
 * Features:
 * - Frame number and delta time display
 * - Simulation time tracking (if available)
 * - Playback controls (play/pause/step/reset)
 * - Frame rate display (smoothed FPS)
 * - Time scale controls for slow-motion/fast-forward
 * - Visual timeline representation
 * 
 * PERFORMANCE:
 * - Updates at controlled rate (configurable, default 30 Hz)
 * - No per-frame allocations
 * - Efficient FPS smoothing with rolling average
 * 
 * USAGE:
 * <pre>
 * TimelinePane timeline = new TimelinePane(engine);
 * 
 * // Start auto-update
 * timeline.startAutoUpdate();
 * 
 * // Stop when done
 * timeline.stopAutoUpdate();
 * </pre>
 */
public class TimelinePane extends BorderPane {
    
    private final NativeEngine engine;
    
    // Timeline state
    private long currentFrame = 0;
    private double currentTime = 0.0;  // Accumulated time in seconds
    private double simTime = 0.0;      // Simulation time (separate from render time)
    private double timeScale = 1.0;    // Time scale multiplier
    private boolean isPaused = false;
    
    // FPS tracking (rolling average)
    private static final int FPS_HISTORY_SIZE = 30;
    private final double[] fpsHistory = new double[FPS_HISTORY_SIZE];
    private int fpsHistoryIndex = 0;
    private double smoothedFps = 60.0;
    
    // UI Components - Top section (info)
    private final Label frameLabel;
    private final Label fpsLabel;
    private final Label deltaTimeLabel;
    private final Label renderTimeLabel;
    private final Label simTimeLabel;
    
    // UI Components - Middle section (timeline visual)
    private final Canvas timelineCanvas;
    private final Label timeScaleLabel;
    
    // UI Components - Bottom section (controls)
    private final Button playPauseButton;
    private final Button stepButton;
    private final Button resetButton;
    private final Slider timeScaleSlider;
    
    // Auto-update handling
    private ScheduledFuture<?> updateFuture;
    private static final int UPDATE_RATE_HZ = 30;
    
    public TimelinePane(NativeEngine engine) {
        this.engine = engine;
        
        // === TOP SECTION: Time Info ===
        VBox topSection = new VBox(10);
        topSection.setPadding(new Insets(10));
        topSection.setStyle("-fx-background-color: #f5f5f5;");
        
        Label titleLabel = new Label("Timeline & Playback");
        titleLabel.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
        
        // Info grid
        GridPane infoGrid = new GridPane();
        infoGrid.setHgap(15);
        infoGrid.setVgap(5);
        
        int row = 0;
        
        // Frame number
        infoGrid.add(new Label("Frame:"), 0, row);
        frameLabel = new Label("0");
        frameLabel.setStyle("-fx-font-weight: bold; -fx-font-size: 14;");
        infoGrid.add(frameLabel, 1, row);
        row++;
        
        // FPS
        infoGrid.add(new Label("FPS:"), 0, row);
        fpsLabel = new Label("0.0");
        fpsLabel.setStyle("-fx-font-weight: bold;");
        infoGrid.add(fpsLabel, 1, row);
        row++;
        
        // Delta time
        infoGrid.add(new Label("Delta Time:"), 0, row);
        deltaTimeLabel = new Label("0.000 ms");
        infoGrid.add(deltaTimeLabel, 1, row);
        row++;
        
        // Render time
        infoGrid.add(new Label("Render Time:"), 0, row);
        renderTimeLabel = new Label("0.000 s");
        infoGrid.add(renderTimeLabel, 1, row);
        row++;
        
        // Simulation time
        infoGrid.add(new Label("Sim Time:"), 0, row);
        simTimeLabel = new Label("0.000 s");
        infoGrid.add(simTimeLabel, 1, row);
        
        topSection.getChildren().addAll(titleLabel, new Separator(), infoGrid);
        
        // === MIDDLE SECTION: Timeline Visual ===
        VBox middleSection = new VBox(10);
        middleSection.setPadding(new Insets(10));
        
        Label timelineLabel = new Label("Timeline:");
        timelineLabel.setStyle("-fx-font-weight: bold;");
        
        timelineCanvas = new Canvas();
        timelineCanvas.setPrefHeight(40);
        
        // Time scale display
        HBox timeScaleBox = new HBox(10);
        timeScaleBox.setAlignment(Pos.CENTER_LEFT);
        Label timeScaleTextLabel = new Label("Time Scale:");
        timeScaleLabel = new Label("1.0x");
        timeScaleLabel.setStyle("-fx-font-weight: bold;");
        timeScaleBox.getChildren().addAll(timeScaleTextLabel, timeScaleLabel);
        
        middleSection.getChildren().addAll(timelineLabel, timelineCanvas, timeScaleBox);
        
        // === BOTTOM SECTION: Playback Controls ===
        VBox bottomSection = new VBox(10);
        bottomSection.setPadding(new Insets(10));
        bottomSection.setStyle("-fx-background-color: #f5f5f5;");
        
        // Control buttons
        HBox buttonBox = new HBox(10);
        buttonBox.setAlignment(Pos.CENTER);
        
        playPauseButton = new Button("⏸ Pause");
        playPauseButton.setPrefWidth(100);
        playPauseButton.setOnAction(e -> togglePlayPause());
        
        stepButton = new Button("⏭ Step");
        stepButton.setPrefWidth(100);
        stepButton.setOnAction(e -> stepForward());
        stepButton.setDisable(!isPaused);
        
        resetButton = new Button("⏮ Reset");
        resetButton.setPrefWidth(100);
        resetButton.setOnAction(e -> reset());
        
        buttonBox.getChildren().addAll(playPauseButton, stepButton, resetButton);
        
        // Time scale slider
        HBox sliderBox = new HBox(10);
        sliderBox.setAlignment(Pos.CENTER);
        
        Label sliderLabel = new Label("Speed:");
        
        timeScaleSlider = new Slider(0.1, 4.0, 1.0);
        timeScaleSlider.setShowTickLabels(true);
        timeScaleSlider.setShowTickMarks(true);
        timeScaleSlider.setMajorTickUnit(1.0);
        timeScaleSlider.setMinorTickCount(3);
        timeScaleSlider.setPrefWidth(250);
        timeScaleSlider.valueProperty().addListener((obs, oldVal, newVal) -> {
            setTimeScale(newVal.doubleValue());
        });
        
        sliderBox.getChildren().addAll(sliderLabel, timeScaleSlider);
        
        bottomSection.getChildren().addAll(buttonBox, sliderBox);
        
        // === Layout ===
        setTop(topSection);
        setCenter(middleSection);
        setBottom(bottomSection);
        
        // Set minimum size
        setMinWidth(300);
        setPrefWidth(400);
        
        // Initialize FPS history
        for (int i = 0; i < FPS_HISTORY_SIZE; i++) {
            fpsHistory[i] = 60.0;
        }
    }
    
    /**
     * Start automatic updates at configured rate.
     */
    public void startAutoUpdate() {
        if (updateFuture != null && !updateFuture.isDone()) {
            return; // Already running
        }
        
        updateFuture = com.astraeus.util.ThreadingUtils.scheduleAtFixedRate(
            () -> Platform.runLater(this::update),
            0,
            1000 / UPDATE_RATE_HZ,
            TimeUnit.MILLISECONDS
        );
    }
    
    /**
     * Stop automatic updates.
     */
    public void stopAutoUpdate() {
        if (updateFuture != null) {
            updateFuture.cancel(false);
            updateFuture = null;
        }
    }
    
    /**
     * Update timeline display (call from update loop or auto-update).
     */
    public void update() {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        // Get telemetry data if available
        if (engine.isTelemetryEnabled()) {
            NativeEngine.TelemetryFrameStats stats = engine.getTelemetryStats();
            if (stats != null) {
                currentFrame = stats.getFrameNumber();
                
                // Update FPS (smoothed)
                double instantFps = 1000.0 / Math.max(stats.getDeltaTimeMs(), 0.001);
                updateFps(instantFps);
                
                // Update labels
                frameLabel.setText(String.format("%d", currentFrame));
                fpsLabel.setText(String.format("%.1f", smoothedFps));
                deltaTimeLabel.setText(String.format("%.3f ms", stats.getDeltaTimeMs()));
                renderTimeLabel.setText(String.format("%.3f s", currentTime));
                simTimeLabel.setText(String.format("%.3f s", simTime));
                
                // Update time accumulators
                if (!isPaused) {
                    double deltaSeconds = stats.getDeltaTimeMs() / 1000.0;
                    currentTime += deltaSeconds * timeScale;
                    simTime += deltaSeconds * timeScale;
                }
            }
        } else {
            // Telemetry disabled, show basic info
            frameLabel.setText("N/A (telemetry disabled)");
        }
        
        // Update timeline visual
        drawTimeline();
    }
    
    /**
     * Update FPS with smoothing.
     */
    private void updateFps(double instantFps) {
        // Clamp to reasonable range
        instantFps = MathUtils.clamp(instantFps, 1.0, 1000.0);
        
        // Add to rolling history
        fpsHistory[fpsHistoryIndex] = instantFps;
        fpsHistoryIndex = (fpsHistoryIndex + 1) % FPS_HISTORY_SIZE;
        
        // Calculate average
        double sum = 0.0;
        for (double fps : fpsHistory) {
            sum += fps;
        }
        smoothedFps = sum / FPS_HISTORY_SIZE;
    }
    
    /**
     * Draw timeline visualization.
     */
    private void drawTimeline() {
        double width = timelineCanvas.getWidth();
        double height = timelineCanvas.getHeight();
        
        if (width <= 0 || height <= 0) {
            return;
        }
        
        var gc = timelineCanvas.getGraphicsContext2D();
        
        // Clear canvas
        gc.setFill(Color.WHITE);
        gc.fillRect(0, 0, width, height);
        
        // Draw border
        gc.setStroke(Color.LIGHTGRAY);
        gc.strokeRect(0, 0, width, height);
        
        // Draw frame indicator (simple progress bar style)
        double progress = (currentFrame % 1000) / 1000.0; // Wrap at 1000 frames
        double barWidth = width * progress;
        
        gc.setFill(isPaused ? Color.ORANGE : Color.LIGHTBLUE);
        gc.fillRect(0, 0, barWidth, height);
        
        // Draw frame markers (every 100 frames)
        gc.setStroke(Color.GRAY);
        for (int i = 0; i <= 10; i++) {
            double x = i * (width / 10.0);
            gc.strokeLine(x, 0, x, height);
        }
        
        // Draw current frame text
        gc.setFill(Color.BLACK);
        gc.fillText(String.format("Frame %d", currentFrame), 10, height / 2 + 5);
    }
    
    /**
     * Toggle play/pause state.
     */
    private void togglePlayPause() {
        isPaused = !isPaused;
        
        if (isPaused) {
            playPauseButton.setText("▶ Play");
            stepButton.setDisable(false);
        } else {
            playPauseButton.setText("⏸ Pause");
            stepButton.setDisable(true);
        }
    }
    
    /**
     * Step forward one frame (when paused).
     */
    private void stepForward() {
        if (!isPaused) {
            return;
        }
        
        // Advance by one frame worth of time
        currentFrame++;
        double deltaSeconds = 1.0 / 60.0; // Assume 60 FPS for stepping
        currentTime += deltaSeconds * timeScale;
        simTime += deltaSeconds * timeScale;
        
        update();
    }
    
    /**
     * Reset timeline to zero.
     */
    private void reset() {
        currentFrame = 0;
        currentTime = 0.0;
        simTime = 0.0;
        
        // Reset FPS history
        for (int i = 0; i < FPS_HISTORY_SIZE; i++) {
            fpsHistory[i] = 60.0;
        }
        fpsHistoryIndex = 0;
        smoothedFps = 60.0;
        
        update();
    }
    
    /**
     * Set time scale multiplier.
     */
    public void setTimeScale(double scale) {
        timeScale = MathUtils.clamp(scale, 0.1, 4.0);
        timeScaleLabel.setText(String.format("%.1fx", timeScale));
        timeScaleSlider.setValue(timeScale);
    }
    
    /**
     * Get current time scale.
     */
    public double getTimeScale() {
        return timeScale;
    }
    
    /**
     * Check if playback is paused.
     */
    public boolean isPaused() {
        return isPaused;
    }
    
    /**
     * Set pause state.
     */
    public void setPaused(boolean paused) {
        if (isPaused != paused) {
            togglePlayPause();
        }
    }
    
    /**
     * Get current frame number.
     */
    public long getCurrentFrame() {
        return currentFrame;
    }
    
    /**
     * Get current render time.
     */
    public double getCurrentTime() {
        return currentTime;
    }
    
    /**
     * Get current simulation time.
     */
    public double getSimTime() {
        return simTime;
    }
    
    /**
     * Simple canvas component for timeline visualization.
     */
    private static class Canvas extends Pane {
        private final javafx.scene.canvas.Canvas canvas;
        
        public Canvas() {
            canvas = new javafx.scene.canvas.Canvas();
            getChildren().add(canvas);
            
            // Bind canvas size to pane size
            widthProperty().addListener((obs, oldVal, newVal) -> {
                canvas.setWidth(newVal.doubleValue());
            });
            heightProperty().addListener((obs, oldVal, newVal) -> {
                canvas.setHeight(newVal.doubleValue());
            });
        }
        
        public javafx.scene.canvas.GraphicsContext getGraphicsContext2D() {
            return canvas.getGraphicsContext2D();
        }
        
        public double getWidth() {
            return canvas.getWidth();
        }
        
        public double getHeight() {
            return canvas.getHeight();
        }
        
        public void setPrefHeight(double height) {
            canvas.setHeight(height);
            super.setPrefHeight(height);
        }
    }
}
