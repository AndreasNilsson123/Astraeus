package com.astraeus.tools;

import java.io.IOException;
import java.nio.file.*;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * ABI Struct Code Generator
 *
 * Generates:
 *  1) C++ POD struct definitions (EngineABI_Structs.h)
 *  2) Java FFM MemoryLayout definitions (StructLayouts.java)
 *
 * from a single YAML schema to prevent drift between languages.
 *
 * Usage:
 *   java com.astraeus.tools.ABICodeGenerator <schema.yaml> <project_root>
 */
public final class ABICodeGenerator {

    private String schemaVersion;
    private String generationTimestamp;
    private String schemaHash;
    private final List<StructDef> structs = new ArrayList<>();

    public static void main(String[] args) {
        if (args.length < 2) {
            System.err.println("Usage: java com.astraeus.tools.ABICodeGenerator <schema.yaml> <project_root>");
            System.exit(1);
        }

        String schemaPath = args[0];
        String projectRoot = args[1];

        try {
            ABICodeGenerator generator = new ABICodeGenerator();
            generator.parseSchema(schemaPath);
            generator.generateCppHeader(projectRoot);
            generator.generateJavaLayouts(projectRoot);

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

    // =============================================================================================
    // Parsing
    // =============================================================================================

    private void parseSchema(String schemaPath) throws Exception {
        System.out.println("Parsing schema: " + schemaPath);

        byte[] schemaBytes = Files.readAllBytes(Paths.get(schemaPath));
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(schemaBytes);
        schemaHash = bytesToHex(hash).substring(0, 16);

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        generationTimestamp = sdf.format(new Date());

        List<String> lines = Files.readAllLines(Paths.get(schemaPath));

        StructDef currentStruct = null;
        FieldDef currentField = null;
        boolean inStructs = false;
        boolean inFields = false;

        boolean readingStructDescriptionBlock = false;
        StringBuilder descBlock = new StringBuilder();
        int descBlockIndent = -1;

        for (int idx = 0; idx < lines.size(); idx++) {
            String line = lines.get(idx);
            String trimmed = line.trim();

            // Handle multiline description block for struct (description: |)
            if (readingStructDescriptionBlock) {
                if (line.isBlank()) {
                    descBlock.append('\n');
                    continue;
                }

                int indent = countLeadingSpaces(line);
                if (indent <= descBlockIndent) {
                    // block ended
                    readingStructDescriptionBlock = false;
                    if (currentStruct != null) {
                        currentStruct.description = descBlock.toString().trim();
                    }
                    descBlock.setLength(0);
                    descBlockIndent = -1;
                    // fallthrough to parse this line normally
                } else {
                    // keep content, strip block indent
                    descBlock.append(line.substring(descBlockIndent + 2)).append('\n');
                    continue;
                }
            }

            // schema version
            if (trimmed.startsWith("version:")) {
                schemaVersion = trimmed.substring("version:".length()).trim().replace("\"", "");
                continue;
            }

            if (trimmed.equals("structs:")) {
                inStructs = true;
                continue;
            }
            if (!inStructs) continue;

            // new struct
            if (line.startsWith("  - name:")) {
                if (currentStruct != null) {
                    structs.add(currentStruct);
                }
                currentStruct = new StructDef();
                currentStruct.name = line.substring(line.indexOf("name:") + 5).trim();
                currentStruct.description = "";
                currentStruct.fields.clear();
                currentField = null;
                inFields = false;
                continue;
            }
            if (currentStruct == null) continue;

            // struct description
            if (line.startsWith("    description:")) {
                String desc = line.substring(line.indexOf("description:") + "description:".length()).trim();
                if (desc.equals("|")) {
                    // Start block. Next lines indented more belong to it.
                    readingStructDescriptionBlock = true;
                    descBlockIndent = countLeadingSpaces(line);
                    descBlock.setLength(0);
                    continue;
                }
                // quoted or inline
                if (desc.startsWith("\"") && desc.endsWith("\"") && desc.length() >= 2) {
                    desc = desc.substring(1, desc.length() - 1);
                }
                currentStruct.description = desc;
                continue;
            }

            if (line.startsWith("    fields:")) {
                inFields = true;
                continue;
            }
            if (!inFields) continue;

            // new field
            if (line.startsWith("      - name:")) {
                currentField = new FieldDef();
                currentField.name = line.substring(line.indexOf("name:") + 5).trim();
                currentField.type = null;
                currentField.description = "";
                currentField.arraySize = 0;
                currentStruct.fields.add(currentField);
                continue;
            }
            if (currentField == null) continue;

            if (line.startsWith("        type:")) {
                currentField.type = line.substring(line.indexOf("type:") + 5).trim();
            } else if (line.startsWith("        array_size:")) {
                currentField.arraySize = Integer.parseInt(
                        line.substring(line.indexOf("array_size:") + "array_size:".length()).trim()
                );
            } else if (line.startsWith("        description:")) {
                String desc = line.substring(line.indexOf("description:") + "description:".length()).trim();
                if (desc.startsWith("\"") && desc.endsWith("\"") && desc.length() >= 2) {
                    desc = desc.substring(1, desc.length() - 1);
                }
                currentField.description = desc;
            }
        }

        if (readingStructDescriptionBlock && currentStruct != null) {
            currentStruct.description = descBlock.toString().trim();
        }

        if (currentStruct != null) {
            structs.add(currentStruct);
        }

        // basic validation
        for (StructDef s : structs) {
            if (s.name == null || s.name.isBlank()) {
                throw new IllegalArgumentException("Struct missing name");
            }
            for (FieldDef f : s.fields) {
                if (f.name == null || f.name.isBlank()) {
                    throw new IllegalArgumentException("Struct " + s.name + " has field with missing name");
                }
                if (f.type == null || f.type.isBlank()) {
                    throw new IllegalArgumentException("Struct " + s.name + " field " + f.name + " missing type");
                }
            }
        }

        System.out.println("Parsed " + structs.size() + " struct definitions");
    }

    private static int countLeadingSpaces(String s) {
        int i = 0;
        while (i < s.length() && s.charAt(i) == ' ') i++;
        return i;
    }

    // =============================================================================================
    // C++ generation
    // =============================================================================================

    private void generateCppHeader(String projectRoot) throws Exception {
        String outputPath = projectRoot + "/engine/generated/EngineABI_Structs.h";
        System.out.println("Generating C++ header: " + outputPath);

        StringBuilder sb = new StringBuilder();

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

        sb.append("#define ASTRAEUS_ABI_SCHEMA_VERSION \"").append(schemaVersion).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_GENERATION_TIMESTAMP \"").append(generationTimestamp).append("\"\n");
        sb.append("#define ASTRAEUS_ABI_SCHEMA_HASH \"").append(schemaHash).append("\"\n\n");

        for (StructDef struct : structs) {
            sb.append("// ").append(struct.description == null ? "" : struct.description).append("\n");
            sb.append("typedef struct ").append(struct.name).append(" {\n");

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

            int align = calculateAlignment(struct);

            sb.append("#ifdef __cplusplus\n");
            sb.append("static_assert(alignof(").append(struct.name).append(") == ").append(align)
                    .append(", \"").append(struct.name).append(" alignment mismatch\");\n");
            sb.append("static_assert(sizeof(").append(struct.name).append(") % ").append(align)
                    .append(" == 0, \"").append(struct.name).append(" size must be multiple of alignment\");\n");

            // Emit offsetof checks for named (non-padding) fields to catch accidental packing changes.
            for (FieldDef field : struct.fields) {
                if (isPaddingField(field)) continue;
                sb.append("static_assert(offsetof(").append(struct.name).append(", ").append(field.name)
                        .append(") % ").append(getTypeAlignment(field.type))
                        .append(" == 0, \"").append(struct.name).append(".").append(field.name)
                        .append(" offset not aligned\");\n");
            }
            sb.append("#endif\n\n");
        }

        sb.append("#ifdef __cplusplus\n");
        sb.append("}\n");
        sb.append("#endif\n\n");

        sb.append("#endif // ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H\n");

        Path path = Paths.get(outputPath);
        Files.createDirectories(path.getParent());
        Files.writeString(path, sb.toString());

        System.out.println("✓ Generated C++ header");
    }

    // =============================================================================================
    // Java generation
    // =============================================================================================

    private void generateJavaLayouts(String projectRoot) throws Exception {
        String outputPath = projectRoot + "/java/frontend/src/main/java/com/astraeus/generated/StructLayouts.java";
        System.out.println("Generating Java layouts: " + outputPath);

        StringBuilder sb = new StringBuilder();

        sb.append("// ============================================================================\n");
        sb.append("// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY\n");
        sb.append("// Generated from: abi_structs_schema.yaml\n");
        sb.append("// Schema version: ").append(schemaVersion).append("\n");
        sb.append("// Generated at: ").append(generationTimestamp).append("\n");
        sb.append("// Schema hash: ").append(schemaHash).append("\n");
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

        sb.append("    public static final String SCHEMA_VERSION = \"").append(schemaVersion).append("\";\n");
        sb.append("    public static final String GENERATION_TIMESTAMP = \"").append(generationTimestamp).append("\";\n");
        sb.append("    public static final String SCHEMA_HASH = \"").append(schemaHash).append("\";\n\n");

        for (StructDef struct : structs) {
            generateJavaStructLayout(sb, struct);
        }

        sb.append("}\n");

        Path path = Paths.get(outputPath);
        Files.createDirectories(path.getParent());
        Files.writeString(path, sb.toString());

        System.out.println("✓ Generated Java layouts");
    }

    private void generateJavaStructLayout(StringBuilder sb, StructDef struct) {
        String structName = struct.name;
        String layoutConst = toUpperSnake(structName) + "_LAYOUT";
        int structAlign = calculateAlignment(struct);

        sb.append("    /** ").append(escapeJavadoc(struct.description)).append(" */\n");
        sb.append("    public static final StructLayout ").append(layoutConst).append(" = MemoryLayout.structLayout(\n");

        // fields
        for (int i = 0; i < struct.fields.size(); i++) {
            FieldDef field = struct.fields.get(i);

            sb.append("        ");

            if (isPaddingField(field)) {
                // Emit anonymous padding (NOT sequenceLayout) to avoid treating it as a real field.
                long bytes = (field.arraySize > 0) ? field.arraySize : 0;
                if (bytes <= 0) {
                    // If user wrote `_padding` without array_size, do nothing (or throw).
                    bytes = 1;
                }
                sb.append("MemoryLayout.paddingLayout(").append(bytes).append(")");
            } else if (field.arraySize > 0) {
                // Non-padding arrays: real sequences.
                sb.append("MemoryLayout.sequenceLayout(").append(field.arraySize).append(", ")
                        .append(mapToJavaLayout(field.type)).append(")")
                        .append(".withName(\"").append(field.name).append("\")");
            } else {
                sb.append(mapToJavaLayout(field.type)).append(".withName(\"").append(field.name).append("\")");
            }

            if (i < struct.fields.size() - 1) sb.append(",");
            sb.append("\n");
        }

        sb.append("    ).withByteAlignment(").append(structAlign).append(");\n\n");

        // VarHandles (skip padding)
        sb.append("    // VarHandles for ").append(structName).append("\n");
        for (FieldDef field : struct.fields) {
            if (isPaddingField(field)) continue;

            String handleName = toUpperSnake(structName) + "_" + toUpperSnake(field.name);

            sb.append("    public static final VarHandle ").append(handleName).append(" = ")
                    .append(layoutConst).append(".varHandle(\n")
                    .append("        MemoryLayout.PathElement.groupElement(\"").append(field.name).append("\")");

            if (field.arraySize > 0) {
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

    private static boolean isPaddingField(FieldDef field) {
        // Convention: field named _padding is treated as anonymous padding (layout-only).
        return "_padding".equals(field.name);
    }

    // =============================================================================================
    // Type mapping / alignment
    // =============================================================================================

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
        for (FieldDef field : struct.fields) {
            if (isPaddingField(field)) continue;
            int align = getTypeAlignment(field.type);
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

    // =============================================================================================
    // Utils
    // =============================================================================================

    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) sb.append(String.format("%02x", b));
        return sb.toString();
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

    // =============================================================================================
    // Schema model
    // =============================================================================================

    static final class StructDef {
        String name;
        String description;
        final List<FieldDef> fields = new ArrayList<>();
    }

    static final class FieldDef {
        String name;
        String type;
        String description;
        int arraySize;
    }
}
