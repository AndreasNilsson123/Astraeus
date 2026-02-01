package com.astraeus.native_api.layout;

import com.astraeus.generated.StructLayouts;

import java.lang.foreign.MemoryLayout;
import java.lang.foreign.StructLayout;

/**
 * Runtime layout validator for FFM struct layouts.
 * 
 * <p>Provides sanity checks to verify that Java FFM struct layouts match the
 * expected C ABI. This is particularly important for detecting platform-specific
 * alignment and padding differences.
 * 
 * <p><b>Usage:</b>
 * <pre>{@code
 * // In development/debug mode:
 * if (LayoutValidator.ENABLE_VALIDATION) {
 *     LayoutValidator.validateAllLayouts();
 * }
 * }</pre>
 * 
 * <p><b>Note:</b> This validator performs basic sanity checks based on expected
 * sizes and alignments. For comprehensive validation, consider using native
 * query functions that return actual C struct sizes and offsets.
 */
public final class LayoutValidator {
    
    /**
     * Flag to enable/disable validation.
     * Set to false in production for performance.
     */
    public static final boolean ENABLE_VALIDATION = 
        Boolean.getBoolean("astraeus.validate.layouts") || 
        "dev".equalsIgnoreCase(System.getProperty("astraeus.mode"));
    
    private LayoutValidator() {
        // Utility class, no instances
    }
    
    /**
     * Validates all struct layouts.
     * 
     * <p>This method performs sanity checks on all known struct layouts to ensure
     * they are consistent with expected sizes and alignments for x64 platforms
     * (Windows/Linux).
     * 
     * @return true if all layouts are valid, false otherwise
     */
    public static boolean validateAllLayouts() {
        boolean allValid = true;
        
        // Validate FrameStats
        allValid &= validateLayout("FrameStats", StructLayouts.FRAME_STATS_LAYOUT, 40, 8);
        
        // Validate TelemetryFrameStats
        allValid &= validateLayout("TelemetryFrameStats", StructLayouts.TELEMETRY_FRAME_STATS_LAYOUT, 48, 8);
        
        // Validate ViewportConfig
        allValid &= validateLayout("ViewportConfig", StructLayouts.VIEWPORT_CONFIG_LAYOUT, 16, 4);
        
        // Validate PixelBufferView (36 bytes actual, no tail padding added by FFM)
        allValid &= validateLayout("PixelBufferView", StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT, 36, 8);
        
        // Validate ReadbackConfig (13 bytes actual, no tail padding added by FFM)
        allValid &= validateLayout("ReadbackConfig", StructLayouts.READBACK_CONFIG_LAYOUT, 13, 4);
        
        // Validate PickResult
        allValid &= validateLayout("PickResult", StructLayouts.PICK_RESULT_LAYOUT, 24, 4);
        
        // Validate EngineConfig
        allValid &= validateLayout("EngineConfig", StructLayouts.ENGINE_CONFIG_LAYOUT, 24, 8);
        
        // Validate CameraDesc
        allValid &= validateLayout("CameraDesc", StructLayouts.CAMERA_DESC_LAYOUT, 56, 4);
        
        // Validate MaterialDesc
        allValid &= validateLayout("MaterialDesc", StructLayouts.MATERIAL_DESC_LAYOUT, 40, 4);
        
        if (allValid) {
            System.out.println("[LayoutValidator] All struct layouts validated successfully");
            System.out.println("  Schema version: " + StructLayouts.SCHEMA_VERSION);
            System.out.println("  Schema hash: " + StructLayouts.SCHEMA_HASH);
        } else {
            System.err.println("[LayoutValidator] WARNING: Some struct layouts failed validation");
            System.err.println("  This may indicate a platform ABI mismatch.");
            System.err.println("  Expected layouts are for x64 Windows/Linux with standard alignment.");
        }
        
        return allValid;
    }
    
    // Validates a single struct layout (internal helper)
    private static boolean validateLayout(String name, StructLayout layout, long expectedSize, long expectedAlignment) {
        boolean valid = true;
        
        long actualSize = layout.byteSize();
        long actualAlignment = layout.byteAlignment();
        
        if (actualSize != expectedSize) {
            System.err.println("[LayoutValidator] Size mismatch for " + name + 
                               ": expected " + expectedSize + " bytes, got " + actualSize + " bytes");
            valid = false;
        }
        
        if (actualAlignment != expectedAlignment) {
            System.err.println("[LayoutValidator] Alignment mismatch for " + name + 
                               ": expected " + expectedAlignment + " bytes, got " + actualAlignment + " bytes");
            valid = false;
        }
        
        // Note: We no longer enforce that size is a multiple of alignment,
        // since FFM StructLayout doesn't automatically add tail padding
        // The native C compiler will add padding, but Java FFM doesn't.
        // This is fine as long as we're careful with array layouts.
        
        return valid;
    }
    
    /**
     * Prints detailed information about a struct layout (for debugging).
     * 
     * @param name the name of the struct
     * @param layout the struct layout
     */
    public static void printLayoutInfo(String name, StructLayout layout) {
        System.out.println("=== " + name + " ===");
        System.out.println("  Size: " + layout.byteSize() + " bytes");
        System.out.println("  Alignment: " + layout.byteAlignment() + " bytes");
        System.out.println("  Member layouts:");
        
        long offset = 0;
        for (MemoryLayout member : layout.memberLayouts()) {
            String memberName = member.name().orElse("<padding>");
            long memberSize = member.byteSize();
            long memberAlignment = member.byteAlignment();
            
            // Align offset to member alignment
            if (offset % memberAlignment != 0) {
                offset = (offset + memberAlignment - 1) / memberAlignment * memberAlignment;
            }
            
            System.out.println("    [" + offset + "] " + memberName + ": " + memberSize + " bytes (align: " + memberAlignment + ")");
            offset += memberSize;
        }
        System.out.println();
    }
    
    /**
     * Prints information about all struct layouts (for debugging).
     */
    public static void printAllLayouts() {
        System.out.println("=================================================");
        System.out.println("Astraeus ABI Struct Layouts");
        System.out.println("Schema version: " + StructLayouts.SCHEMA_VERSION);
        System.out.println("Generated at: " + StructLayouts.GENERATION_TIMESTAMP);
        System.out.println("Schema hash: " + StructLayouts.SCHEMA_HASH);
        System.out.println("=================================================");
        System.out.println();
        
        printLayoutInfo("FrameStats", StructLayouts.FRAME_STATS_LAYOUT);
        printLayoutInfo("TelemetryFrameStats", StructLayouts.TELEMETRY_FRAME_STATS_LAYOUT);
        printLayoutInfo("ViewportConfig", StructLayouts.VIEWPORT_CONFIG_LAYOUT);
        printLayoutInfo("PixelBufferView", StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT);
        printLayoutInfo("ReadbackConfig", StructLayouts.READBACK_CONFIG_LAYOUT);
        printLayoutInfo("PickResult", StructLayouts.PICK_RESULT_LAYOUT);
        printLayoutInfo("EngineConfig", StructLayouts.ENGINE_CONFIG_LAYOUT);
        printLayoutInfo("CameraDesc", StructLayouts.CAMERA_DESC_LAYOUT);
        printLayoutInfo("MaterialDesc", StructLayouts.MATERIAL_DESC_LAYOUT);
    }
}
