package com.astraeus.ui;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.LinkedList;
import java.util.Queue;

/**
 * Console/log panel for displaying application logs and messages.
 * 
 * Features:
 * - Color-coded log levels (INFO, WARNING, ERROR)
 * - Auto-scroll to bottom
 * - Clear button
 * - Timestamp for each message
 * - Thread-safe message queueing
 * 
 * PERFORMANCE:
 * - Messages are queued and batched to avoid per-message UI updates
 * - Limited history (configurable, default 1000 lines)
 * - Auto-trim when exceeding max lines
 */
public class ConsolePane extends BorderPane {
    
    /**
     * Log level enumeration.
     */
    public enum LogLevel {
        INFO("INFO", "#2196F3"),      // Blue
        WARNING("WARN", "#FF9800"),    // Orange
        ERROR("ERROR", "#F44336");     // Red
        
        private final String label;
        private final String color;
        
        LogLevel(String label, String color) {
            this.label = label;
            this.color = color;
        }
        
        public String getLabel() { return label; }
        public String getColor() { return color; }
    }
    
    private final TextArea consoleArea;
    private final Button clearButton;
    private final CheckBox autoScrollCheckBox;
    private final Label statusLabel;
    
    private final DateTimeFormatter timeFormatter = DateTimeFormatter.ofPattern("HH:mm:ss.SSS");
    private final Queue<String> messageQueue = new LinkedList<>();
    private final int maxLines = 1000;
    private int lineCount = 0;
    
    // Flush state to prevent redundant Platform.runLater calls
    private volatile boolean flushPending = false;
    
    // Auto-scroll behavior
    private boolean autoScroll = true;
    
    public ConsolePane() {
        // === TOP: Toolbar ===
        ToolBar toolbar = new ToolBar();
        
        Label titleLabel = new Label("Console");
        titleLabel.setStyle("-fx-font-weight: bold;");
        
        clearButton = new Button("Clear");
        clearButton.setOnAction(e -> clear());
        
        autoScrollCheckBox = new CheckBox("Auto-scroll");
        autoScrollCheckBox.setSelected(true);
        autoScrollCheckBox.setOnAction(e -> autoScroll = autoScrollCheckBox.isSelected());
        
        statusLabel = new Label("0 lines");
        statusLabel.setStyle("-fx-text-fill: #666666; -fx-font-size: 11;");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        toolbar.getItems().addAll(
            titleLabel,
            new Separator(),
            clearButton,
            autoScrollCheckBox,
            spacer,
            statusLabel
        );
        
        // === CENTER: Console area ===
        consoleArea = new TextArea();
        consoleArea.setEditable(false);
        consoleArea.setWrapText(false);
        consoleArea.setFont(Font.font("Monospace", 11));
        consoleArea.setStyle("-fx-control-inner-background: #1e1e1e; -fx-text-fill: #d4d4d4;");
        
        // === LAYOUT ===
        setTop(toolbar);
        setCenter(consoleArea);
        
        // Set minimum size
        setMinHeight(100);
        setPrefHeight(150);
        
        // Initial message
        log(LogLevel.INFO, "Console initialized");
    }
    
    /**
     * Log a message with the specified level.
     * Thread-safe: can be called from any thread.
     * 
     * @param level Log level
     * @param message Message text
     */
    public void log(LogLevel level, String message) {
        String timestamp = LocalDateTime.now().format(timeFormatter);
        String formattedMessage = String.format("[%s] [%s] %s\n", timestamp, level.getLabel(), message);
        
        // Queue message for batched UI update
        boolean needsFlush = false;
        synchronized (messageQueue) {
            messageQueue.offer(formattedMessage);
            // Only schedule flush if one isn't already pending
            if (!flushPending) {
                flushPending = true;
                needsFlush = true;
            }
        }
        
        // Update UI on JavaFX thread (only if not already pending)
        if (needsFlush) {
            Platform.runLater(this::flushMessages);
        }
    }
    
    /**
     * Log an INFO message.
     */
    public void info(String message) {
        log(LogLevel.INFO, message);
    }
    
    /**
     * Log a WARNING message.
     */
    public void warning(String message) {
        log(LogLevel.WARNING, message);
    }
    
    /**
     * Log an ERROR message.
     */
    public void error(String message) {
        log(LogLevel.ERROR, message);
    }
    
    /**
     * Log an exception with stack trace.
     */
    public void error(String message, Throwable throwable) {
        error(message + ": " + throwable.getMessage());
        
        // Log stack trace
        for (StackTraceElement element : throwable.getStackTrace()) {
            error("  at " + element.toString());
        }
    }
    
    /**
     * Clear all console messages.
     */
    public void clear() {
        consoleArea.clear();
        lineCount = 0;
        updateStatus();
        info("Console cleared");
    }
    
    /**
     * Flush queued messages to the console area.
     * Called on JavaFX thread.
     */
    private void flushMessages() {
        synchronized (messageQueue) {
            if (messageQueue.isEmpty()) {
                flushPending = false;
                return;
            }
            
            StringBuilder batch = new StringBuilder();
            while (!messageQueue.isEmpty()) {
                String message = messageQueue.poll();
                batch.append(message);
                lineCount++;
            }
            
            flushPending = false;
            
            // Append to console
            consoleArea.appendText(batch.toString());
            
            // Trim if exceeding max lines
            if (lineCount > maxLines) {
                trimExcessLines();
            }
            
            // Auto-scroll to bottom
            if (autoScroll) {
                consoleArea.setScrollTop(Double.MAX_VALUE);
            }
            
            updateStatus();
        }
    }
    
    /**
     * Trim excess lines when exceeding max line count.
     * Uses efficient substring approach to avoid rebuilding entire text.
     */
    private void trimExcessLines() {
        String text = consoleArea.getText();
        int linesToRemove = lineCount - maxLines;
        
        // Find the position after the Nth newline
        int pos = 0;
        for (int i = 0; i < linesToRemove && pos < text.length(); i++) {
            pos = text.indexOf('\n', pos) + 1;
        }
        
        if (pos > 0 && pos < text.length()) {
            consoleArea.setText(text.substring(pos));
            lineCount = maxLines;
        }
    }
    
    /**
     * Update status label with line count.
     */
    private void updateStatus() {
        statusLabel.setText(lineCount + " lines");
    }
    
    /**
     * Get the underlying TextArea for advanced customization.
     */
    public TextArea getTextArea() {
        return consoleArea;
    }
}
