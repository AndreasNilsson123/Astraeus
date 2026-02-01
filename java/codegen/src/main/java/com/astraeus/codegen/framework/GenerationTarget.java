package com.astraeus.codegen.framework;

import com.astraeus.codegen.schema.SchemaModel;

/**
 * A code generation target that produces output from a schema model.
 * Implementations generate code for specific languages or purposes (e.g., Java layouts, C++ headers).
 */
public interface GenerationTarget {
    
    /**
     * Get the unique name of this target (e.g., "java-layouts", "cpp-header").
     */
    String getName();
    
    /**
     * Get a human-readable description of this target.
     */
    String getDescription();
    
    /**
     * Generate code for this target.
     * 
     * @param schema The schema model to generate from
     * @param context The generation context with paths and options
     * @throws Exception if generation fails
     */
    void generate(SchemaModel schema, GenerationContext context) throws Exception;
}
