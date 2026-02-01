package com.astraeus.util;

import java.lang.foreign.*;
import java.nio.ByteOrder;

/**
 * FFM (Foreign Function & Memory API) utility functions.
 */
public class FFMUtils {
    
    private static final Arena GLOBAL_ARENA = Arena.global();
    
    /**
     * Allocate native memory in the global arena.
     */
    public static MemorySegment allocate(long size) {
        return GLOBAL_ARENA.allocate(size, 8); // 8-byte alignment
    }
    
    /**
     * Allocate native memory for a value layout.
     */
    public static MemorySegment allocate(ValueLayout layout) {
        return GLOBAL_ARENA.allocate(layout);
    }
    
    /**
     * Allocate native memory for a struct layout.
     */
    public static MemorySegment allocate(MemoryLayout layout) {
        return GLOBAL_ARENA.allocate(layout);
    }
    
    /**
     * Create a null pointer (address 0).
     */
    public static MemorySegment nullPointer() {
        return MemorySegment.NULL;
    }
    
    /**
     * Check if memory segment is null.
     */
    public static boolean isNull(MemorySegment segment) {
        return segment == null || segment.address() == 0;
    }
    
    /**
     * Get byte order for the current platform.
     */
    public static ByteOrder nativeByteOrder() {
        return ByteOrder.nativeOrder();
    }
    
    /**
     * Convert Java string to null-terminated C string.
     */
    public static MemorySegment toCString(String str) {
        if (str == null) {
            return nullPointer();
        }
        return GLOBAL_ARENA.allocateFrom(str);
    }
    
    /**
     * Convert C string to Java string.
     */
    public static String fromCString(MemorySegment segment) {
        if (isNull(segment)) {
            return null;
        }
        return segment.getString(0);
    }
    
    /**
     * Copy data from Java array to native memory.
     */
    public static void copyToNative(float[] source, MemorySegment destination) {
        for (int i = 0; i < source.length; i++) {
            destination.setAtIndex(ValueLayout.JAVA_FLOAT, i, source[i]);
        }
    }
    
    /**
     * Copy data from Java array to native memory.
     */
    public static void copyToNative(int[] source, MemorySegment destination) {
        for (int i = 0; i < source.length; i++) {
            destination.setAtIndex(ValueLayout.JAVA_INT, i, source[i]);
        }
    }
    
    /**
     * Copy data from native memory to Java array.
     */
    public static void copyFromNative(MemorySegment source, float[] destination) {
        for (int i = 0; i < destination.length; i++) {
            destination[i] = source.getAtIndex(ValueLayout.JAVA_FLOAT, i);
        }
    }
    
    /**
     * Copy data from native memory to Java array.
     */
    public static void copyFromNative(MemorySegment source, int[] destination) {
        for (int i = 0; i < destination.length; i++) {
            destination[i] = source.getAtIndex(ValueLayout.JAVA_INT, i);
        }
    }
    
    /**
     * Get address of a memory segment as a long.
     */
    public static long addressOf(MemorySegment segment) {
        return segment.address();
    }
    
    /**
     * Reinterpret a memory segment with a new size.
     */
    public static MemorySegment reinterpret(MemorySegment segment, long newSize) {
        return segment.reinterpret(newSize);
    }
}
