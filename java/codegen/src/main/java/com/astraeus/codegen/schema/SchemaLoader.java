package com.astraeus.codegen.schema;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Loads and parses the ABI struct schema YAML file.
 * Uses simple ad-hoc parsing (no external YAML libraries).
 */
public final class SchemaLoader {
    
    private SchemaLoader() {}
    
    /**
     * Load schema from the given file path.
     */
    public static SchemaModel load(Path schemaPath) throws Exception {
        System.out.println("Loading schema: " + schemaPath);
        
        if (!Files.exists(schemaPath)) {
            throw new IOException("Schema file not found: " + schemaPath);
        }
        
        // Compute hash of schema file
        byte[] schemaBytes = Files.readAllBytes(schemaPath);
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(schemaBytes);
        String schemaHash = bytesToHex(hash).substring(0, 16);
        
        // Generation timestamp
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String generationTimestamp = sdf.format(new Date());
        
        // Parse YAML
        List<String> lines = Files.readAllLines(schemaPath);
        
        String version = null;
        String namespace = null;
        List<SchemaModel.StructDef> structs = new ArrayList<>();
        
        SchemaModel.StructDef currentStruct = null;
        SchemaModel.FieldDef currentField = null;
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
                        currentStruct.setDescription(descBlock.toString().trim());
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
                version = trimmed.substring("version:".length()).trim().replace("\"", "");
                continue;
            }
            
            // namespace
            if (trimmed.startsWith("namespace:")) {
                namespace = trimmed.substring("namespace:".length()).trim().replace("\"", "");
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
                currentStruct = new SchemaModel.StructDef();
                currentStruct.setName(line.substring(line.indexOf("name:") + 5).trim());
                currentStruct.setDescription("");
                currentStruct.getFields().clear();
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
                currentStruct.setDescription(desc);
                continue;
            }
            
            if (line.startsWith("    fields:")) {
                inFields = true;
                continue;
            }
            if (!inFields) continue;
            
            // new field
            if (line.startsWith("      - name:")) {
                currentField = new SchemaModel.FieldDef();
                currentField.setName(line.substring(line.indexOf("name:") + 5).trim());
                currentField.setType(null);
                currentField.setDescription("");
                currentField.setArraySize(0);
                currentStruct.getFields().add(currentField);
                continue;
            }
            if (currentField == null) continue;
            
            if (line.startsWith("        type:")) {
                currentField.setType(line.substring(line.indexOf("type:") + 5).trim());
            } else if (line.startsWith("        array_size:")) {
                currentField.setArraySize(Integer.parseInt(
                        line.substring(line.indexOf("array_size:") + "array_size:".length()).trim()
                ));
            } else if (line.startsWith("        description:")) {
                String desc = line.substring(line.indexOf("description:") + "description:".length()).trim();
                if (desc.startsWith("\"") && desc.endsWith("\"") && desc.length() >= 2) {
                    desc = desc.substring(1, desc.length() - 1);
                }
                currentField.setDescription(desc);
            }
        }
        
        if (readingStructDescriptionBlock && currentStruct != null) {
            currentStruct.setDescription(descBlock.toString().trim());
        }
        
        if (currentStruct != null) {
            structs.add(currentStruct);
        }
        
        // Validation
        if (version == null || version.isBlank()) {
            throw new IllegalArgumentException("Schema missing version");
        }
        if (namespace == null || namespace.isBlank()) {
            namespace = "astraeus"; // default
        }
        
        for (SchemaModel.StructDef s : structs) {
            if (s.getName() == null || s.getName().isBlank()) {
                throw new IllegalArgumentException("Struct missing name");
            }
            for (SchemaModel.FieldDef f : s.getFields()) {
                if (f.getName() == null || f.getName().isBlank()) {
                    throw new IllegalArgumentException("Struct " + s.getName() + " has field with missing name");
                }
                if (f.getType() == null || f.getType().isBlank()) {
                    throw new IllegalArgumentException("Struct " + s.getName() + " field " + f.getName() + " missing type");
                }
            }
        }
        
        System.out.println("Loaded " + structs.size() + " struct definitions (version: " + version + ")");
        
        return new SchemaModel(version, namespace, structs, schemaHash, generationTimestamp);
    }
    
    private static int countLeadingSpaces(String s) {
        int i = 0;
        while (i < s.length() && s.charAt(i) == ' ') i++;
        return i;
    }
    
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) sb.append(String.format("%02x", b));
        return sb.toString();
    }
}
