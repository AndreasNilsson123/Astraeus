package com.astraeus.native_api;

import java.lang.foreign.*;
import java.lang.invoke.MethodHandle;

/**
 * Native FFM bindings for Astraeus engine C API.
 * Uses Java 21+ Foreign Function & Memory API (FFM).
 */
public class EngineBindings {
    
    private static final Linker LINKER = Linker.nativeLinker();
    private static final SymbolLookup LIBRARY;
    
    // Pixel format constants (must match EngineAPI.h)
    public static final int PIXEL_FORMAT_RGBA8 = 0;
    public static final int PIXEL_FORMAT_BGRA8 = 1;
    public static final int PIXEL_FORMAT_ARGB8 = 2;
    public static final int PIXEL_FORMAT_R32UI = 3;
    
    /**
     * Memory layout for ReadbackConfig struct.
     * 
     * Note: Manual padding is platform-dependent. This layout assumes x64 Linux/Windows.
     * In production, consider generating layouts from native headers or using jextract.
     */
    public static final StructLayout READBACK_CONFIG_LAYOUT = MemoryLayout.structLayout(
        ValueLayout.JAVA_INT.withName("max_width"),
        ValueLayout.JAVA_INT.withName("max_height"),
        ValueLayout.JAVA_INT.withName("format"),
        ValueLayout.JAVA_BOOLEAN.withName("enable_double_buffer"),
        MemoryLayout.paddingLayout(3)  // Padding for alignment
    );
    
    /**
     * Memory layout for PixelBufferView struct.
     * 
     * IMPORTANT: This layout is manually defined and platform-dependent.
     * Assumes x64 Linux/Windows with standard alignment.
     * 
     * Layout breakdown:
     * - ADDRESS (data): 8 bytes
     * - INT (width): 4 bytes (no padding needed, total now 12)
     * - INT (height): 4 bytes (total now 16, aligned)
     * - INT (stride): 4 bytes (total now 20)
     * - INT (format): 4 bytes (total now 24, aligned)
     * - INT (max_backing_width): 4 bytes (total now 28)
     * - INT (max_backing_height): 4 bytes (total now 32, aligned)
     * - INT (max_backing_size): 4 bytes (total now 36)
     * 
     * C struct alignment typically aligns to largest field (8 bytes for pointer),
     * so no explicit padding needed between INT fields after the ADDRESS field.
     * 
     * For production use, consider:
     * 1. Using jextract tool to generate layouts from C headers automatically
     * 2. Testing on all target platforms to verify layout compatibility
     * 3. Using runtime layout queries if available
     * 4. Documenting supported platforms explicitly
     */
    public static final StructLayout PIXEL_BUFFER_VIEW_LAYOUT = MemoryLayout.structLayout(
        ValueLayout.ADDRESS.withName("data"),
        ValueLayout.JAVA_INT.withName("width"),
        ValueLayout.JAVA_INT.withName("height"),
        ValueLayout.JAVA_INT.withName("stride"),
        ValueLayout.JAVA_INT.withName("format"),
        ValueLayout.JAVA_INT.withName("max_backing_width"),
        ValueLayout.JAVA_INT.withName("max_backing_height"),
        ValueLayout.JAVA_INT.withName("max_backing_size")
    );
    
    /**
     * Memory layout for EngineConfig struct.
     * 
     * Note: Manual padding is platform-dependent. This layout assumes x64 Linux/Windows.
     * In production, consider generating layouts from native headers or using jextract.
     */
    public static final StructLayout ENGINE_CONFIG_LAYOUT = MemoryLayout.structLayout(
        ValueLayout.JAVA_INT.withName("initial_width"),
        ValueLayout.JAVA_INT.withName("initial_height"),
        ValueLayout.JAVA_BOOLEAN.withName("enable_validation"),
        MemoryLayout.paddingLayout(3), // Padding for alignment
        ValueLayout.JAVA_BOOLEAN.withName("enable_debug_output"),
        MemoryLayout.paddingLayout(3), // Padding for alignment
        ValueLayout.ADDRESS.withName("log_file_path")
    );
    
