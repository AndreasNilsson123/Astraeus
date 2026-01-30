package com.astraeus.tools;

import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * ABI Struct Code Generator
 * 
 * Generates C++ POD struct definitions and Java FFM MemoryLayout classes
 * from a single YAML schema to prevent drift between languages.
 * 
 * Usage:
 *   java com.astraeus.tools.ABICodeGenerator <schema.yaml> <output_dir>
 */
public class ABICodeGenerator {
    
    private static final String SCHEMA_VERSION_KEY = "version:";
    private static final String STRUCT_KEY = "  - name:";
    
    private String schemaVersion;
    private String generationTimestamp;
    private String schemaHash;
    private List<StructDef> structs = new ArrayList<>();
    
    public static void main(String[] args) {
        if (args.length < 2) {
            System.err.println("Usage: java com.astraeus.tools.ABICodeGenerator <schema.yaml> <output_dir>");
            System.exit(1);
        }
        
        String schemaPath = args[0];
        String outputDir = args[1];
        
        try {
            ABICodeGenerator generator = new ABICodeGenerator();
            generator.parseSchema(schemaPath);
            generator.generateCppHeader(outputDir);
            generator.generateJavaLayouts(outputDir);
            
            System.out.println("✓ Code generation completed successfully");
            System.out.println("  Schema version: " + generator.schemaVersion);
            System.out.println("  Timestamp: " + generator.generationTimestamp);
            System.out.println("  Schema hash: " + generator.schemaHash);
            
        } catch (Exception e) {
            System.err.println("ERROR: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private void parseSchema(String schemaPath) throws Exception {
        System.out.println("Parsing schema: " + schemaPath);
        
        // Calculate hash of schema file
        byte[] schemaBytes = Files.readAllBytes(Paths.get(schemaPath));
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(schemaBytes);
        schemaHash = bytesToHex(hash).substring(0, 16); // First 16 chars
        
        // Generate timestamp
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        generationTimestamp = sdf.format(new Date());
        
        // Parse YAML manually (simple parsing, no external dependencies)
        List<String> lines = Files.readAllLines(Paths.get(schemaPath));
        
        StructDef currentStruct = null;
        FieldDef currentField = null;
        boolean inStructs = false;
        boolean inFields = false;
        
        for (String line : lines) {
            String trimmed = line.trim();
            
            // Parse schema version
            if (trimmed.startsWith("version:")) {
                schemaVersion = trimmed.substring(8).trim().replace("\"", "");
                continue;
            }
            
            // Check for structs section
            if (trimmed.equals("structs:")) {
                inStructs = true;
                continue;
            }
            
            if (!inStructs) continue;
            
            // Parse struct definition
            if (line.startsWith("  - name:")) {
                if (currentStruct != null) {
                    structs.add(currentStruct);
                }
                currentStruct = new StructDef();
                String name = line.substring(line.indexOf("name:") + 5).trim();
                currentStruct.name = name;
                inFields = false;
                continue;
            }
            
            if (currentStruct == null) continue;
            
            // Parse struct description
            if (line.startsWith("    description:")) {
                String desc = line.substring(line.indexOf("description:") + 12).trim();
                if (desc.startsWith("\"")) {
                    desc = desc.substring(1, desc.length() - 1);
                } else if (desc.equals("|")) {
                    // Multi-line description - just use a placeholder for now
                    desc = "Multi-line description";
                }
                currentStruct.description = desc;
                continue;
            }
            
            // Parse fields section
            if (line.startsWith("    fields:")) {
                inFields = true;
                continue;
            }
            
            if (!inFields) continue;
            
            // Parse field definition
            if (line.startsWith("      - name:")) {
                currentField = new FieldDef();
                String name = line.substring(line.indexOf("name:") + 5).trim();
                currentField.name = name;
                currentStruct.fields.add(currentField);
                continue;
            }
            
            if (currentField == null) continue;
            
            // Parse field properties
            if (line.startsWith("        type:")) {
                String type = line.substring(line.indexOf("type:") + 5).trim();
                currentField.type = type;
            } else if (line.startsWith("        array_size:")) {
                String size = line.substring(line.indexOf("array_size:") + 11).trim();
                currentField.arraySize = Integer.parseInt(size);
            } else if (line.startsWith("        description:")) {
                String desc = line.substring(line.indexOf("description:") + 12).trim();
                if (desc.startsWith("\"")) {
                    desc = desc.substring(1, desc.length() - 1);
                }
                currentField.description = desc;
            }
        }
        
        // Add last struct
        if (currentStruct != null) {
            structs.add(currentStruct);
        }
        
        System.out.println("Parsed " + structs.size() + " struct definitions");
    }
    
    private void generateCppHeader(String outputDir) throws Exception {
        String outputPath = outputDir + "/engine/api/EngineABI_Structs.gen.h";
        System.out.println("Generating C++ header: " + outputPath);
        
        StringBuilder sb = new StringBuilder();
        
        // Header guard and includes
        sb.append("// ============================================================================\n");
        sb.append("// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n");
        sb.append("// Generated from: abi_structs_schema.yaml\n");
        sb.append("// Schema version: ").append(schemaVersion).append("\n");
        sb.append("// Generated at: ").append(generationTimestamp).append("\n");
        sb.append("// Schema hash: ").append(schemaHash).append("\n");
        sb.append("// ============================================================================\n\n");
        
        sb.append("#ifndef ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n");
        sb.append("#define ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n\n");
        
        sb.append("#include <stdint.h>\n");
        sb.append("#include <stdbool.h>\n");
        sb.append("#include <stddef.h>\n\n");
        
        sb.append("#ifdef __cplusplus\n");
        sb.append("extern \"C\" {\n");
        sb.append("#endif\n\n");
        
        sb.append("// Schema version and generation metadata\n");
        sb.append("#define ASTRAEUS_ABI_SCHEMA_VERSION \"").append(schemaVersion).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_GENERATION_TIMESTAMP \"").append(generationTimestamp).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_SCHEMA_HASH \"").append(schemaHash).append("\"\n\n");
        
        // Generate structs
        for (StructDef struct : structs) {
            sb.append("// ").append(struct.description).append("\n");
            sb.append("typedef struct {\n");
            
            for (FieldDef field : struct.fields) {
                String cType = mapToCType(field.type);
                sb.append("    ").append(cType).append(" ").append(field.name);
                if (field.arraySize > 0) {
                    sb.append("[").append(field.arraySize).append("]");
                }
                sb.append(";");
                if (field.description != null && !field.description.isEmpty()) {
                    sb.append("  // ").append(field.description);
                }
                sb.append("\n");
            }
            
            sb.append("} ").append(struct.name).append(";\n\n");
            
            // Add static assertions for struct size and alignment
            sb.append("// Compile-time size and alignment checks for ").append(struct.name).append("\n");
            sb.append("#ifdef __cplusplus\n");
            sb.append("static_assert(sizeof(").append(struct.name).append(") % ").append(calculateAlignment(struct)).append(" == 0,\n");
            sb.append("              \"").append(struct.name).append(" size must be aligned to ").append(calculateAlignment(struct)).append(" bytes\");\n");
            sb.append("#endif\n\n");
        }
        
        sb.append("#ifdef __cplusplus\n");
        sb.append("}\n");
        sb.append("#endif\n\n");
        
        sb.append("#endif // ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n");
        
        // Write file
        Path path = Paths.get(outputPath);
        Files.createDirectories(path.getParent());
        Files.writeString(path, sb.toString());
        
        System.out.println("✓ Generated C++ header");
    }
    
