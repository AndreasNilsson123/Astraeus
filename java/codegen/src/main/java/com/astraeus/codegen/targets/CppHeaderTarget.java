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
 * Generation target for C++ POD struct header.
 * Generates EngineABI_Structs.h with struct definitions and static assertions.
 */
public final class CppHeaderTarget implements GenerationTarget {
    
    @Override
    public String getName() {
        return "cpp-header";
    }
    
    @Override
    public String getDescription() {
        return "C++ POD struct definitions for ABI";
    }
    
    @Override
    public void generate(SchemaModel schema, GenerationContext context) throws Exception {
        // Determine output path
        Path engineGeneratedDir = context.getRepoLayout().getEngineGeneratedDir();
        Path outputFile = PathResolver.resolveCppIncludePath(engineGeneratedDir, "EngineABI_Structs.h");
        
        System.out.println("Generating C++ header: " + outputFile);
        
        // Create output directory
        Files.createDirectories(engineGeneratedDir);
        
        // Generate content
        StringBuilder sb = new StringBuilder();
        
        sb.append("// ============================================================================\n");
        sb.append("// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n");
        sb.append("// Generated from: abi_structs_schema.yaml\n");
        sb.append("// Schema version: ").append(schema.getVersion()).append("\n");
        sb.append("// Generated at: ").append(schema.getGenerationTimestamp()).append("\n");
        sb.append("// Schema hash: ").append(schema.getSchemaHash()).append("\n");
        sb.append("// ============================================================================\n\n");
        
        sb.append("#ifndef ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n");
        sb.append("#define ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n\n");
        
        sb.append("#include <stdint.h>\n");
        sb.append("#include <stdbool.h>\n");
        sb.append("#include <stddef.h>\n\n");
        
        sb.append("#ifdef __cplusplus\n");
        sb.append("extern \"C\" {\n");
        sb.append("#endif\n\n");
        
        sb.append("#define ASTRAEUS_ABI_SCHEMA_VERSION \"").append(schema.getVersion()).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_GENERATION_TIMESTAMP \"").append(schema.getGenerationTimestamp()).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_SCHEMA_HASH \"").append(schema.getSchemaHash()).append("\"\n\n");
        
        for (StructDef struct : schema.getStructs()) {
            generateStruct(sb, struct);
        }
        
        sb.append("#ifdef __cplusplus\n");
        sb.append("}\n");
        sb.append("#endif\n\n");
        
        sb.append("#endif // ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n");
        
        // Write to file
        Files.writeString(outputFile, sb.toString());
        
        System.out.println("✓ Generated C++ header");
    }
    
    private void generateStruct(StringBuilder sb, StructDef struct) {
        sb.append("// ").append(struct.getDescription() == null ? "" : struct.getDescription()).append("\n");
        sb.append("typedef struct ").append(struct.getName()).append(" {\n");
        
        for (FieldDef field : struct.getFields()) {
            String cType = mapToCType(field.getType());
            sb.append("    ").append(cType).append(" ").append(field.getName());
            if (field.getArraySize() > 0) {
                sb.append("[").append(field.getArraySize()).append("]");
            }
            sb.append(";");
            if (field.getDescription() != null && !field.getDescription().isEmpty()) {
                sb.append("  // ").append(field.getDescription());
            }
            sb.append("\n");
        }
        
        sb.append("} ").append(struct.getName()).append(";\n\n");
        
        int align = calculateAlignment(struct);
        
        sb.append("#ifdef __cplusplus\n");
        sb.append("static_assert(alignof(").append(struct.getName()).append(") == ").append(align)
                .append(", \"").append(struct.getName()).append(" alignment mismatch\");\n");
        sb.append("static_assert(sizeof(").append(struct.getName()).append(") % ").append(align)
                .append(" == 0, \"").append(struct.getName()).append(" size must be multiple of alignment\");\n");
        
        // Emit offsetof checks for named (non-padding) fields to catch accidental packing changes.
        for (FieldDef field : struct.getFields()) {
            if (field.isPadding()) continue;
            sb.append("static_assert(offsetof(").append(struct.getName()).append(", ").append(field.getName())
                    .append(") % ").append(getTypeAlignment(field.getType()))
                    .append(" == 0, \"").append(struct.getName()).append(".").append(field.getName())
                    .append(" offset not aligned\");\n");
        }
        sb.append("#endif\n\n");
    }
    
    private static String mapToCType(String type) {
        return switch (type) {
            case "uint8" -> "uint8_t";
            case "uint16" -> "uint16_t";
            case "uint32" -> "uint32_t";
            case "uint64" -> "uint64_t";
            case "int8" -> "int8_t";
            case "int16" -> "int16_t";
            case "int32" -> "int32_t";
            case "int64" -> "int64_t";
            case "float32" -> "float";
            case "float64" -> "double";
            case "bool" -> "bool";
            case "pointer" -> "void*";
            case "char_pointer" -> "const char*";
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
}
