package com.astraeus.native_api.lifecycle;

public class ViewportConfig {
    private int width;
    private int height;
    private float aspectRatio;
    
    public ViewportConfig(int width, int height) {
        this.width = width;
        this.height = height;
        this.aspectRatio = (float) width / (float) height;
    }
    
    public ViewportConfig(int width, int height, float aspectRatio) {
        this.width = width;
        this.height = height;
        this.aspectRatio = aspectRatio;
    }
    
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public float getAspectRatio() { return aspectRatio; }
}