    /**
     * Memory layout for FrameStats struct.
     * 
     * Note: Manual padding is platform-dependent. This layout assumes x64 Linux/Windows.
     * In production, consider generating layouts from native headers or using jextract.
     */
    public static final StructLayout FRAME_STATS_LAYOUT = MemoryLayout.structLayout(
        ValueLayout.JAVA_LONG.withName("frame_number"),
        ValueLayout.JAVA_DOUBLE.withName("delta_time_ms"),
        ValueLayout.JAVA_DOUBLE.withName("render_time_ms"),
        ValueLayout.JAVA_INT.withName("draw_calls"),
        ValueLayout.JAVA_INT.withName("triangle_count"),
        ValueLayout.JAVA_INT.withName("entity_count"),
        MemoryLayout.paddingLayout(4) // Padding for alignment
    );
    
    /**
     * Memory layout for PickResult struct.
     * 
     * Layout breakdown (C struct):
     * - UINT32 (entity_id): 4 bytes
     * - FLOAT (depth): 4 bytes (total 8)
     * - FLOAT (world_x): 4 bytes (total 12)
     * - FLOAT (world_y): 4 bytes (total 16)
     * - FLOAT (world_z): 4 bytes (total 20)
     * - BOOL (hit): 1 byte
     * - PADDING: 3 bytes (alignment to 4-byte boundary, total 24)
     * 
     * Note: Manual padding is platform-dependent. This layout assumes x64 Linux/Windows.
     * In production, consider generating layouts from native headers or using jextract.
     */
    public static final StructLayout PICK_RESULT_LAYOUT = MemoryLayout.structLayout(
        ValueLayout.JAVA_INT.withName("entity_id"),
        ValueLayout.JAVA_FLOAT.withName("depth"),
        ValueLayout.JAVA_FLOAT.withName("world_x"),
        ValueLayout.JAVA_FLOAT.withName("world_y"),
        ValueLayout.JAVA_FLOAT.withName("world_z"),
        ValueLayout.JAVA_BOOLEAN.withName("hit"),
        MemoryLayout.paddingLayout(3) // Padding for alignment
    );
    
    // Function descriptors (C ABI signatures)
    private static final FunctionDescriptor CREATE_ENGINE_DESC = FunctionDescriptor.of(
        ValueLayout.ADDRESS,  // return: EngineHandle*
        ValueLayout.ADDRESS   // param: EngineConfig*
    );
    
