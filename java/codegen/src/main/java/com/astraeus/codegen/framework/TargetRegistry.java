package com.astraeus.codegen.framework;

import com.astraeus.codegen.targets.CppHeaderTarget;
import com.astraeus.codegen.targets.JavaLayoutsTarget;

import java.util.*;

/**
 * Registry of all available code generation targets.
 * Targets are registered statically for simplicity (not using ServiceLoader).
 */
public final class TargetRegistry {
    
    private static final Map<String, GenerationTarget> TARGETS = new LinkedHashMap<>();
    
    static {
        // Register all available targets
        register(new JavaLayoutsTarget());
        register(new CppHeaderTarget());
    }
    
    private TargetRegistry() {}
    
    /**
     * Register a generation target.
     */
    public static void register(GenerationTarget target) {
        TARGETS.put(target.getName(), target);
    }
    
    /**
     * Get all available targets.
     */
    public static List<GenerationTarget> getAvailableTargets() {
        return new ArrayList<>(TARGETS.values());
    }
    
    /**
     * Get a target by name.
     */
    public static Optional<GenerationTarget> getTarget(String name) {
        return Optional.ofNullable(TARGETS.get(name));
    }
    
    /**
     * Check if a target exists.
     */
    public static boolean hasTarget(String name) {
        return TARGETS.containsKey(name);
    }
}
