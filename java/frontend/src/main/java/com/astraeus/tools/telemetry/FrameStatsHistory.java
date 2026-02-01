package com.astraeus.tools.telemetry;

import com.astraeus.native_api.model.FrameStats;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Ring buffer for storing frame statistics history.
 * Efficiently stores recent frame stats for charting and analysis.
 * 
 * <p>Thread Safety: Not thread-safe. Designed for single-threaded UI use.</p>
 * 
 * <p>Usage:</p>
 * <pre>{@code
 * FrameStatsHistory history = new FrameStatsHistory(300); // Last 300 frames
 * 
 * // In update loop:
 * FrameStats stats = engine.getTelemetryStats();
 * history.add(stats);
 * 
 * // Get history for charting:
 * List<FrameStats> recent = history.getAll();
 * }</pre>
 */
public class FrameStatsHistory {
    
    private final int maxSize;
    private final List<FrameStats> buffer;
    private int insertIndex = 0;
    private boolean filled = false;
    
    /**
     * Create a new frame stats history buffer.
     * 
     * @param maxSize Maximum number of frames to store
     * @throws IllegalArgumentException if maxSize <= 0
     */
    public FrameStatsHistory(int maxSize) {
        if (maxSize <= 0) {
            throw new IllegalArgumentException("maxSize must be positive");
        }
        this.maxSize = maxSize;
        this.buffer = new ArrayList<>(maxSize);
    }
    
    /**
     * Add a frame stats entry to the history.
     * When the buffer is full, oldest entries are overwritten.
     * 
     * @param stats Frame statistics to add (null values are ignored)
     */
    public void add(FrameStats stats) {
        if (stats == null) {
            return;
        }
        
        if (!filled && buffer.size() < maxSize) {
            // Still filling initial buffer
            buffer.add(stats);
            if (buffer.size() >= maxSize) {
                filled = true;
                insertIndex = 0;
            }
        } else {
            // Overwrite oldest entry (ring buffer behavior)
            if (insertIndex >= buffer.size()) {
                insertIndex = 0;
            }
            buffer.set(insertIndex, stats);
            insertIndex = (insertIndex + 1) % maxSize;
        }
    }
    
    /**
     * Get all frame stats in chronological order (oldest first).
     * 
     * @return Unmodifiable list of frame stats (may be empty, never null)
     */
    public List<FrameStats> getAll() {
        if (buffer.isEmpty()) {
            return Collections.emptyList();
        }
        
        if (!filled) {
            // Buffer not yet full, return in insertion order
            return Collections.unmodifiableList(buffer);
        }
        
        // Reorder ring buffer to chronological order
        List<FrameStats> ordered = new ArrayList<>(maxSize);
        for (int i = 0; i < maxSize; i++) {
            int index = (insertIndex + i) % maxSize;
            if (index < buffer.size()) {
                ordered.add(buffer.get(index));
            }
        }
        return Collections.unmodifiableList(ordered);
    }
    
    /**
     * Get the most recent N frame stats in chronological order.
     * 
     * @param count Number of recent frames to retrieve
     * @return Unmodifiable list of frame stats (may be smaller than count if fewer frames are available)
     */
    public List<FrameStats> getRecent(int count) {
        List<FrameStats> all = getAll();
        if (count <= 0 || all.isEmpty()) {
            return Collections.emptyList();
        }
        
        int start = Math.max(0, all.size() - count);
        return all.subList(start, all.size());
    }
    
    /**
     * Get the latest (most recent) frame stats.
     * 
     * @return Latest frame stats, or null if history is empty
     */
    public FrameStats getLatest() {
        if (buffer.isEmpty()) {
            return null;
        }
        
        if (!filled) {
            return buffer.get(buffer.size() - 1);
        }
        
        // Most recent is just before insertIndex
        int latestIndex = (insertIndex - 1 + maxSize) % maxSize;
        return buffer.get(latestIndex);
    }
    
    /**
     * Get the number of frames currently stored in history.
     * 
     * @return Number of frames (0 to maxSize)
     */
    public int size() {
        return buffer.size();
    }
    
    /**
     * Get the maximum capacity of this history buffer.
     * 
     * @return Maximum number of frames
     */
    public int getCapacity() {
        return maxSize;
    }
    
    /**
     * Check if the buffer is empty.
     * 
     * @return true if no frames are stored
     */
    public boolean isEmpty() {
        return buffer.isEmpty();
    }
    
    /**
     * Clear all stored frame stats.
     */
    public void clear() {
        buffer.clear();
        insertIndex = 0;
        filled = false;
    }
}
