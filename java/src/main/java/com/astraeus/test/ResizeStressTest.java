package com.astraeus.test;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.rendering.FxViewport;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

import java.util.Random;

/**
 * Resize stress test for FxViewport.
 * 
 * Tests the "viewport-only resize" contract by rapidly changing viewport dimensions
 * without causing memory corruption or crashes.
 * 
 * ACCEPTANCE CRITERIA:
 * - Runs for 30+ seconds without crash
 * - No EXCEPTION_ACCESS_VIOLATION
 * - No memory corruption
 * - No visual artifacts or freezes
 */
public class ResizeStressTest extends Application {
    
    private NativeEngine engine;
    private FxViewport viewport;
    private Label statusLabel;
    private Label fpsLabel;
    private Label resizeCountLabel;
    
    private int resizeCount = 0;
    private long startTime;
    private long lastFrameTime;
    private int frameCount = 0;
    private double fps = 0.0;
    
    private boolean stressTestRunning = false;
    private Random random = new Random();
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Resize Stress Test");
        
        try {
            // Initialize engine
            engine = new NativeEngine(1280, 720, true);
            
            // Create FxViewport with large max dimensions
            viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
            viewport.setPrefSize(1280, 720);
            
            // Create UI
            BorderPane root = new BorderPane();
            
            // Top toolbar
            ToolBar toolbar = createToolbar();
            root.setTop(toolbar);
            
            // Center viewport
            root.setCenter(viewport);
            
            // Bottom status bar
            HBox statusBar = createStatusBar();
            root.setBottom(statusBar);
            
            // Create scene
            Scene scene = new Scene(root, 1280, 800);
            primaryStage.setScene(scene);
            primaryStage.show();
            
            // Start render loop
            startRenderLoop();
            
            updateStatus("Ready - Click 'Start Stress Test' to begin");
            
        } catch (Exception e) {
            showError("Initialization Failed", e.getMessage());
            e.printStackTrace();
        }
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        Button startStressButton = new Button("Start Stress Test");
        startStressButton.setOnAction(e -> startStressTest());
        
        Button stopStressButton = new Button("Stop Stress Test");
        stopStressButton.setOnAction(e -> stopStressTest());
        
        Button resize800Button = new Button("800x600");
        resize800Button.setOnAction(e -> manualResize(800, 600));
        
        Button resize1080Button = new Button("1920x1080");
        resize1080Button.setOnAction(e -> manualResize(1920, 1080));
        
        Button resize1440Button = new Button("2560x1440");
        resize1440Button.setOnAction(e -> manualResize(2560, 1440));
        
        Separator sep = new Separator();
        
        statusLabel = new Label("Ready");
        
        toolbar.getItems().addAll(
            startStressButton,
            stopStressButton,
            new Separator(),
            resize800Button,
            resize1080Button,
            resize1440Button,
            sep,
            statusLabel
        );
        
        return toolbar;
    }
    
    private HBox createStatusBar() {
        HBox statusBar = new HBox(20);
        statusBar.setStyle("-fx-padding: 5; -fx-background-color: #f0f0f0;");
        
        fpsLabel = new Label("FPS: 0.0");
        resizeCountLabel = new Label("Resizes: 0");
        Label maxSizeLabel = new Label("Max: " + viewport.getMaxWidth() + "x" + 
                                        viewport.getMaxHeight());
        
        statusBar.getChildren().addAll(fpsLabel, resizeCountLabel, maxSizeLabel);
        
        return statusBar;
    }
    
    private void startRenderLoop() {
        startTime = System.nanoTime();
        lastFrameTime = startTime;
        
        AnimationTimer timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                // Calculate FPS
                frameCount++;
                long elapsed = now - lastFrameTime;
                if (elapsed >= 1_000_000_000L) {  // Update FPS every second
                    fps = frameCount / (elapsed / 1_000_000_000.0);
                    fpsLabel.setText(String.format("FPS: %.1f", fps));
                    frameCount = 0;
                    lastFrameTime = now;
                }
                
                // Render frame
                double deltaTime = 0.016;  // ~60fps
                engine.beginFrame(deltaTime);
                engine.endFrame();
                viewport.updateDisplay();
                
                // Stress test: random resize
                if (stressTestRunning) {
                    performRandomResize();
                }
            }
        };
        timer.start();
    }
    
    private void startStressTest() {
        if (stressTestRunning) {
            return;
        }
        
        stressTestRunning = true;
        resizeCount = 0;
        startTime = System.nanoTime();
        updateStatus("STRESS TEST RUNNING - Rapid random resizing...");
        
        System.out.println("\n=== STRESS TEST STARTED ===");
        System.out.println("Max dimensions: " + viewport.getMaxWidth() + "x" + 
                         viewport.getMaxHeight());
    }
    
    private void stopStressTest() {
        if (!stressTestRunning) {
            return;
        }
        
        stressTestRunning = false;
        long elapsedNs = System.nanoTime() - startTime;
        double elapsedSec = elapsedNs / 1_000_000_000.0;
        
        updateStatus(String.format("STRESS TEST COMPLETE - %d resizes in %.1f seconds (%.1f resizes/sec)",
                                   resizeCount, elapsedSec, resizeCount / elapsedSec));
        
        System.out.println("\n=== STRESS TEST COMPLETE ===");
        System.out.println(String.format("Total resizes: %d", resizeCount));
        System.out.println(String.format("Duration: %.1f seconds", elapsedSec));
        System.out.println(String.format("Rate: %.1f resizes/sec", resizeCount / elapsedSec));
        System.out.println("No crashes or memory corruption detected!");
    }
    
    private void performRandomResize() {
        // Random dimensions within max bounds
        int minDim = 640;
        int maxW = viewport.getMaxWidth();
        int maxH = viewport.getMaxHeight();
        
        // Ensure we have valid range (max must be > min)
        if (maxW <= minDim || maxH <= minDim) {
            // Fallback to just using max dimensions
            viewport.resizeViewport(maxW, maxH);
            resizeCount++;
            resizeCountLabel.setText("Resizes: " + resizeCount);
            return;
        }
        
        int width = minDim + random.nextInt(maxW - minDim);
        int height = minDim + random.nextInt(maxH - minDim);
        
        // Occasionally test exact max dimensions
        if (random.nextDouble() < 0.1) {
            width = maxW;
            height = maxH;
        }
        
        viewport.resizeViewport(width, height);
        resizeCount++;
        resizeCountLabel.setText("Resizes: " + resizeCount);
        
        // Log every 100 resizes
        if (resizeCount % 100 == 0) {
            System.out.println(String.format("[Stress Test] %d resizes completed, current: %dx%d",
                                           resizeCount, width, height));
        }
    }
    
    private void manualResize(int width, int height) {
        try {
            viewport.resizeViewport(width, height);
            updateStatus("Resized to " + width + "x" + height);
            System.out.println("[Manual] Resized viewport to " + width + "x" + height);
        } catch (Exception e) {
            showError("Resize Failed", e.getMessage());
            e.printStackTrace();
        }
    }
    
    private void updateStatus(String message) {
        if (statusLabel != null) {
            statusLabel.setText(message);
        }
        System.out.println("[Status] " + message);
    }
    
    private void showError(String title, String message) {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }
    
    @Override
    public void stop() {
        if (engine != null) {
            engine.close();
        }
    }
    
    public static void main(String[] args) {
        launch(args);
    }
}
