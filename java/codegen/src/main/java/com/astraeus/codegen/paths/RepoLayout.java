package com.astraeus.codegen.paths;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Finds and provides access to repository structure.
 * Auto-detects the repository root from any working directory.
 */
public final class RepoLayout {
    
    private final Path repoRoot;
    
    private RepoLayout(Path repoRoot) {
        this.repoRoot = repoRoot.toAbsolutePath().normalize();
    }
    
    /**
     * Auto-detect repository root from current working directory.
     * Searches upward for markers like .git, engine/api/abi_structs_schema.yaml, etc.
     */
    public static RepoLayout autoDetect() throws IOException {
        Path cwd = Paths.get("").toAbsolutePath().normalize();
        return autoDetectFrom(cwd);
    }
    
    /**
     * Auto-detect repository root starting from a given path.
     */
    public static RepoLayout autoDetectFrom(Path startPath) throws IOException {
        Path current = startPath.toAbsolutePath().normalize();
        
        // Try up to 10 levels up
        for (int i = 0; i < 10; i++) {
            if (looksLikeRepoRoot(current)) {
                System.out.println("Detected repository root: " + current);
                return new RepoLayout(current);
            }
            
            Path parent = current.getParent();
            if (parent == null || parent.equals(current)) {
                break;
            }
            current = parent;
        }
        
        throw new IOException("Could not detect repository root from: " + startPath + 
                              ". Please run from within the repository or specify --repo");
    }
    
    /**
     * Create a layout with an explicit repository root.
     */
    public static RepoLayout fromPath(Path repoRoot) throws IOException {
        Path normalized = repoRoot.toAbsolutePath().normalize();
        if (!looksLikeRepoRoot(normalized)) {
            throw new IOException("Path does not appear to be Astraeus repository root: " + normalized);
        }
        return new RepoLayout(normalized);
    }
    
    private static boolean looksLikeRepoRoot(Path path) {
        // Check for multiple indicators
        boolean hasGit = Files.exists(path.resolve(".git"));
        boolean hasEngine = Files.isDirectory(path.resolve("engine"));
        boolean hasJava = Files.isDirectory(path.resolve("java"));
        boolean hasSchema = Files.exists(path.resolve("engine").resolve("api").resolve("abi_structs_schema.yaml"));
        
        // Must have at least 2 indicators
        int count = 0;
        if (hasGit) count++;
        if (hasEngine) count++;
        if (hasJava) count++;
        if (hasSchema) count++;
        
        return count >= 2;
    }
    
    /**
     * Get the repository root path.
     */
    public Path getRepoRoot() {
        return repoRoot;
    }
    
    /**
     * Get the engine/ directory.
     */
    public Path getEngineDir() {
        return repoRoot.resolve("engine");
    }
    
    /**
     * Get the java/ directory.
     */
    public Path getJavaDir() {
        return repoRoot.resolve("java");
    }
    
    /**
     * Get the java/frontend/ directory.
     */
    public Path getFrontendDir() {
        return getJavaDir().resolve("frontend");
    }
    
    /**
     * Get the java/codegen/ directory.
     */
    public Path getCodegenDir() {
        return getJavaDir().resolve("codegen");
    }
    
    /**
     * Get the default schema file path.
     */
    public Path getDefaultSchemaPath() {
        return getEngineDir().resolve("api").resolve("abi_structs_schema.yaml");
    }
    
    /**
     * Get the engine/generated/ directory (C++ generated output).
     */
    public Path getEngineGeneratedDir() {
        return getEngineDir().resolve("generated");
    }
    
    /**
     * Get the frontend build/generated directory (Java generated output).
     */
    public Path getFrontendGeneratedDir() {
        return getFrontendDir().resolve("build").resolve("generated")
                .resolve("sources").resolve("astraeusAbi").resolve("main");
    }
}