    private void generateJavaLayouts(String outputDir) throws Exception {
        String outputPath = outputDir + "/java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java";
        System.out.println("Generating Java layouts: " + outputPath);
        
        StringBuilder sb = new StringBuilder();
        
        // Package and imports
        sb.append("// ============================================================================\n");
        sb.append("// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n");
        sb.append("// Generated from: abi_structs_schema.yaml\n");
        sb.append("// Schema version: ").append(schemaVersion).append("\n");
        sb.append("// Generated at: ").append(generationTimestamp).append("\n");
        sb.append("// Schema hash: ").append(schemaHash).append("\n");
        sb.append("// ============================================================================\n\n");
        
        sb.append("package com.astraeus.native_api;\n\n");
        
        sb.append("import java.lang.foreign.*;\n");
        sb.append("import java.lang.invoke.VarHandle;\n\n");
        
        sb.append("/**\n");
        sb.append(" * Auto-generated FFM MemoryLayout definitions for ABI structs.\n");
        sb.append(" * \n");
        sb.append(" * DO NOT EDIT THIS FILE MANUALLY.\n");
        sb.append(" * Regenerate using: java com.astraeus.tools.ABICodeGenerator\n");
        sb.append(" */\n");
        sb.append("public class StructLayouts {\n\n");
        
        sb.append("    // Schema metadata\n");
        sb.append("    public static final String SCHEMA_VERSION = \"").append(schemaVersion).append("\";\n");
        sb.append("    public static final String GENERATION_TIMESTAMP = \"").append(generationTimestamp).append("\";\n");
        sb.append("    public static final String SCHEMA_HASH = \"").append(schemaHash).append("\";\n\n");
        
        // Generate layout for each struct
        for (StructDef struct : structs) {
            generateJavaStructLayout(sb, struct);
        }
        
        sb.append("}\n");
        
        // Write file
        Path path = Paths.get(outputPath);
        Files.createDirectories(path.getParent());
        Files.writeString(path, sb.toString());
        
        System.out.println("✓ Generated Java layouts");
    }
    
