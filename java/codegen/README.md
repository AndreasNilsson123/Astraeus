# Astraeus ABI Code Generator

Extensible, path-safe code generation framework for the Astraeus FFM ABI layer.

## Overview

This module generates language bindings from a single schema definition (`engine/api/abi_structs_schema.yaml`):

- **C++ POD structs** → `engine/generated/EngineABI_Structs.h`
- **Java FFM layouts** → `java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java`

## Architecture

```
com.astraeus.codegen/
├── framework/          Core abstractions
│   ├── GenerationTarget      Plugin interface for targets
│   ├── GenerationContext     Context passed to targets
│   └── TargetRegistry        Registry of available targets
├── targets/            Concrete generation targets
│   ├── JavaLayoutsTarget     Generates Java FFM MemoryLayouts
│   └── CppHeaderTarget       Generates C++ POD headers
├── paths/              Path resolution (cross-platform)
│   ├── RepoLayout            Repository structure knowledge
│   └── PathResolver          Path utilities (no "/" hardcoded)
├── schema/             Schema model and parsing
│   ├── SchemaModel           Parsed schema representation
│   └── SchemaLoader          YAML parser (ad-hoc, no deps)
└── CodegenCli          Main entry point
```

## Usage

### From Command Line

```bash
# Auto-detect repository and generate all targets
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli

# List available targets
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --list-targets

# Generate specific targets only
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --targets java-layouts,cpp-header

# Specify repository explicitly (if auto-detection fails)
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --repo /path/to/Astraeus

# Use custom schema
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --schema custom_schema.yaml

# Show help
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --help
```

### From Gradle

```bash
# From repository root
./gradlew :codegen:generateAbi

# From java/ directory
../gradlew :codegen:generateAbi

# Frontend build automatically triggers codegen
./gradlew :frontend:build
```

The `:frontend:compileJava` task automatically depends on `:codegen:generateAbi`, so generated sources are always up-to-date.

## Path Resolution

All path operations use `java.nio.file.Path` for Windows/Linux compatibility. **No "/" hardcoded.**

### Repository Auto-Detection

`RepoLayout.autoDetect()` searches upward from CWD for:
- `.git` directory
- `engine/` directory
- `java/` directory  
- `engine/api/abi_structs_schema.yaml`

Must find at least 2 indicators. Works from:
- Repository root: `/path/to/Astraeus`
- Java directory: `/path/to/Astraeus/java`
- Any subdirectory

### Path Methods

```java
RepoLayout layout = RepoLayout.autoDetect();

layout.getRepoRoot();                  // /path/to/Astraeus
layout.getEngineDir();                 // /path/to/Astraeus/engine
layout.getJavaDir();                   // /path/to/Astraeus/java
layout.getFrontendDir();               // /path/to/Astraeus/java/frontend
layout.getCodegenDir();                // /path/to/Astraeus/java/codegen
layout.getDefaultSchemaPath();         // /path/to/Astraeus/engine/api/abi_structs_schema.yaml
layout.getEngineGeneratedDir();        // /path/to/Astraeus/engine/generated
layout.getFrontendGeneratedDir();      // /path/to/Astraeus/java/frontend/build/generated/sources/astraeusAbi/main
```

All methods return `Path` objects. Use `Path.resolve()` for further path operations.

## Adding a New Target

### 1. Create Target Class

```java
package com.astraeus.codegen.targets;

import com.astraeus.codegen.framework.*;
import com.astraeus.codegen.schema.SchemaModel;

public final class MyNewTarget implements GenerationTarget {
    
    @Override
    public String getName() {
        return "my-target";  // Used in --targets CLI arg
    }
    
    @Override
    public String getDescription() {
        return "Generates my custom output";
    }
    
    @Override
    public void generate(SchemaModel schema, GenerationContext context) throws Exception {
        // Access repository layout
        RepoLayout layout = context.getRepoLayout();
        
        // Compute output path (use Path, not String concatenation!)
        Path outputFile = layout.getRepoRoot()
            .resolve("my_output")
            .resolve("generated_file.txt");
        
        // Generate content
        StringBuilder sb = new StringBuilder();
        sb.append("// Generated code\n");
        
        for (SchemaModel.StructDef struct : schema.getStructs()) {
            sb.append("struct ").append(struct.getName()).append("\n");
            for (SchemaModel.FieldDef field : struct.getFields()) {
                if (!field.isPadding()) {
                    sb.append("  ").append(field.getName()).append(": ").append(field.getType()).append("\n");
                }
            }
        }
        
        // Write output (create directories first!)
        Files.createDirectories(outputFile.getParent());
        Files.writeString(outputFile, sb.toString());
        
        System.out.println("✓ Generated my target: " + outputFile);
    }
}
```

### 2. Register Target

In `TargetRegistry.java`, add to the static initializer:

```java
static {
    register(new JavaLayoutsTarget());
    register(new CppHeaderTarget());
    register(new MyNewTarget());  // <-- Add this
}
```

### 3. Test

```bash
# List targets (should show "my-target")
java -cp ... com.astraeus.codegen.CodegenCli --list-targets

# Run your target
java -cp ... com.astraeus.codegen.CodegenCli --targets my-target
```

