package com.astraeus.codegen.targets;

import com.astraeus.codegen.framework.GenerationContext;
import com.astraeus.codegen.framework.GenerationTarget;
import com.astraeus.codegen.paths.PathResolver;
import com.astraeus.codegen.schema.SchemaModel;
import com.astraeus.codegen.schema.SchemaModel.StructDef;
import com.astraeus.codegen.schema.SchemaModel.FieldDef;

import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Generation target for Java FFM MemoryLayout definitions.
 * Generates StructLayouts.java with layout and VarHandle definitions.
 */
public final class JavaLayoutsTarget implements GenerationTarget {
    
    @Override
    public String getName() {
        return "java-layouts";
    }
    
    @Override
    public String getDescription() {
        return "Java FFM MemoryLayout definitions for ABI structs";
    }
    
    @Override
    public void generate(SchemaModel schema, GenerationContext context) throws Exception {
        // Determine output path
        Path outputDir = context.getRepoLayout().getFrontendGeneratedDir();
        Path packageDir = PathResolver.resolveJavaPackageDir(outputDir, "com.astraeus.generated");
        Path outputFile = packageDir.resolve("StructLayouts.java");
        
        System.out.println("Generating Java layouts: " + outputFile);
        
        // Create output directory
        Files.createDirectories(packageDir);
        
        // Generate content
        StringBuilder sb = new StringBuilder();
        
        sb.append("// ============================================================================\n");
        sb.append("// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n");
        sb.append("// Generated from: abi_structs_schema.yaml\n");
        sb.append("// Schema version: ").append(schema.getVersion()).append("\n");
        sb.append("// Generated at: ").append(schema.getGenerationTimestamp()).append("\n");
        sb.append("// Schema hash: ").append(schema.getSchemaHash()).append("\n");
        sb.append("// ============================================================================\n\n");
        
        sb.append("package com.astraeus.generated;\n\n");
        
        sb.append("import java.lang.foreign.*;\n");
        sb.append("import java.lang.invoke.VarHandle;\n\n");
        
        sb.append("/**\n");
        sb.append(" * Auto-generated FFM MemoryLayout definitions for ABI structs.\n");
        sb.append(" *\n");
        sb.append(" * <p><b>IMPORTANT ABI NOTES</b>\n");
        sb.append(" * <ul>\n");
        sb.append(" *   <li>Padding fields in the schema are emitted as {@link MemoryLayout#paddingLayout(long)} (anonymous padding),\n");
        sb.append(" *       NOT as named byte arrays. This matches C ABI intent.</li>\n");
        sb.append(" *   <li>Each struct layout is given an explicit byte alignment matching the largest member alignment.</li>\n");
        sb.append(" * </ul>\n");
        sb.append(" */\n");
        sb.append("public final class StructLayouts {\n\n");
        
        sb.append("    private StructLayouts() {}\n\n");
        
        sb.append("    public static final String SCHEMA_VERSION = \"").append(schema.getVersion()).append("\";\n");
        sb.append("    public static final String GENERATION_TIMESTAMP = \"").append(schema.getGenerationTimestamp()).append("\";\n");
        sb.append("    public static final String SCHEMA_HASH = \"").append(schema.getSchemaHash()).append("\";\n\n");
        
        for (StructDef struct : schema.getStructs()) {
            generateStructLayout(sb, struct);
        }
        
        sb.append("}\n");
        
        // Write to file
        Files.writeString(outputFile, sb.toString());
        
        System.out.println("✓ Generated Java layouts");
    }
    
