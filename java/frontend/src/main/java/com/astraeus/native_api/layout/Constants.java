package com.astraeus.native_api.layout;

/**
 * Centralized constants for Astraeus Engine C API.
 * 
 * <p>These constants must match the definitions in EngineAPI.h.
 * Any changes here should be coordinated with the FFM/ABI agent.
 */
public final class Constants {
    
    private Constants() {
        // Utility class, no instances
    }
    
    // =============================================================================
    // Result Codes (must match AstraeusResult enum in EngineAPI.h)
    // =============================================================================
    
    public static final int ASTRAEUS_SUCCESS = 0;
    public static final int ASTRAEUS_ERROR_INVALID_HANDLE = 1;
    public static final int ASTRAEUS_ERROR_INVALID_PARAMETER = 2;
    public static final int ASTRAEUS_ERROR_OUT_OF_MEMORY = 3;
    public static final int ASTRAEUS_ERROR_NOT_INITIALIZED = 4;
    public static final int ASTRAEUS_ERROR_UNKNOWN = 255;
    
    // =============================================================================
    // Pixel Format Constants (must match PixelFormat enum in EngineAPI.h)
    // =============================================================================
    
    public static final int PIXEL_FORMAT_RGBA8 = 0;
    public static final int PIXEL_FORMAT_BGRA8 = 1;
    public static final int PIXEL_FORMAT_ARGB8 = 2;
    public static final int PIXEL_FORMAT_R32UI = 3;
    
    // =============================================================================
    // Camera Mode Constants (must match CameraMode enum in EngineAPI.h)
    // =============================================================================
    
    public static final int CAMERA_MODE_ORBIT = 0;
    public static final int CAMERA_MODE_FLY = 1;
    public static final int CAMERA_MODE_PAN = 2;
    
    // =============================================================================
    // Alpha Mode Constants (must match AlphaMode enum in EngineAPI.h)
    // =============================================================================
    
    public static final int ALPHA_MODE_OPAQUE = 0;
    public static final int ALPHA_MODE_BLEND = 1;
    public static final int ALPHA_MODE_MASK = 2;
    
    // =============================================================================
    // String Buffer Limits
    // =============================================================================
    
    /**
     * Maximum length for pass names (including null terminator).
     */
    public static final int MAX_PASS_NAME_LENGTH = 64;
    
    /**
     * Maximum length for error messages (including null terminator).
     */
    public static final int MAX_ERROR_MESSAGE_LENGTH = 256;
}
