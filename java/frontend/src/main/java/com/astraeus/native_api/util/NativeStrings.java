package com.astraeus.native_api.util;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.nio.charset.StandardCharsets;

/**
 * Utility class for safe string operations across the FFM boundary.
 * 
 * <p>Provides bounded, null-terminated string read/write helpers that prevent
 * buffer overruns and ensure consistent string handling patterns.
 * 
 * <p><b>Key Features:</b>
 * <ul>
 *   <li>Bounded reads: Never reads beyond specified buffer size</li>
 *   <li>Null-terminator handling: Properly handles C-style null-terminated strings</li>
 *   <li>UTF-8 encoding: All strings use UTF-8 encoding</li>
 *   <li>Safe copies: Prevents buffer overflows when writing</li>
 * </ul>
 */
public final class NativeStrings {
    
    private NativeStrings() {
        // Utility class, no instances
    }
    
    /**
     * Reads a null-terminated C string from a memory segment.
     * 
     * <p>Reads up to {@code maxLength} bytes or until a null terminator is found,
     * whichever comes first. The returned string does not include the null terminator.
     * 
     * @param segment the memory segment containing the string
     * @param maxLength maximum number of bytes to read (including null terminator)
     * @return the decoded string, or empty string if segment is null or maxLength is 0
     * @throws IllegalArgumentException if maxLength is negative
     */
    public static String readCString(MemorySegment segment, int maxLength) {
        if (maxLength < 0) {
            throw new IllegalArgumentException("maxLength must be non-negative");
        }
        
        if (segment == null || segment == MemorySegment.NULL || maxLength == 0) {
            return "";
        }
        
        // Find the null terminator
        int length = 0;
        while (length < maxLength && segment.get(java.lang.foreign.ValueLayout.JAVA_BYTE, length) != 0) {
            length++;
        }
        
        if (length == 0) {
            return "";
        }
        
        // Copy bytes and decode
        byte[] bytes = new byte[length];
        MemorySegment.copy(segment, java.lang.foreign.ValueLayout.JAVA_BYTE, 0, bytes, 0, length);
        return new String(bytes, StandardCharsets.UTF_8);
    }
    
    /**
     * Reads a null-terminated C string from a pointer address.
     * 
     * <p>This is a convenience method that wraps the address in a MemorySegment
     * and calls {@link #readCString(MemorySegment, int)}.
     * 
     * @param address the memory address of the string
     * @param maxLength maximum number of bytes to read (including null terminator)
     * @return the decoded string, or empty string if address is 0
     * @throws IllegalArgumentException if maxLength is negative
     */
    public static String readCString(long address, int maxLength) {
        if (address == 0) {
            return "";
        }
        MemorySegment segment = MemorySegment.ofAddress(address).reinterpret(maxLength);
        return readCString(segment, maxLength);
    }
    
    /**
     * Writes a Java string to a memory segment as a null-terminated C string.
     * 
     * <p>The string is encoded as UTF-8 and a null terminator is appended.
     * If the encoded string (including null terminator) exceeds {@code maxLength},
     * it is truncated to fit.
     * 
     * @param segment the destination memory segment
     * @param str the string to write (null is treated as empty string)
     * @param maxLength maximum number of bytes to write (including null terminator)
     * @return the number of bytes written (including null terminator)
     * @throws IllegalArgumentException if maxLength is less than 1
     */
    public static int writeCString(MemorySegment segment, String str, int maxLength) {
        if (maxLength < 1) {
            throw new IllegalArgumentException("maxLength must be at least 1 (for null terminator)");
        }
        
        if (segment == null || segment == MemorySegment.NULL) {
            return 0;
        }
        
        if (str == null || str.isEmpty()) {
            segment.set(java.lang.foreign.ValueLayout.JAVA_BYTE, 0, (byte) 0);
            return 1;
        }
        
        // Encode to UTF-8
        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        
        // Calculate how many bytes we can actually write (leave room for null terminator)
        int bytesToWrite = Math.min(bytes.length, maxLength - 1);
        
        // Copy bytes
        MemorySegment.copy(bytes, 0, segment, java.lang.foreign.ValueLayout.JAVA_BYTE, 0, bytesToWrite);
        
        // Add null terminator
        segment.set(java.lang.foreign.ValueLayout.JAVA_BYTE, bytesToWrite, (byte) 0);
        
        return bytesToWrite + 1;
    }
    
    /**
     * Allocates a new memory segment and writes a Java string to it as a null-terminated C string.
     * 
     * <p>The memory is allocated in the provided arena and will be freed when the arena is closed.
     * 
     * @param arena the memory arena for allocation
     * @param str the string to write (null is treated as empty string)
     * @return a memory segment containing the null-terminated string
     */
    public static MemorySegment allocateCString(Arena arena, String str) {
        if (str == null || str.isEmpty()) {
            MemorySegment segment = arena.allocate(1);
            segment.set(java.lang.foreign.ValueLayout.JAVA_BYTE, 0, (byte) 0);
            return segment;
        }
        
        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        MemorySegment segment = arena.allocate(bytes.length + 1);
        MemorySegment.copy(bytes, 0, segment, java.lang.foreign.ValueLayout.JAVA_BYTE, 0, bytes.length);
        segment.set(java.lang.foreign.ValueLayout.JAVA_BYTE, bytes.length, (byte) 0);
        return segment;
    }
    
    /**
     * Validates that a string fits within a specified byte limit when encoded as UTF-8.
     * 
     * @param str the string to validate
     * @param maxBytesIncludingNull the maximum number of bytes (including null terminator)
     * @return true if the string fits, false otherwise
     */
    public static boolean fitsInBuffer(String str, int maxBytesIncludingNull) {
        if (str == null || str.isEmpty()) {
            return true; // Empty string always fits (just null terminator)
        }
        
        if (maxBytesIncludingNull < 1) {
            return false;
        }
        
        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        return bytes.length < maxBytesIncludingNull; // Strictly less than (need room for null)
    }
}