    private void generateStructLayout(StringBuilder sb, StructDef struct) {
        String structName = struct.getName();
        String layoutConst = toUpperSnake(structName) + "_LAYOUT";
        int structAlign = calculateAlignment(struct);
        
        sb.append("    /** ").append(escapeJavadoc(struct.getDescription())).append(" */\n");
        sb.append("    public static final StructLayout ").append(layoutConst).append(" = MemoryLayout.structLayout(\n");
        
        // fields
        for (int i = 0; i < struct.getFields().size(); i++) {
            FieldDef field = struct.getFields().get(i);
            
            sb.append("        ");
            
            if (field.isPadding()) {
                // Emit anonymous padding (NOT sequenceLayout) to avoid treating it as a real field.
                long bytes = (field.getArraySize() > 0) ? field.getArraySize() : 0;
                if (bytes <= 0) {
                    // If user wrote `_padding` without array_size, do nothing (or throw).
                    bytes = 1;
                }
                sb.append("MemoryLayout.paddingLayout(").append(bytes).append(")");
            } else if (field.getArraySize() > 0) {
                // Non-padding arrays: real sequences.
                sb.append("MemoryLayout.sequenceLayout(").append(field.getArraySize()).append(", ")
                        .append(mapToJavaLayout(field.getType())).append(")")
                        .append(".withName(\"").append(field.getName()).append("\")");
            } else {
                sb.append(mapToJavaLayout(field.getType())).append(".withName(\"").append(field.getName()).append("\")");
            }
            
            if (i < struct.getFields().size() - 1) sb.append(",");
            sb.append("\n");
        }
        
        sb.append("    ).withByteAlignment(").append(structAlign).append(");\n\n");
        
        // VarHandles (skip padding)
        sb.append("    // VarHandles for ").append(structName).append("\n");
        for (FieldDef field : struct.getFields()) {
            if (field.isPadding()) continue;
            
            String handleName = toUpperSnake(structName) + "_" + toUpperSnake(field.getName());
            
            sb.append("    public static final VarHandle ").append(handleName).append(" = ")
                    .append(layoutConst).append(".varHandle(\n")
                    .append("        MemoryLayout.PathElement.groupElement(\"").append(field.getName()).append("\")");
            
            if (field.getArraySize() > 0) {
                // Provide element handle access for sequence element (caller can use index)
                sb.append(",\n        MemoryLayout.PathElement.sequenceElement()");
            }
            sb.append("\n    );\n");
        }
        sb.append("\n");
    }
    
    private static String escapeJavadoc(String s) {
        if (s == null) return "";
        return s.replace("*/", "*\\/");
    }
    
    private static String mapToJavaLayout(String type) {
        return switch (type) {
            case "uint8", "int8" -> "ValueLayout.JAVA_BYTE";
            case "uint16", "int16" -> "ValueLayout.JAVA_SHORT";
            case "uint32", "int32" -> "ValueLayout.JAVA_INT";
            case "uint64", "int64" -> "ValueLayout.JAVA_LONG";
            case "float32" -> "ValueLayout.JAVA_FLOAT";
            case "float64" -> "ValueLayout.JAVA_DOUBLE";
            case "bool" -> "ValueLayout.JAVA_BOOLEAN";   // 1 byte
            case "pointer", "char_pointer" -> "ValueLayout.ADDRESS";
            default -> throw new IllegalArgumentException("Unknown type: " + type);
        };
    }
    
    private static int calculateAlignment(StructDef struct) {
        int maxAlign = 1;
        for (FieldDef field : struct.getFields()) {
            if (field.isPadding()) continue;
            int align = getTypeAlignment(field.getType());
            if (align > maxAlign) maxAlign = align;
        }
        return maxAlign;
    }
    
    private static int getTypeAlignment(String type) {
        // x64 ABI assumptions (Windows/Linux): pointer/double=8.
        return switch (type) {
            case "uint8", "int8", "bool" -> 1;
            case "uint16", "int16" -> 2;
            case "uint32", "int32", "float32" -> 4;
            case "uint64", "int64", "float64", "pointer", "char_pointer" -> 8;
            default -> 1;
        };
    }
    
    private static String toUpperSnake(String s) {
        // Convert CamelCase or lower_snake to UPPER_SNAKE
        StringBuilder out = new StringBuilder();
        char prev = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '-') c = '_';
            if (Character.isUpperCase(c) && i > 0 && prev != '_' && Character.isLowerCase(prev)) {
                out.append('_');
            }
            out.append(Character.toUpperCase(c));
            prev = c;
        }
        return out.toString().replace("__", "_");
    }
}
