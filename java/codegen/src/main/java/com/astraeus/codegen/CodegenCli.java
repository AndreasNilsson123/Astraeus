package com.astraeus.codegen;

import com.astraeus.codegen.framework.GenerationContext;
import com.astraeus.codegen.framework.GenerationTarget;
import com.astraeus.codegen.framework.TargetRegistry;
import com.astraeus.codegen.paths.RepoLayout;
import com.astraeus.codegen.schema.SchemaLoader;
import com.astraeus.codegen.schema.SchemaModel;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;

/**
 * Command-line interface for the Astraeus ABI code generator.
 * 
 * <p>Usage:
 * <pre>
 *   java com.astraeus.codegen.CodegenCli [options]
 *   
 * Options:
 *   --repo &lt;path&gt;           Repository root (auto-detected if not specified)
 *   --schema &lt;path&gt;         Schema file (default: engine/api/abi_structs_schema.yaml)
 *   --out &lt;dir&gt;             Output base directory (optional)
 *   --targets &lt;list&gt;        Comma-separated target names or "all" (default: all)
 *   --list-targets          List available targets and exit
 *   --help                  Show this help message
 * </pre>
 */
public final class CodegenCli {
    
    public static void main(String[] args) {
        try {
            new CodegenCli().run(args);
        } catch (Exception e) {
            System.err.println("ERROR: " + e.getMessage());
            if (Boolean.getBoolean("codegen.debug")) {
                e.printStackTrace();
            }
            System.exit(1);
        }
    }
    
    private void run(String[] args) throws Exception {
        // Parse arguments
        Map<String, String> options = parseArgs(args);
        
        // Handle special modes
        if (options.containsKey("help")) {
            printHelp();
            return;
        }
        
        if (options.containsKey("list-targets")) {
            listTargets();
            return;
        }
        
        // Auto-detect or load repository layout
        RepoLayout repoLayout;
        if (options.containsKey("repo")) {
            Path repoPath = Paths.get(options.get("repo"));
            repoLayout = RepoLayout.fromPath(repoPath);
        } else {
            repoLayout = RepoLayout.autoDetect();
        }
        
        // Determine schema path
        Path schemaPath;
        if (options.containsKey("schema")) {
            schemaPath = Paths.get(options.get("schema"));
        } else {
            schemaPath = repoLayout.getDefaultSchemaPath();
        }
        
        // Load schema
        SchemaModel schema = SchemaLoader.load(schemaPath);
        
        // Determine targets to run
        List<GenerationTarget> targets = determineTargets(options);
        
        if (targets.isEmpty()) {
            System.err.println("No targets selected. Use --targets or --list-targets.");
            System.exit(1);
        }
        
        // Build generation context
        GenerationContext.Builder contextBuilder = GenerationContext.builder()
                .repoLayout(repoLayout);
        
        if (options.containsKey("out")) {
            contextBuilder.outputBaseDir(Paths.get(options.get("out")));
        }
        
        GenerationContext context = contextBuilder.build();
        
        // Run generation
        System.out.println("========================================");
        System.out.println("Astraeus ABI Code Generator");
        System.out.println("========================================");
        System.out.println("Repository: " + repoLayout.getRepoRoot());
        System.out.println("Schema: " + schemaPath);
        System.out.println("Targets: " + targets.stream().map(GenerationTarget::getName).collect(Collectors.joining(", ")));
        System.out.println("========================================\n");
        
        for (GenerationTarget target : targets) {
            System.out.println("Running target: " + target.getName() + " (" + target.getDescription() + ")");
            target.generate(schema, context);
            System.out.println();
        }
        
        System.out.println("========================================");
        System.out.println("✓ Code generation completed successfully");
        System.out.println("========================================");
        System.out.println("Schema version: " + schema.getVersion());
        System.out.println("Schema hash: " + schema.getSchemaHash());
        System.out.println("Generated at: " + schema.getGenerationTimestamp());
        System.out.println("========================================");
    }
    
    private Map<String, String> parseArgs(String[] args) {
        Map<String, String> options = new LinkedHashMap<>();
        
        for (int i = 0; i < args.length; i++) {
            String arg = args[i];
            
            if (arg.equals("--help") || arg.equals("-h")) {
                options.put("help", "true");
            } else if (arg.equals("--list-targets")) {
                options.put("list-targets", "true");
            } else if (arg.startsWith("--")) {
                String key = arg.substring(2);
                if (i + 1 < args.length && !args[i + 1].startsWith("--")) {
                    options.put(key, args[i + 1]);
                    i++;
                } else {
                    options.put(key, "true");
                }
            }
        }
        
        return options;
    }
    
    private List<GenerationTarget> determineTargets(Map<String, String> options) {
        String targetSpec = options.getOrDefault("targets", "all");
        
        if ("all".equalsIgnoreCase(targetSpec)) {
            return TargetRegistry.getAvailableTargets();
        }
        
        List<GenerationTarget> targets = new ArrayList<>();
        for (String name : targetSpec.split(",")) {
            name = name.trim();
            Optional<GenerationTarget> target = TargetRegistry.getTarget(name);
            if (target.isPresent()) {
                targets.add(target.get());
            } else {
                System.err.println("Warning: Unknown target '" + name + "' (use --list-targets to see available targets)");
            }
        }
        
        return targets;
    }
    
    private void listTargets() {
        System.out.println("Available code generation targets:");
        System.out.println();
        
        for (GenerationTarget target : TargetRegistry.getAvailableTargets()) {
            System.out.printf("  %-20s %s%n", target.getName(), target.getDescription());
        }
        
        System.out.println();
        System.out.println("Use --targets <name1>,<name2> to select specific targets");
        System.out.println("Use --targets all to run all targets (default)");
    }
    
    private void printHelp() {
        System.out.println("Astraeus ABI Code Generator");
        System.out.println();
        System.out.println("Usage:");
        System.out.println("  java com.astraeus.codegen.CodegenCli [options]");
        System.out.println();
        System.out.println("Options:");
        System.out.println("  --repo <path>           Repository root (auto-detected if not specified)");
        System.out.println("  --schema <path>         Schema file (default: engine/api/abi_structs_schema.yaml)");
        System.out.println("  --out <dir>             Output base directory (optional)");
        System.out.println("  --targets <list>        Comma-separated target names or 'all' (default: all)");
        System.out.println("  --list-targets          List available targets and exit");
        System.out.println("  --help, -h              Show this help message");
        System.out.println();
        System.out.println("Examples:");
        System.out.println("  # Generate all targets (auto-detect repo)");
        System.out.println("  java com.astraeus.codegen.CodegenCli");
        System.out.println();
        System.out.println("  # Generate only Java layouts");
        System.out.println("  java com.astraeus.codegen.CodegenCli --targets java-layouts");
        System.out.println();
        System.out.println("  # Specify repo and schema explicitly");
        System.out.println("  java com.astraeus.codegen.CodegenCli --repo /path/to/repo --schema custom_schema.yaml");
        System.out.println();
        System.out.println("  # List available targets");
        System.out.println("  java com.astraeus.codegen.CodegenCli --list-targets");
    }
}
