package com.astraeus.native_api.util;

import com.astraeus.native_api.layout.Constants;

import java.lang.foreign.MemorySegment;

/**
 * Utility class for error checking and return code handling for native API calls.
 * 
 * <p>Provides consistent patterns for:
 * <ul>
 *   <li>Validating return codes from native calls</li>
 *   <li>Checking handle validity</li>
 *   <li>Throwing appropriate exceptions with descriptive messages</li>
 * </ul>
 * 
 * <p>All native API functions that return {@code AstraeusResult} should use these
 * methods to validate the result and provide meaningful error messages.
 */
public final class NativeChecks {
    
    private NativeChecks() {
        // Utility class, no instances
    }
    
    /**
     * Checks if a result code indicates success.
     * 
     * @param result the result code from a native function
     * @return true if the result indicates success, false otherwise
     */
    public static boolean isSuccess(int result) {
        return result == Constants.ASTRAEUS_SUCCESS;
    }
    
    /**
     * Checks if a result code indicates an error.
     * 
     * @param result the result code from a native function
     * @return true if the result indicates an error, false otherwise
     */
    public static boolean isError(int result) {
        return result != Constants.ASTRAEUS_SUCCESS;
    }
    
    /**
     * Throws an exception if the result code indicates an error.
     * 
     * @param result the result code from a native function
     * @param operation a description of the operation that was attempted (for error message)
     * @throws RuntimeException if the result indicates an error
     */
    public static void checkResult(int result, String operation) {
        if (isSuccess(result)) {
            return;
        }
        
        String errorMessage = getErrorMessage(result);
        throw new RuntimeException(operation + " failed: " + errorMessage + " (code: " + result + ")");
    }
    
    /**
     * Throws an exception if the result code indicates an error, with additional context.
     * 
     * @param result the result code from a native function
     * @param operation a description of the operation that was attempted
     * @param context additional context information (e.g., "viewport handle: 0x1234")
     * @throws RuntimeException if the result indicates an error
     */
    public static void checkResult(int result, String operation, String context) {
        if (isSuccess(result)) {
            return;
        }
        
        String errorMessage = getErrorMessage(result);
        throw new RuntimeException(operation + " failed: " + errorMessage + 
                                   " (code: " + result + ") [" + context + "]");
    }
    
    /**
     * Returns a human-readable error message for a given result code.
     * 
     * @param result the result code
     * @return a descriptive error message
     */
    public static String getErrorMessage(int result) {
        return switch (result) {
            case Constants.ASTRAEUS_SUCCESS -> "Success";
            case Constants.ASTRAEUS_ERROR_INVALID_HANDLE -> "Invalid handle";
            case Constants.ASTRAEUS_ERROR_INVALID_PARAMETER -> "Invalid parameter";
            case Constants.ASTRAEUS_ERROR_OUT_OF_MEMORY -> "Out of memory";
            case Constants.ASTRAEUS_ERROR_NOT_INITIALIZED -> "Not initialized";
            case Constants.ASTRAEUS_ERROR_UNKNOWN -> "Unknown error";
            default -> "Unrecognized error code";
        };
    }
    
    /**
     * Checks if a handle (pointer) is valid (non-null).
     * 
     * @param handle the handle to check
     * @return true if the handle is valid, false otherwise
     */
    public static boolean isValidHandle(MemorySegment handle) {
        return handle != null && handle != MemorySegment.NULL;
    }
    
    /**
     * Throws an exception if a handle is null.
     * 
     * @param handle the handle to check
     * @param handleName the name of the handle (for error message)
     * @throws IllegalArgumentException if the handle is null
     */
    public static void requireValidHandle(MemorySegment handle, String handleName) {
        if (!isValidHandle(handle)) {
            throw new IllegalArgumentException(handleName + " is null or invalid");
        }
    }
    
    /**
     * Checks if a return value indicates a boolean true (1) or false (0).
     * 
     * <p>This is useful for native functions that return bool, which is represented
     * as a byte in FFM (0 = false, non-zero = true).
     * 
     * @param value the boolean value from native code
     * @return true if value is non-zero, false otherwise
     */
    public static boolean asBoolean(byte value) {
        return value != 0;
    }
    
    /**
     * Converts a Java boolean to a native bool value.
     * 
     * @param value the Java boolean
     * @return 1 if true, 0 if false
     */
    public static byte fromBoolean(boolean value) {
        return value ? (byte) 1 : (byte) 0;
    }
    
    /**
     * Validates that a parameter is within a valid range.
     * 
     * @param value the value to check
     * @param min the minimum valid value (inclusive)
     * @param max the maximum valid value (inclusive)
     * @param paramName the name of the parameter (for error message)
     * @throws IllegalArgumentException if the value is out of range
     */
    public static void requireInRange(int value, int min, int max, String paramName) {
        if (value < min || value > max) {
            throw new IllegalArgumentException(paramName + " must be in range [" + min + ", " + max + "], got: " + value);
        }
    }
    
    /**
     * Validates that a parameter is non-negative.
     * 
     * @param value the value to check
     * @param paramName the name of the parameter (for error message)
     * @throws IllegalArgumentException if the value is negative
     */
    public static void requireNonNegative(int value, String paramName) {
        if (value < 0) {
            throw new IllegalArgumentException(paramName + " must be non-negative, got: " + value);
        }
    }
    
    /**
     * Validates that a parameter is positive (greater than zero).
     * 
     * @param value the value to check
     * @param paramName the name of the parameter (for error message)
     * @throws IllegalArgumentException if the value is not positive
     */
    public static void requirePositive(int value, String paramName) {
        if (value <= 0) {
            throw new IllegalArgumentException(paramName + " must be positive, got: " + value);
        }
    }
}