    private static final FunctionDescriptor DESTROY_ENGINE_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS   // param: EngineHandle
    );
    
    private static final FunctionDescriptor IS_VALID_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_BOOLEAN,  // return: bool (C99 bool is compatible with Java boolean)
        ValueLayout.ADDRESS         // param: EngineHandle
    );
    
    private static final FunctionDescriptor BEGIN_FRAME_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,   // param: EngineHandle
        ValueLayout.JAVA_DOUBLE // param: delta_time
    );
    
    private static final FunctionDescriptor END_FRAME_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS   // param: EngineHandle
    );
    
    private static final FunctionDescriptor RESIZE_VIEWPORT_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: width
        ValueLayout.JAVA_INT      // param: height
    );
    
    private static final FunctionDescriptor CONFIGURE_READBACK_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_BOOLEAN, // return: bool
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.ADDRESS,      // param: ReadbackConfig* (color)
        ValueLayout.ADDRESS       // param: ReadbackConfig* (id)
    );
    
    private static final FunctionDescriptor GET_COLOR_BUFFER_DESC = FunctionDescriptor.of(
        PIXEL_BUFFER_VIEW_LAYOUT,  // return: PixelBufferView (struct by value)
        ValueLayout.ADDRESS        // param: EngineHandle
    );
    
    private static final FunctionDescriptor GET_ID_BUFFER_DESC = FunctionDescriptor.of(
        PIXEL_BUFFER_VIEW_LAYOUT,  // return: PixelBufferView (struct by value)
        ValueLayout.ADDRESS        // param: EngineHandle
    );
    
    private static final FunctionDescriptor CREATE_ENTITY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,     // return: entity_id
        ValueLayout.ADDRESS       // param: EngineHandle
    );
    
    private static final FunctionDescriptor DESTROY_ENTITY_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT      // param: entity_id
    );
    
    private static final FunctionDescriptor PICK_DESC = FunctionDescriptor.of(
        PICK_RESULT_LAYOUT,       // return: PickResult (struct by value)
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: screen_x
        ValueLayout.JAVA_INT      // param: screen_y
    );
    
    private static final FunctionDescriptor GET_FRAME_STATS_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.ADDRESS       // param: FrameStats* (out)
    );
    
    // Method handles
    public static final MethodHandle CREATE_ENGINE;
    public static final MethodHandle DESTROY_ENGINE;
    public static final MethodHandle IS_VALID;
    public static final MethodHandle BEGIN_FRAME;
    public static final MethodHandle END_FRAME;
    public static final MethodHandle RESIZE_VIEWPORT;
    public static final MethodHandle CONFIGURE_READBACK;
    public static final MethodHandle GET_COLOR_BUFFER;
    public static final MethodHandle GET_ID_BUFFER;
    public static final MethodHandle CREATE_ENTITY;
    public static final MethodHandle DESTROY_ENTITY;
    public static final MethodHandle PICK;
    public static final MethodHandle GET_FRAME_STATS;
    
    static {
        try {
            // Load native library
            System.loadLibrary("astraeus");
            LIBRARY = SymbolLookup.loaderLookup();
            
            // Link functions
            CREATE_ENGINE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_create_engine").orElseThrow(),
                CREATE_ENGINE_DESC
            );
            
            DESTROY_ENGINE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_destroy_engine").orElseThrow(),
                DESTROY_ENGINE_DESC
            );
            
            IS_VALID = LINKER.downcallHandle(
                LIBRARY.find("astraeus_is_valid").orElseThrow(),
                IS_VALID_DESC
            );
            
            BEGIN_FRAME = LINKER.downcallHandle(
                LIBRARY.find("astraeus_begin_frame").orElseThrow(),
                BEGIN_FRAME_DESC
            );
            
            END_FRAME = LINKER.downcallHandle(
                LIBRARY.find("astraeus_end_frame").orElseThrow(),
                END_FRAME_DESC
            );
            
            RESIZE_VIEWPORT = LINKER.downcallHandle(
                LIBRARY.find("astraeus_resize_viewport").orElseThrow(),
                RESIZE_VIEWPORT_DESC
            );
            
            CREATE_ENTITY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_create_entity").orElseThrow(),
                CREATE_ENTITY_DESC
            );
            
            DESTROY_ENTITY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_destroy_entity").orElseThrow(),
                DESTROY_ENTITY_DESC
            );
            
            CONFIGURE_READBACK = LINKER.downcallHandle(
                LIBRARY.find("astraeus_configure_readback").orElseThrow(),
                CONFIGURE_READBACK_DESC
            );
            
            GET_COLOR_BUFFER = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_color_buffer").orElseThrow(),
                GET_COLOR_BUFFER_DESC
            );
            
            GET_ID_BUFFER = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_id_buffer").orElseThrow(),
                GET_ID_BUFFER_DESC
            );
            
            PICK = LINKER.downcallHandle(
                LIBRARY.find("astraeus_pick").orElseThrow(),
                PICK_DESC
            );
            
            GET_FRAME_STATS = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_frame_stats").orElseThrow(),
                GET_FRAME_STATS_DESC
            );
            
        } catch (Exception e) {
            throw new ExceptionInInitializerError("Failed to load Astraeus native library: " + e.getMessage());
        }
    }
}
