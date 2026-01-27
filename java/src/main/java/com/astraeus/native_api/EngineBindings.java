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
    
    private static final FunctionDescriptor CREATE_ENTITY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,     // return: entity_id
        ValueLayout.ADDRESS       // param: EngineHandle
    );
    
    private static final FunctionDescriptor DESTROY_ENTITY_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT      // param: entity_id
    );
    
    // Method handles
    public static final MethodHandle CREATE_ENGINE;
    public static final MethodHandle DESTROY_ENGINE;
    public static final MethodHandle IS_VALID;
    public static final MethodHandle BEGIN_FRAME;
    public static final MethodHandle END_FRAME;
    public static final MethodHandle RESIZE_VIEWPORT;
    public static final MethodHandle CREATE_ENTITY;
    public static final MethodHandle DESTROY_ENTITY;
    
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
            
        } catch (Exception e) {
            throw new ExceptionInInitializerError("Failed to load Astraeus native library: " + e.getMessage());
        }
    }
    
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
}
