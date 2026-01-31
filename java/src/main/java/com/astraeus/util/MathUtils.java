package com.astraeus.util;

/**
 * Math utility functions for the Astraeus Java application.
 * Simple, non-domain-specific math helpers.
 */
public class MathUtils {
    
    /**
     * Clamp a value between min and max.
     */
    public static int clamp(int value, int min, int max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Clamp a value between min and max.
     */
    public static float clamp(float value, float min, float max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Clamp a value between min and max.
     */
    public static double clamp(double value, double min, double max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Linear interpolation between two values.
     */
    public static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    /**
     * Linear interpolation between two values.
     */
    public static double lerp(double a, double b, double t) {
        return a + (b - a) * t;
    }
    
    /**
     * Check if two floats are approximately equal.
     */
    public static boolean approxEqual(float a, float b, float epsilon) {
        return Math.abs(a - b) < epsilon;
    }
    
    /**
     * Check if two doubles are approximately equal.
     */
    public static boolean approxEqual(double a, double b, double epsilon) {
        return Math.abs(a - b) < epsilon;
    }
    
    /**
     * Convert degrees to radians.
     */
    public static float toRadians(float degrees) {
        return (float) Math.toRadians(degrees);
    }
    
    /**
     * Convert radians to degrees.
     */
    public static float toDegrees(float radians) {
        return (float) Math.toDegrees(radians);
    }
    
    /**
     * Map a value from one range to another.
     */
    public static float map(float value, float fromMin, float fromMax, float toMin, float toMax) {
        float normalized = (value - fromMin) / (fromMax - fromMin);
        return toMin + normalized * (toMax - toMin);
    }
    
    /**
     * Map a value from one range to another.
     */
    public static double map(double value, double fromMin, double fromMax, double toMin, double toMax) {
        double normalized = (value - fromMin) / (fromMax - fromMin);
        return toMin + normalized * (toMax - toMin);
    }
    
    /**
     * Check if value is within range [min, max].
     */
    public static boolean inRange(int value, int min, int max) {
        return value >= min && value <= max;
    }
    
    /**
     * Check if value is within range [min, max].
     */
    public static boolean inRange(float value, float min, float max) {
        return value >= min && value <= max;
    }
    
    /**
     * Check if value is within range [min, max].
     */
    public static boolean inRange(double value, double min, double max) {
        return value >= min && value <= max;
    }
}
