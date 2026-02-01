package com.astraeus.native_api;

import com.astraeus.native_api.model.PickResult;

import java.lang.foreign.MemorySegment;

/**
 * View wrapper for PickResult struct from native engine.
 * Provides safe access to picking data returned from the C API.
 * 
 * <p>This class is immutable and represents a snapshot of a pick operation result.</p>
 * 
 * @deprecated Use {@link PickResult} in the {@link com.astraeus.native_api.model} package instead.
 *             This class is maintained for backward compatibility.
 */
@Deprecated
public class PickingView extends PickResult {
    
    /**
     * Create a PickingView from a native PickResult struct.
     * 
     * @param structSegment Memory segment containing the PickResult struct
     * @deprecated Use {@link PickResult} instead
     */
    @Deprecated
    public PickingView(MemorySegment structSegment) {
        super(structSegment);
    }
}
