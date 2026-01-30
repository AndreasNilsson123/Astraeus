package com.astraeus.tools;

import javax.tools.JavaCompiler;
import javax.tools.ToolProvider;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;

/**
 * Portable replacement for regenerate_abi_codegen.sh
 *
 * Usage:
 *   java com.astraeus.tools.ABICodegenRunner [projectRoot]
 *
 * If projectRoot is omitted, current working directory is used.
 */
public final class ABICodegenRunner {

    private static final String CODEGEN_CLASS = "com.astraeus.tools.ABICodeGenerator";

    private static final String CODEGEN_SRC_REL = "java/src/main/java/com/astraeus/tools/ABICodeGenerator.java";

    private static final String SCHEMA_REL = "engine/api/abi_structs_schema.yaml";

    public static void main(String[] args) throws Exception {
        Path projectRoot = args.length > 0 ? Paths.get(args[0]).toAbsolutePath() : Paths.get("").toAbsolutePath();

        System.out.println("================================================");
        System.out.println("Regenerating ABI Struct Code");
        System.out.println("================================================");
        System.out.println("Project root: " + projectRoot);

        Path schemaFile = projectRoot.resolve(SCHEMA_REL);
        Path codegenSrc = projectRoot.resolve(CODEGEN_SRC_REL);

        if (!Files.exists(schemaFile)) {
            fail("Schema file not found: " + schemaFile);
        }
        if (!Files.exists(codegenSrc)) {
            fail("Code generator source not found: " + codegenSrc);
        }

        Path tempOut = Files.createTempDirectory("abi_codegen");
        try {
            compileCodegen(codegenSrc, tempOut);
            runCodegen(tempOut, schemaFile, projectRoot);
        } finally {
            deleteRecursive(tempOut);
        }

        System.out.println();
        System.out.println("================================================");
        System.out.println("✓ Code generation completed");
        System.out.println("================================================");
        System.out.println();
        System.out.println("Generated files:");
        System.out.println("  - engine/api/EngineABI_Structs.gen.h");
        System.out.println("  - java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java");
        System.out.println();
        System.out.println("Run 'verify_abi_codegen' to verify the generated files.");
    }

    private static void compileCodegen(Path source, Path outputDir) throws Exception {
        System.out.println("Compiling code generator…");

        JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        if (compiler == null) {
            fail("No system Java compiler found. Are you running a JDK (not JRE)?");
        }

        int result = compiler.run(null, System.out, System.err, "--source", "17", "--target", "17", "-d", outputDir.toString(), source.toString());

        if (result != 0) {
            fail("Code generator compilation failed");
        }
    }

    private static void runCodegen(Path classDir, Path schema, Path projectRoot) throws Exception {

        System.out.println("Running code generator…");

        ProcessBuilder pb = new ProcessBuilder("java", "-cp", classDir.toString(), CODEGEN_CLASS, schema.toString(), projectRoot.toString());

        pb.inheritIO();

        Process p = pb.start();
        int exit = p.waitFor();

        if (exit != 0) {
            fail("Code generator exited with status " + exit);
        }
    }

    private static void deleteRecursive(Path path) {
        try {
            if (!Files.exists(path)) return;
            Files.walk(path)
                    .sorted((a, b) -> b.compareTo(a))
                    .forEach(p -> {
                        try { Files.deleteIfExists(p); }
                        catch (Exception ignored) {}
                    });
        } catch (Exception ignored) {}
    }

    private static void fail(String msg) {
        System.err.println("ERROR: " + msg);
        System.exit(1);
    }
}