    private void generateJavaStructLayout(StringBuilder sb, StructDef struct) {
        String layoutName = struct.name.toUpperCase() + "_LAYOUT";
        
        sb.append("    // ").append(struct.description).append("\n");
        sb.append("    public static final StructLayout ").append(layoutName).append(" = MemoryLayout.structLayout(\n");
        
        // Generate fields
        for (int i = 0; i < struct.fields.size(); i++) {
            FieldDef field = struct.fields.get(i);
            sb.append("        ");
            
            if (field.arraySize > 0) {
                // Array field
                sb.append("MemoryLayout.sequenceLayout(").append(field.arraySize).append(", ");
                sb.append(mapToJavaLayout(field.type)).append(")");
            } else {
                sb.append(mapToJavaLayout(field.type));
            }
            
            sb.append(".withName(\"").append(field.name).append("\")");
            
            if (i < struct.fields.size() - 1) {
                sb.append(",");
            }
            sb.append("\n");
        }
        
        sb.append("    );\n\n");
        
        // Generate VarHandles for each field
        sb.append("    // VarHandles for ").append(struct.name).append("\n");
        for (FieldDef field : struct.fields) {
            String handleName = struct.name.toUpperCase() + "_" + field.name.toUpperCase();
            sb.append("    public static final VarHandle ").append(handleName).append(" = ");
            sb.append(layoutName).append(".varHandle(\n");
            sb.append("        MemoryLayout.PathElement.groupElement(\"").append(field.name).append("\")");
            if (field.arraySize > 0) {
                sb.append(",\n        MemoryLayout.PathElement.sequenceElement()");
            }
            sb.append("\n    );\n");
        }
        sb.append("\n");
    }
    
    private String mapToCType(String type) {
        switch (type) {
            case "uint8": return "uint8_t";
            case "uint16": return "uint16_t";
            case "uint32": return "uint32_t";
            case "uint64": return "uint64_t";
            case "int8": return "int8_t";
            case "int16": return "int16_t";
            case "int32": return "int32_t";
            case "int64": return "int64_t";
            case "float32": return "float";
            case "float64": return "double";
            case "bool": return "bool";
            case "pointer": return "void*";
            default: throw new IllegalArgumentException("Unknown type: " + type);
        }
    }
    
    private String mapToJavaLayout(String type) {
        switch (type) {
            case "uint8": return "ValueLayout.JAVA_BYTE";
            case "uint16": return "ValueLayout.JAVA_SHORT";
            case "uint32": return "ValueLayout.JAVA_INT";
            case "uint64": return "ValueLayout.JAVA_LONG";
            case "int8": return "ValueLayout.JAVA_BYTE";
            case "int16": return "ValueLayout.JAVA_SHORT";
            case "int32": return "ValueLayout.JAVA_INT";
            case "int64": return "ValueLayout.JAVA_LONG";
            case "float32": return "ValueLayout.JAVA_FLOAT";
            case "float64": return "ValueLayout.JAVA_DOUBLE";
            case "bool": return "ValueLayout.JAVA_BOOLEAN";
            case "pointer": return "ValueLayout.ADDRESS";
            default: throw new IllegalArgumentException("Unknown type: " + type);
        }
    }
    
    private int calculateAlignment(StructDef struct) {
        int maxAlign = 1;
        for (FieldDef field : struct.fields) {
            int align = getTypeAlignment(field.type);
            if (align > maxAlign) {
                maxAlign = align;
            }
        }
        return maxAlign;
    }
    
    private int getTypeAlignment(String type) {
        switch (type) {
            case "uint8":
            case "int8":
            case "bool":
                return 1;
            case "uint16":
            case "int16":
                return 2;
            case "uint32":
            case "int32":
            case "float32":
                return 4;
            case "uint64":
            case "int64":
            case "float64":
            case "pointer":
                return 8;
            default:
                return 1;
        }
    }
    
    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
    
    // Data classes for parsed schema
    static class StructDef {
        String name;
        String description;
        List<FieldDef> fields = new ArrayList<>();
    }
    
    static class FieldDef {
        String name;
        String type;
        String description;
        int arraySize = 0;
    }
}
