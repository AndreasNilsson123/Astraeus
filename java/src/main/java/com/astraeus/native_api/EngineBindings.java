package com.astraeus.native_api;

import com.astraeus.generated.StructLayouts;

import java.lang.foreign.*;
import java.lang.invoke.MethodHandle;

/**
 * Native FFM bindings for Astraeus engine C API.
 * Uses Java 21+ Foreign Function & Memory API (FFM).
 * 
 * NOTE: Struct layouts are now auto-generated in StructLayouts.gen.java
 */
public class EngineBindings {
    
    private static final Linker LINKER = Linker.nativeLinker();
    private static final SymbolLookup LIBRARY;
    
    // Pixel format constants (must match EngineAPI.h)
    public static final int PIXEL_FORMAT_RGBA8 = 0;
    public static final int PIXEL_FORMAT_BGRA8 = 1;
    public static final int PIXEL_FORMAT_ARGB8 = 2;
    public static final int PIXEL_FORMAT_R32UI = 3;
    
    // Use generated struct layouts from StructLayouts.gen.java
    public static final StructLayout READBACK_CONFIG_LAYOUT = StructLayouts.READBACK_CONFIG_LAYOUT;
    public static final StructLayout PIXEL_BUFFER_VIEW_LAYOUT = StructLayouts.PIXEL_BUFFER_VIEW_LAYOUT;
    public static final StructLayout ENGINE_CONFIG_LAYOUT = StructLayouts.ENGINE_CONFIG_LAYOUT;
    public static final StructLayout FRAME_STATS_LAYOUT = StructLayouts.FRAME_STATS_LAYOUT;
    public static final StructLayout PICK_RESULT_LAYOUT = StructLayouts.PICK_RESULT_LAYOUT;
    public static final StructLayout TELEMETRY_FRAME_STATS_LAYOUT = StructLayouts.TELEMETRY_FRAME_STATS_LAYOUT;
    public static final StructLayout CAMERA_DESC_LAYOUT = StructLayouts.CAMERA_DESC_LAYOUT;
    public static final StructLayout MATERIAL_DESC_LAYOUT = StructLayouts.MATERIAL_DESC_LAYOUT;
    public static final StructLayout VIEWPORT_CONFIG_LAYOUT = StructLayouts.VIEWPORT_CONFIG_LAYOUT;
    
    // Result code constants (must match EngineAPI.h)
    public static final int ASTRAEUS_SUCCESS = 0;
    public static final int ASTRAEUS_ERROR_INVALID_HANDLE = 1;
    public static final int ASTRAEUS_ERROR_INVALID_PARAMETER = 2;
    public static final int ASTRAEUS_ERROR_OUT_OF_MEMORY = 3;
    public static final int ASTRAEUS_ERROR_NOT_INITIALIZED = 4;
    public static final int ASTRAEUS_ERROR_UNKNOWN = 255;
    
    // Camera mode constants
    public static final int CAMERA_MODE_ORBIT = 0;
    public static final int CAMERA_MODE_FLY = 1;
    public static final int CAMERA_MODE_PAN = 2;
    
    // Alpha mode constants
    public static final int ALPHA_MODE_OPAQUE = 0;
    public static final int ALPHA_MODE_BLEND = 1;
    public static final int ALPHA_MODE_MASK = 2;
    
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
    
