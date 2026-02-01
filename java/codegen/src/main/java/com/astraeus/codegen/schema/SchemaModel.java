package com.astraeus.codegen.schema;

import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Schema model representing the parsed ABI struct schema.
 * Contains all struct definitions and metadata.
 */
public final class SchemaModel {
    
    private final String version;
    private final String namespace;
    private final List<StructDef> structs;
    private final String schemaHash;
    private final String generationTimestamp;
    
    public SchemaModel(String version, String namespace, List<StructDef> structs, 
                      String schemaHash, String generationTimestamp) {
        this.version = Objects.requireNonNull(version, "version");
        this.namespace = Objects.requireNonNull(namespace, "namespace");
        this.structs = List.copyOf(structs);
        this.schemaHash = Objects.requireNonNull(schemaHash, "schemaHash");
        this.generationTimestamp = Objects.requireNonNull(generationTimestamp, "generationTimestamp");
    }
    
    public String getVersion() {
        return version;
    }
    
    public String getNamespace() {
        return namespace;
    }
    
    public List<StructDef> getStructs() {
        return structs;
    }
    
    public String getSchemaHash() {
        return schemaHash;
    }
    
    public String getGenerationTimestamp() {
        return generationTimestamp;
    }
    
    /**
     * Definition of a single struct in the schema.
     */
    public static final class StructDef {
        private String name;
        private String description;
        private final List<FieldDef> fields = new ArrayList<>();
        
        public StructDef() {}
        
        public String getName() {
            return name;
        }
        
        public void setName(String name) {
            this.name = name;
        }
        
        public String getDescription() {
            return description;
        }
        
        public void setDescription(String description) {
            this.description = description;
        }
        
        public List<FieldDef> getFields() {
            return fields;
        }
    }
    
    /**
     * Definition of a single field within a struct.
     */
    public static final class FieldDef {
        private String name;
        private String type;
        private String description;
        private int arraySize;
        
        public FieldDef() {}
        
        public String getName() {
            return name;
        }
        
        public void setName(String name) {
            this.name = name;
        }
        
        public String getType() {
            return type;
        }
        
        public void setType(String type) {
            this.type = type;
        }
        
        public String getDescription() {
            return description;
        }
        
        public void setDescription(String description) {
            this.description = description;
        }
        
        public int getArraySize() {
            return arraySize;
        }
        
        public void setArraySize(int arraySize) {
            this.arraySize = arraySize;
        }
        
        /**
         * Check if this field represents padding (by naming convention).
         */
        public boolean isPadding() {
            return "_padding".equals(name);
        }
    }
}
