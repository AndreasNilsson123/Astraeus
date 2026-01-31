package com.astraeus.native_api.lifecycle;

public class EngineConfig {
    private int initialWidth = 1280;
    private int initialHeight = 720;
    private boolean enableValidation = false;
    private boolean enableDebugOutput = false;
    private String logFilePath = null;
    
    public EngineConfig() {}
    
    public EngineConfig setInitialWidth(int width) {
        this.initialWidth = width;
        return this;
    }
    
    public EngineConfig setInitialHeight(int height) {
        this.initialHeight = height;
        return this;
    }
    
    public EngineConfig setInitialSize(int width, int height) {
        this.initialWidth = width;
        this.initialHeight = height;
        return this;
    }
    
    public EngineConfig setEnableValidation(boolean enable) {
        this.enableValidation = enable;
        return this;
    }
    
    public EngineConfig setEnableDebugOutput(boolean enable) {
        this.enableDebugOutput = enable;
        return this;
    }
    
    public EngineConfig setLogFilePath(String path) {
        this.logFilePath = path;
        return this;
    }
    
    public int getInitialWidth() { return initialWidth; }
    public int getInitialHeight() { return initialHeight; }
    public boolean isEnableValidation() { return enableValidation; }
    public boolean isEnableDebugOutput() { return enableDebugOutput; }
    public String getLogFilePath() { return logFilePath; }
}
