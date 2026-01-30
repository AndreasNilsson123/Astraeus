package com.astraeus.ui;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Properties;

/**
 * Layout configuration manager for workspace persistence.
 * 
 * Saves and loads:
 * - Window size and position
 * - Split pane divider positions
 * - Pane visibility states
 * 
 * Configuration is stored in: ~/.astraeus/workspace-layout.properties
 * 
 * USAGE:
 * <pre>
 * LayoutConfig config = LayoutConfig.load();
 * 
 * // Get values with defaults
 * double dividerPos = config.getDividerPosition("left", 0.2);
 * boolean visible = config.isPaneVisible("console", true);
 * 
 * // Set values
 * config.setDividerPosition("left", 0.25);
 * config.setPaneVisible("console", false);
 * 
 * // Save to disk
 * config.save();
 * </pre>
 */
public class LayoutConfig {
    
    private static final String CONFIG_DIR = ".astraeus";
    private static final String CONFIG_FILE = "workspace-layout.properties";
    
    private final Properties properties;
    private final Path configPath;
    
    /**
     * Private constructor. Use load() to create instances.
     */
    private LayoutConfig(Properties properties, Path configPath) {
        this.properties = properties;
        this.configPath = configPath;
    }
    
    /**
     * Load configuration from disk.
     * Creates a new default configuration if file doesn't exist or is corrupt.
     * 
     * @return LayoutConfig instance
     */
    public static LayoutConfig load() {
        Path configPath = getConfigPath();
        Properties properties = new Properties();
        
        // Try to load existing configuration
        if (Files.exists(configPath)) {
            try (InputStream in = Files.newInputStream(configPath)) {
                properties.load(in);
                System.out.println("[LayoutConfig] Loaded configuration from: " + configPath);
            } catch (IOException e) {
                System.err.println("[LayoutConfig] Failed to load configuration: " + e.getMessage());
                System.err.println("[LayoutConfig] Using default configuration");
                properties = createDefaultProperties();
            }
        } else {
            System.out.println("[LayoutConfig] No existing configuration found, using defaults");
            properties = createDefaultProperties();
        }
        
        return new LayoutConfig(properties, configPath);
    }
    
    /**
     * Save configuration to disk.
     * Creates parent directory if it doesn't exist.
     * 
     * @return true if save succeeded, false otherwise
     */
    public boolean save() {
        try {
            // Ensure parent directory exists
            Files.createDirectories(configPath.getParent());
            
            // Save properties
            try (OutputStream out = Files.newOutputStream(configPath)) {
                properties.store(out, "Astraeus Workspace Layout Configuration");
            }
            
            System.out.println("[LayoutConfig] Saved configuration to: " + configPath);
            return true;
            
        } catch (IOException e) {
            System.err.println("[LayoutConfig] Failed to save configuration: " + e.getMessage());
            return false;
        }
    }
    
    // ==================== Window Properties ====================
    
    /**
     * Get window width.
     */
    public double getWindowWidth(double defaultValue) {
        return getDouble("window.width", defaultValue);
    }
    
    /**
     * Set window width.
     */
    public void setWindowWidth(double width) {
        setDouble("window.width", width);
    }
    
    /**
     * Get window height.
     */
    public double getWindowHeight(double defaultValue) {
        return getDouble("window.height", defaultValue);
    }
    
    /**
     * Set window height.
     */
    public void setWindowHeight(double height) {
        setDouble("window.height", height);
    }
    
    /**
     * Get window X position.
     */
    public double getWindowX(double defaultValue) {
        return getDouble("window.x", defaultValue);
    }
    
    /**
     * Set window X position.
     */
    public void setWindowX(double x) {
        setDouble("window.x", x);
    }
    
    /**
     * Get window Y position.
     */
    public double getWindowY(double defaultValue) {
        return getDouble("window.y", defaultValue);
    }
    
    /**
     * Set window Y position.
     */
    public void setWindowY(double y) {
        setDouble("window.y", y);
    }
    
    /**
     * Check if window is maximized.
     */
    public boolean isWindowMaximized(boolean defaultValue) {
        return getBoolean("window.maximized", defaultValue);
    }
    
