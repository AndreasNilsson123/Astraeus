# Codegen Quick Reference

## Common Commands

### Generate All Targets
```bash
# From anywhere in the repo
cd /path/to/Astraeus
./java/gradlew :codegen:generateAbi

# Or use Java directly
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli
```

### List Available Targets
```bash
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --list-targets
```

### Generate Specific Target
```bash
# Only Java layouts
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --targets java-layouts

# Only C++ header
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --targets cpp-header
```

### Frontend Build (Auto-generates)
```bash
cd java
./gradlew :frontend:build
# Automatically runs :codegen:generateAbi before compilation
```

## Output Locations

- **C++ Header**: `engine/generated/EngineABI_Structs.h`
- **Java Layouts**: `java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java`

## Schema Location

`engine/api/abi_structs_schema.yaml`

## Adding a New Target

1. Create `java/codegen/src/main/java/com/astraeus/codegen/targets/MyTarget.java`
2. Implement `GenerationTarget` interface
3. Register in `TargetRegistry.java` static block
4. Test with `--list-targets`

## Troubleshooting

### "Could not detect repository root"
```bash
java ... CodegenCli --repo /absolute/path/to/Astraeus
```

### Generated files not found in IDE
```bash
./java/gradlew :codegen:generateAbi
# Then refresh/reimport Gradle project in IDE
```

### Force regeneration
```bash
rm -rf engine/generated java/frontend/build/generated
./java/gradlew :codegen:generateAbi
```

## Package Structure

```
com.astraeus.codegen/
├── CodegenCli              - Main entry point
├── framework/              - Core abstractions
│   ├── GenerationTarget    - Plugin interface
│   ├── GenerationContext   - Context for generation
│   └── TargetRegistry      - Target registration
├── targets/                - Implementations
│   ├── JavaLayoutsTarget   - Java FFM layouts
│   └── CppHeaderTarget     - C++ structs
├── paths/                  - Path utilities
│   ├── RepoLayout          - Repository structure
│   └── PathResolver        - Path operations
└── schema/                 - Schema model
    ├── SchemaModel         - Schema representation
    └── SchemaLoader        - YAML parser
```

See `java/codegen/README.md` for full documentation.
