package com.astraeus.codegen.framework;

import com.astraeus.codegen.paths.RepoLayout;

import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

/**
 * Context passed to generation targets during code generation.
 * Provides access to repository layout, output directories, and options.
 */
public final class GenerationContext {
    
    private final RepoLayout repoLayout;
    private final Path outputBaseDir;
    private final Map<String, String> options;
    
    private GenerationContext(Builder builder) {
        this.repoLayout = builder.repoLayout;
        this.outputBaseDir = builder.outputBaseDir;
        this.options = Map.copyOf(builder.options);
    }
    
    public RepoLayout getRepoLayout() {
        return repoLayout;
    }
    
    public Path getOutputBaseDir() {
        return outputBaseDir;
    }
    
    public Map<String, String> getOptions() {
        return options;
    }
    
    public String getOption(String key) {
        return options.get(key);
    }
    
    public String getOption(String key, String defaultValue) {
        return options.getOrDefault(key, defaultValue);
    }
    
    public static Builder builder() {
        return new Builder();
    }
    
    public static final class Builder {
        private RepoLayout repoLayout;
        private Path outputBaseDir;
        private final Map<String, String> options = new HashMap<>();
        
        public Builder repoLayout(RepoLayout repoLayout) {
            this.repoLayout = repoLayout;
            return this;
        }
        
        public Builder outputBaseDir(Path outputBaseDir) {
            this.outputBaseDir = outputBaseDir;
            return this;
        }
        
        public Builder option(String key, String value) {
            this.options.put(key, value);
            return this;
        }
        
        public Builder options(Map<String, String> options) {
            this.options.putAll(options);
            return this;
        }
        
        public GenerationContext build() {
            if (repoLayout == null) {
                throw new IllegalStateException("repoLayout is required");
            }
            return new GenerationContext(this);
        }
    }
}
