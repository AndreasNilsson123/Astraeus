package com.astraeus.util;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Debug utility functions for the Astraeus application.
 */
public class DebugUtils {
    
    private static final Logger LOGGER = Logger.getLogger("Astraeus");
    private static boolean debugEnabled = false;
    
    /**
     * Enable or disable debug mode.
     */
    public static void setDebugEnabled(boolean enabled) {
        debugEnabled = enabled;
        if (enabled) {
            LOGGER.setLevel(Level.FINE);
        } else {
            LOGGER.setLevel(Level.INFO);
        }
    }
    
    /**
     * Check if debug mode is enabled.
     */
    public static boolean isDebugEnabled() {
        return debugEnabled;
    }
    
    /**
     * Log debug message (only if debug is enabled).
     */
    public static void debug(String message) {
        if (debugEnabled) {
            LOGGER.fine(message);
        }
    }
    
    /**
     * Log debug message with format arguments.
     */
    public static void debug(String format, Object... args) {
        if (debugEnabled) {
            LOGGER.fine(String.format(format, args));
        }
    }
    
    /**
     * Log info message.
     */
    public static void info(String message) {
        LOGGER.info(message);
    }
    
    /**
     * Log warning message.
     */
    public static void warning(String message) {
        LOGGER.warning(message);
    }
    
    /**
     * Log error message.
     */
    public static void error(String message) {
        LOGGER.severe(message);
    }
    
    /**
     * Log error message with exception.
     */
    public static void error(String message, Throwable throwable) {
        LOGGER.log(Level.SEVERE, message, throwable);
    }
    
    /**
     * Assert condition (throws exception if false and debug is enabled).
     */
    public static void assertCondition(boolean condition, String message) {
        if (debugEnabled && !condition) {
            throw new AssertionError(message);
        }
    }
    
    /**
     * Measure execution time of a runnable.
     */
    public static long measureTime(Runnable task) {
        long start = System.nanoTime();
        task.run();
        long end = System.nanoTime();
        return (end - start) / 1_000_000; // Convert to milliseconds
    }
    
    /**
     * Measure and log execution time.
     */
    public static void measureAndLog(String taskName, Runnable task) {
        long time = measureTime(task);
        debug("%s took %d ms", taskName, time);
    }
    
    /**
     * Print current thread information.
     */
    public static void printThreadInfo() {
        Thread current = Thread.currentThread();
        debug("Thread: %s (ID: %d, Priority: %d)", 
              current.getName(), 
              current.getId(), 
              current.getPriority());
    }
}