    private static final FunctionDescriptor BEGIN_FRAME_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,    // return: AstraeusResult (uint32_t)
        ValueLayout.ADDRESS,     // param: EngineHandle
        ValueLayout.JAVA_DOUBLE  // param: delta_time
    );
    
    private static final FunctionDescriptor END_FRAME_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,    // return: AstraeusResult (uint32_t)
        ValueLayout.ADDRESS      // param: EngineHandle
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
    
    private static final FunctionDescriptor GET_COLOR_BUFFER_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS        // param: PixelBufferView* (out)
    );
    
    private static final FunctionDescriptor GET_ID_BUFFER_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS        // param: PixelBufferView* (out)
    );
    
    private static final FunctionDescriptor CREATE_ENTITY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,     // return: entity_id
        ValueLayout.ADDRESS       // param: EngineHandle
    );
    
    private static final FunctionDescriptor DESTROY_ENTITY_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT      // param: entity_id
    );
    
    private static final FunctionDescriptor SET_ENTITY_TRANSFORM_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: entity_id
        ValueLayout.JAVA_FLOAT,   // param: pos_x
        ValueLayout.JAVA_FLOAT,   // param: pos_y
        ValueLayout.JAVA_FLOAT,   // param: pos_z
        ValueLayout.JAVA_FLOAT,   // param: rot_x
        ValueLayout.JAVA_FLOAT,   // param: rot_y
        ValueLayout.JAVA_FLOAT,   // param: rot_z
        ValueLayout.JAVA_FLOAT,   // param: scale_x
        ValueLayout.JAVA_FLOAT,   // param: scale_y
        ValueLayout.JAVA_FLOAT    // param: scale_z
    );
    
    private static final FunctionDescriptor SET_ENTITY_RENDERABLE_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: entity_id
        ValueLayout.JAVA_BOOLEAN  // param: visible
    );
    
    private static final FunctionDescriptor SET_ENTITY_COLOR_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: entity_id
        ValueLayout.JAVA_FLOAT,   // param: r
        ValueLayout.JAVA_FLOAT,   // param: g
        ValueLayout.JAVA_FLOAT,   // param: b
        ValueLayout.JAVA_FLOAT    // param: a
    );
    
    private static final FunctionDescriptor SET_ENTITY_TRAIL_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,      // param: EngineHandle
        ValueLayout.JAVA_INT,     // param: entity_id
        ValueLayout.JAVA_INT      // param: max_points
    );

    private static final FunctionDescriptor PICK_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,  // EngineHandle
        ValueLayout.JAVA_INT, // screen_x
        ValueLayout.JAVA_INT, // screen_y
        ValueLayout.ADDRESS   // PickResult* out
    );
    
    private static final FunctionDescriptor ENABLE_TELEMETRY_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.JAVA_BOOLEAN   // param: enabled
    );
    
    private static final FunctionDescriptor IS_TELEMETRY_ENABLED_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_BOOLEAN,  // return: bool
        ValueLayout.ADDRESS        // param: EngineHandle
    );
    
    private static final FunctionDescriptor GET_TELEMETRY_FRAME_STATS_DESC = FunctionDescriptor.ofVoid(
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS        // param: TelemetryFrameStats* (out)
    );
    
    private static final FunctionDescriptor GET_TELEMETRY_HISTORY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: uint32_t (frames written)
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS,       // param: TelemetryFrameStats* buffer (out)
        ValueLayout.JAVA_INT       // param: max_frames
    );
    
    private static final FunctionDescriptor GET_PASS_COUNT_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: uint32_t
        ValueLayout.ADDRESS        // param: EngineHandle
    );
    
    private static final FunctionDescriptor GET_PASS_TIMING_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_BOOLEAN,  // return: bool
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.JAVA_INT,      // param: pass_index
        ValueLayout.ADDRESS,       // param: char* name_buffer (out)
        ValueLayout.JAVA_INT,      // param: name_buffer_size
        ValueLayout.ADDRESS        // param: double* time_ms (out)
    );
    
    // API versioning
    private static final FunctionDescriptor API_VERSION_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT       // return: uint32_t (version)
    );
    
    private static final FunctionDescriptor LAST_ERROR_DESC = FunctionDescriptor.of(
        ValueLayout.ADDRESS,       // return: const char*
        ValueLayout.ADDRESS        // param: EngineHandle
    );
    
    // Viewport API
    private static final FunctionDescriptor VIEWPORT_CREATE_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS,       // param: ViewportConfig*
        ValueLayout.ADDRESS        // param: ViewportHandle* (out)
    );
    
    private static final FunctionDescriptor VIEWPORT_DESTROY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS        // param: ViewportHandle
    );
    
    private static final FunctionDescriptor VIEWPORT_RESIZE_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: ViewportHandle
        ValueLayout.JAVA_INT,      // param: width
        ValueLayout.JAVA_INT       // param: height
    );
    
    private static final FunctionDescriptor VIEWPORT_GET_COLOR_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: ViewportHandle
        ValueLayout.ADDRESS        // param: PixelBufferView* (out)
    );
    
    private static final FunctionDescriptor VIEWPORT_GET_IDBUFFER_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: ViewportHandle
        ValueLayout.ADDRESS        // param: PixelBufferView* (out)
    );
    
    // Camera API
    private static final FunctionDescriptor CAMERA_GET_ACTIVE_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: ViewportHandle
        ValueLayout.ADDRESS        // param: CameraHandle* (out)
    );
    
    private static final FunctionDescriptor CAMERA_GET_DESC_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: CameraHandle
        ValueLayout.ADDRESS        // param: CameraDesc* (out)
    );
    
    private static final FunctionDescriptor CAMERA_SET_DESC_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: CameraHandle
        ValueLayout.ADDRESS        // param: const CameraDesc*
    );
    
    // Materials API
    private static final FunctionDescriptor MATERIAL_CREATE_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.ADDRESS,       // param: MaterialDesc*
        ValueLayout.ADDRESS        // param: MaterialHandle* (out)
    );
    
    private static final FunctionDescriptor MATERIAL_UPDATE_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: MaterialHandle
        ValueLayout.ADDRESS        // param: const MaterialDesc*
    );
    
    private static final FunctionDescriptor MATERIAL_DESTROY_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS        // param: MaterialHandle
    );
    
    private static final FunctionDescriptor ENTITY_SET_MATERIAL_DESC = FunctionDescriptor.of(
        ValueLayout.JAVA_INT,      // return: AstraeusResult
        ValueLayout.ADDRESS,       // param: EngineHandle
        ValueLayout.JAVA_INT,      // param: entity_id
        ValueLayout.ADDRESS        // param: MaterialHandle
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
    public static final MethodHandle SET_ENTITY_TRANSFORM;
    public static final MethodHandle SET_ENTITY_RENDERABLE;
    public static final MethodHandle SET_ENTITY_COLOR;
    public static final MethodHandle SET_ENTITY_TRAIL;
    public static final MethodHandle PICK;
    public static final MethodHandle ENABLE_TELEMETRY;
    public static final MethodHandle IS_TELEMETRY_ENABLED;
    public static final MethodHandle GET_TELEMETRY_FRAME_STATS;
    public static final MethodHandle GET_TELEMETRY_HISTORY;
    public static final MethodHandle GET_PASS_COUNT;
    public static final MethodHandle GET_PASS_TIMING;
    public static final MethodHandle API_VERSION;
    public static final MethodHandle LAST_ERROR;
    public static final MethodHandle VIEWPORT_CREATE;
    public static final MethodHandle VIEWPORT_DESTROY;
    public static final MethodHandle VIEWPORT_RESIZE;
    public static final MethodHandle VIEWPORT_GET_COLOR;
    public static final MethodHandle VIEWPORT_GET_IDBUFFER;
    public static final MethodHandle CAMERA_GET_ACTIVE;
    public static final MethodHandle CAMERA_GET_DESC;
    public static final MethodHandle CAMERA_SET_DESC;
    public static final MethodHandle MATERIAL_CREATE;
    public static final MethodHandle MATERIAL_UPDATE;
    public static final MethodHandle MATERIAL_DESTROY;
    public static final MethodHandle ENTITY_SET_MATERIAL;
    
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
            
            SET_ENTITY_TRANSFORM = LINKER.downcallHandle(
                LIBRARY.find("astraeus_set_entity_transform").orElseThrow(),
                SET_ENTITY_TRANSFORM_DESC
            );
            
            SET_ENTITY_RENDERABLE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_set_entity_renderable").orElseThrow(),
                SET_ENTITY_RENDERABLE_DESC
            );
            
            SET_ENTITY_COLOR = LINKER.downcallHandle(
                LIBRARY.find("astraeus_set_entity_color").orElseThrow(),
                SET_ENTITY_COLOR_DESC
            );
            
            SET_ENTITY_TRAIL = LINKER.downcallHandle(
                LIBRARY.find("astraeus_set_entity_trail").orElseThrow(),
                SET_ENTITY_TRAIL_DESC
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
            
            ENABLE_TELEMETRY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_enable_telemetry").orElseThrow(),
                ENABLE_TELEMETRY_DESC
            );
            
            IS_TELEMETRY_ENABLED = LINKER.downcallHandle(
                LIBRARY.find("astraeus_is_telemetry_enabled").orElseThrow(),
                IS_TELEMETRY_ENABLED_DESC
            );
            
            GET_TELEMETRY_FRAME_STATS = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_telemetry_frame_stats").orElseThrow(),
                GET_TELEMETRY_FRAME_STATS_DESC
            );
            
            GET_TELEMETRY_HISTORY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_telemetry_history").orElseThrow(),
                GET_TELEMETRY_HISTORY_DESC
            );
            
            GET_PASS_COUNT = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_pass_count").orElseThrow(),
                GET_PASS_COUNT_DESC
            );
            
            GET_PASS_TIMING = LINKER.downcallHandle(
                LIBRARY.find("astraeus_get_pass_timing").orElseThrow(),
                GET_PASS_TIMING_DESC
            );
            
            API_VERSION = LINKER.downcallHandle(
                LIBRARY.find("astraeus_api_version").orElseThrow(),
                API_VERSION_DESC
            );
            
            LAST_ERROR = LINKER.downcallHandle(
                LIBRARY.find("astraeus_last_error").orElseThrow(),
                LAST_ERROR_DESC
            );
            
            VIEWPORT_CREATE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_viewport_create").orElseThrow(),
                VIEWPORT_CREATE_DESC
            );
            
            VIEWPORT_DESTROY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_viewport_destroy").orElseThrow(),
                VIEWPORT_DESTROY_DESC
            );
            
            VIEWPORT_RESIZE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_viewport_resize").orElseThrow(),
                VIEWPORT_RESIZE_DESC
            );
            
            VIEWPORT_GET_COLOR = LINKER.downcallHandle(
                LIBRARY.find("astraeus_viewport_get_color").orElseThrow(),
                VIEWPORT_GET_COLOR_DESC
            );
            
            VIEWPORT_GET_IDBUFFER = LINKER.downcallHandle(
                LIBRARY.find("astraeus_viewport_get_idbuffer").orElseThrow(),
                VIEWPORT_GET_IDBUFFER_DESC
            );
            
            CAMERA_GET_ACTIVE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_camera_get_active").orElseThrow(),
                CAMERA_GET_ACTIVE_DESC
            );
            
            CAMERA_GET_DESC = LINKER.downcallHandle(
                LIBRARY.find("astraeus_camera_get_desc").orElseThrow(),
                CAMERA_GET_DESC_DESC
            );
            
            CAMERA_SET_DESC = LINKER.downcallHandle(
                LIBRARY.find("astraeus_camera_set_desc").orElseThrow(),
                CAMERA_SET_DESC_DESC
            );
            
            MATERIAL_CREATE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_material_create").orElseThrow(),
                MATERIAL_CREATE_DESC
            );
            
            MATERIAL_UPDATE = LINKER.downcallHandle(
                LIBRARY.find("astraeus_material_update").orElseThrow(),
                MATERIAL_UPDATE_DESC
            );
            
            MATERIAL_DESTROY = LINKER.downcallHandle(
                LIBRARY.find("astraeus_material_destroy").orElseThrow(),
                MATERIAL_DESTROY_DESC
            );
            
            ENTITY_SET_MATERIAL = LINKER.downcallHandle(
                LIBRARY.find("astraeus_entity_set_material").orElseThrow(),
                ENTITY_SET_MATERIAL_DESC
            );
            
        } catch (Exception e) {
            throw new ExceptionInInitializerError("Failed to load Astraeus native library: " + e.getMessage());
        }
    }
}