## Schema Model

The `SchemaModel` provides:

```java
SchemaModel schema = SchemaLoader.load(schemaPath);

schema.getVersion();              // "1.0.0"
schema.getNamespace();            // "astraeus"
schema.getSchemaHash();           // "a1b2c3..." (first 16 chars of SHA-256)
schema.getGenerationTimestamp();  // "2024-01-15 10:30:00"

for (SchemaModel.StructDef struct : schema.getStructs()) {
    String name = struct.getName();
    String desc = struct.getDescription();
    
    for (SchemaModel.FieldDef field : struct.getFields()) {
        String fieldName = field.getName();
        String fieldType = field.getType();       // "uint32", "float64", etc.
        int arraySize = field.getArraySize();     // 0 if not array
        boolean isPadding = field.isPadding();    // true if name == "_padding"
    }
}
```

### Type Mappings

Schema types map to:

| Schema Type | C++ Type       | Java Layout            |
|-------------|----------------|------------------------|
| uint8       | uint8_t        | JAVA_BYTE              |
| uint16      | uint16_t       | JAVA_SHORT             |
| uint32      | uint32_t       | JAVA_INT               |
| uint64      | uint64_t       | JAVA_LONG              |
| int8        | int8_t         | JAVA_BYTE              |
| int16       | int16_t        | JAVA_SHORT             |
| int32       | int32_t        | JAVA_INT               |
| int64       | int64_t        | JAVA_LONG              |
| float32     | float          | JAVA_FLOAT             |
| float64     | double         | JAVA_DOUBLE            |
| bool        | bool           | JAVA_BOOLEAN           |
| pointer     | void*          | ADDRESS                |
| char_pointer| const char*    | ADDRESS                |

## Design Constraints

1. **No external dependencies** beyond JDK (ad-hoc YAML parsing)
2. **Cross-platform paths**: Always use `Path`, never `String + "/"`
3. **Deterministic output**: Same input always produces same output
4. **CWD-agnostic**: Works from any directory in repository
5. **Gradle integration**: Proper input/output tracking for up-to-date checks

## Gradle Integration Details

### codegen/build.gradle.kts

```kotlin
tasks.register<JavaExec>("generateAbi") {
    dependsOn(tasks.named("classes"))
    classpath = sourceSets["main"].runtimeClasspath
    mainClass.set("com.astraeus.codegen.CodegenCli")
    
    // Inputs: schema file (triggers regeneration on change)
    inputs.file("../../engine/api/abi_structs_schema.yaml")
    
    // Outputs: generated directories (for up-to-date checks)
    outputs.dir("../../engine/generated")
    outputs.dir(layout.buildDirectory.dir("generated/sources/astraeusAbi/main"))
}
```

### frontend/build.gradle.kts

```kotlin
// Add generated sources to source set
val codegenGeneratedDir = project(":codegen").layout.buildDirectory.dir("generated/sources/astraeusAbi/main")

sourceSets {
    named("main") {
        java.srcDir(codegenGeneratedDir)
    }
}

// Ensure codegen runs before compilation
tasks.named<JavaCompile>("compileJava") {
    dependsOn(":codegen:generateAbi")
}
```

## Troubleshooting

### "Could not detect repository root"

Run with explicit `--repo`:
```bash
java -cp ... com.astraeus.codegen.CodegenCli --repo /absolute/path/to/Astraeus
```

Or run from a directory within the repository (not outside it).

### "Package does not exist" errors in IDE

1. Run `./gradlew :codegen:generateAbi` to generate sources
2. Refresh/reimport Gradle project in IDE
3. Check that `java/frontend/build/generated/sources/astraeusAbi/main` is marked as source root

### Generated files not updating

Force regeneration:
```bash
# Clean and regenerate
./gradlew clean :codegen:generateAbi

# Or delete generated directories manually
rm -rf engine/generated java/frontend/build/generated
./gradlew :codegen:generateAbi
```

### Path issues on Windows

All paths use `java.nio.file.Path`, which handles Windows backslashes automatically. If you see path issues:
- Check that you're using `Path.resolve()`, not string concatenation
- Use `path.toString()` only for display, never for path construction

## Development Workflow

1. **Modify schema**: Edit `engine/api/abi_structs_schema.yaml`
2. **Regenerate**: Run `./gradlew :codegen:generateAbi` (or build frontend)
3. **Review output**: Check generated files in `engine/generated/` and `java/frontend/build/generated/...`
4. **Add new target**: Create target class, register in `TargetRegistry`, test with `--list-targets`

## Testing

```bash
# Test from repo root
cd /path/to/Astraeus
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli

# Test from java/ directory
cd /path/to/Astraeus/java
java -cp codegen/build/classes/java/main com.astraeus.codegen.CodegenCli

# Test from codegen/ directory
cd /path/to/Astraeus/java/codegen
java -cp build/classes/java/main com.astraeus.codegen.CodegenCli

# Verify outputs exist
ls -la ../../engine/generated/EngineABI_Structs.h
ls -la ../frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java
```

## License

Part of the Astraeus project. See repository root LICENSE file.
