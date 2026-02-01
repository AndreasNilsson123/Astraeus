package com.astraeus.codegen.paths;

import java.nio.file.Path;

/**
 * Utilities for resolving paths in a cross-platform manner.
 * All operations use {@link Path} to ensure Windows/Linux compatibility.
 */
public final class PathResolver {
    
    private PathResolver() {}
    
    /**
     * Resolve a Java package name to a directory path.
     * Example: "com.astraeus.generated" -> "com/astraeus/generated"
     * 
     * @param base Base directory
     * @param packageName Java package name (e.g., "com.astraeus.generated")
     * @return Path to package directory
     */
    public static Path resolveJavaPackageDir(Path base, String packageName) {
        Path result = base;
        for (String part : packageName.split("\\.")) {
            result = result.resolve(part);
        }
        return result;
    }
    
    /**
     * Resolve a C++ include path.
     * Example: ("engine/generated", "EngineABI_Structs.h") -> "engine/generated/EngineABI_Structs.h"
     * 
     * @param base Base directory
     * @param fileName C++ header file name
     * @return Full path to header file
     */
    public static Path resolveCppIncludePath(Path base, String fileName) {
        return base.resolve(fileName);
    }
    
    /**
     * Convert a Java package name to a path string using forward slashes.
     * Useful for documentation and display purposes.
     * 
     * @param packageName Java package name
     * @return Path string with forward slashes
     */
    public static String packageNameToPath(String packageName) {
        return packageName.replace('.', '/');
    }
}