    /**
     * Set window maximized state.
     */
    public void setWindowMaximized(boolean maximized) {
        setBoolean("window.maximized", maximized);
    }
    
    // ==================== Divider Positions ====================
    
    /**
     * Get divider position for a split pane.
     * 
     * @param name Divider name (e.g., "main.horizontal", "left.vertical")
     * @param defaultValue Default position (0.0 to 1.0)
     * @return Divider position
     */
    public double getDividerPosition(String name, double defaultValue) {
        return getDouble("divider." + name, defaultValue);
    }
    
    /**
     * Set divider position for a split pane.
     * 
     * @param name Divider name
     * @param position Position (0.0 to 1.0)
     */
    public void setDividerPosition(String name, double position) {
        setDouble("divider." + name, position);
    }
    
    // ==================== Pane Visibility ====================
    
    /**
     * Check if a pane is visible.
     * 
     * @param paneName Pane name (e.g., "scene-inspector", "console", "properties")
     * @param defaultValue Default visibility
     * @return true if pane should be visible
     */
    public boolean isPaneVisible(String paneName, boolean defaultValue) {
        return getBoolean("pane." + paneName + ".visible", defaultValue);
    }
    
    /**
     * Set pane visibility.
     * 
     * @param paneName Pane name
     * @param visible Visibility state
     */
    public void setPaneVisible(String paneName, boolean visible) {
        setBoolean("pane." + paneName + ".visible", visible);
    }
    
    // ==================== Internal Helpers ====================
    
    /**
     * Get double property with default.
     */
    private double getDouble(String key, double defaultValue) {
        String value = properties.getProperty(key);
        if (value != null) {
            try {
                return Double.parseDouble(value);
            } catch (NumberFormatException e) {
                System.err.println("[LayoutConfig] Invalid double for key '" + key + "': " + value);
            }
        }
        return defaultValue;
    }
    
    /**
     * Set double property.
     */
    private void setDouble(String key, double value) {
        properties.setProperty(key, String.valueOf(value));
    }
    
    /**
     * Get boolean property with default.
     */
    private boolean getBoolean(String key, boolean defaultValue) {
        String value = properties.getProperty(key);
        if (value != null) {
            return Boolean.parseBoolean(value);
        }
        return defaultValue;
    }
    
    /**
     * Set boolean property.
     */
    private void setBoolean(String key, boolean value) {
        properties.setProperty(key, String.valueOf(value));
    }
    
    /**
     * Get configuration file path.
     * Returns: ~/.astraeus/workspace-layout.properties
     * Falls back to current directory if user.home is not available.
     */
    private static Path getConfigPath() {
        String userHome = System.getProperty("user.home");
        if (userHome == null || userHome.isEmpty()) {
            System.err.println("[LayoutConfig] user.home property not set, using current directory");
            userHome = System.getProperty("user.dir", ".");
        }
        return Paths.get(userHome, CONFIG_DIR, CONFIG_FILE);
    }
    
    /**
     * Create default properties.
     */
    private static Properties createDefaultProperties() {
        Properties props = new Properties();
        
        // Window defaults
        props.setProperty("window.width", "1600");
        props.setProperty("window.height", "900");
        props.setProperty("window.maximized", "false");
        
        // Divider defaults
        props.setProperty("divider.main.horizontal", "0.75");  // Main horizontal split (top vs bottom)
        props.setProperty("divider.main.vertical", "0.20");    // Left split
        props.setProperty("divider.right.vertical", "0.80");   // Right split
        
        // Pane visibility defaults
        props.setProperty("pane.scene-inspector.visible", "true");
        props.setProperty("pane.properties.visible", "true");
        props.setProperty("pane.console.visible", "true");
        props.setProperty("pane.telemetry.visible", "false");
        
        return props;
    }
    
    /**
     * Get all properties (for debugging).
     */
    public Properties getProperties() {
        return properties;
    }
    
    /**
     * Print configuration to console (for debugging).
     */
    public void print() {
        System.out.println("=== Layout Configuration ===");
        properties.forEach((key, value) -> {
            System.out.println("  " + key + " = " + value);
        });
        System.out.println("===========================");
    }
}
