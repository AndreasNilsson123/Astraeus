# Migration Guide: Old Codegen → New Extensible Framework

## Overview

The ABI code generator has been refactored from a monolithic tool to an extensible, path-safe framework. This guide helps you migrate.

## What Changed

### Package Names
**Old**: `com.astraeus.tools.ABICodeGenerator`  
**New**: `com.astraeus.codegen.CodegenCli`

### Gradle Tasks
**Old**: `./gradlew :codegen:generateBindings`  
**New**: `./gradlew :codegen:generateAbi`

### Shell Scripts
**Old**: Manual compilation and execution  
**New**: Uses Gradle task (`regenerate_abi.sh` updated)

### Output Locations
**Old**:
- C++: Various locations
- Java: `java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java`

**New**:
- C++: `engine/generated/EngineABI_Structs.h`
- Java: `java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java`

## Migration Steps

### 1. Update Your Build Scripts

**Before:**
```bash
java -cp ... com.astraeus.tools.ABICodeGenerator schema.yaml /path/to/root
```

**After:**
```bash
# Simplest: use Gradle
./java/gradlew :codegen:generateAbi

# Or directly:
java -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli
```

### 2. Update Gradle Dependencies

**Before:**
```kotlin
tasks.named("compileJava") {
    dependsOn(":codegen:generateBindings")
}
```

**After:**
```kotlin
tasks.named("compileJava") {
    dependsOn(":codegen:generateAbi")
}
```

### 3. Update Import Paths

If you had custom scripts importing generated code, update paths:

**Before:**
```java
import com.astraeus.native_api.StructLayouts;
```

**After:**
```java
import com.astraeus.generated.StructLayouts;
```

### 4. Update CI/CD Pipelines

**Before:**
```yaml
- run: bash regenerate_abi.sh
```

**After:**
```yaml
# Script already updated, but you can also use Gradle directly
- run: cd java && ./gradlew :codegen:generateAbi
```

## New Features You Can Use

### Target Selection
Generate only what you need:
```bash
# Only Java
java ... CodegenCli --targets java-layouts

# Only C++
java ... CodegenCli --targets cpp-header
```

### List Available Targets
```bash
java ... CodegenCli --list-targets
```

### Explicit Repository Path
Useful in unusual environments:
```bash
java ... CodegenCli --repo /explicit/path/to/Astraeus
```

### Custom Schema
```bash
java ... CodegenCli --schema /path/to/custom_schema.yaml
```

## Backwards Compatibility

### Generated Code
- **Fully compatible**: Generated structs/layouts are identical (except timestamp)
- **No code changes needed**: Existing code using generated structures works as-is

### Schema Format
- **Fully compatible**: `abi_structs_schema.yaml` format unchanged
- **No migration needed**: Existing schema files work without modification

### Breaking Changes
**None for normal usage**. The only changes are:
- Gradle task name: `generateBindings` → `generateAbi`
- Package name: `com.astraeus.tools` → `com.astraeus.codegen`
- Main class name: `ABICodeGenerator` → `CodegenCli`

These only affect build scripts, not generated code or application code.

## Troubleshooting

### "Package com.astraeus.tools does not exist"
**Cause**: Build script still references old package  
**Fix**: Update to `com.astraeus.codegen.CodegenCli`

### "Task 'generateBindings' not found"
**Cause**: Gradle task renamed  
**Fix**: Use `:codegen:generateAbi`

### "Could not detect repository root"
**Cause**: Running from unusual location  
**Fix**: Add `--repo /path/to/Astraeus`

### Generated files in wrong location
**Cause**: Old scripts/tasks still running  
**Fix**: 
1. Clean old locations: `rm -rf java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java`
2. Run new task: `./java/gradlew :codegen:generateAbi`
3. Update import statements if needed

## Rollback (If Needed)

If you need to rollback temporarily:

1. Old files are preserved in git history:
   ```bash
   git show HEAD~1:java/codegen/src/main/java/com/astraeus/codegen/ABICodeGenerator.java > old_generator.java
   ```

2. However, we recommend adapting to the new system as it provides:
   - Better path safety
   - Extensibility for new targets
   - Improved error messages
   - CWD-agnostic operation

## Getting Help

1. **Documentation**: See `java/codegen/README.md`
2. **Quick Reference**: See `CODEGEN_QUICK_REFERENCE.md`
3. **CLI Help**: Run `java ... CodegenCli --help`
4. **List Targets**: Run `java ... CodegenCli --list-targets`

## Testing Your Migration

After migrating, verify:

```bash
# 1. Clean build
./java/gradlew clean

# 2. Generate ABI
./java/gradlew :codegen:generateAbi

# 3. Check files exist
ls -l engine/generated/EngineABI_Structs.h
ls -l java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java

# 4. Build frontend
./java/gradlew :frontend:build

# 5. Verify application runs
./java/gradlew :frontend:run
```

If all steps pass, migration is complete!

## Benefits of New System

- ✅ **Path Safety**: No hardcoded paths, works on Windows/Linux
- ✅ **Extensibility**: Easy to add new targets (Rust, docs, etc.)
- ✅ **CWD-Agnostic**: Works from any directory in repo
- ✅ **Better CLI**: Rich options and help messages
- ✅ **Gradle Integration**: Proper caching and up-to-date checks
- ✅ **Documentation**: Comprehensive guides and examples
- ✅ **Maintainability**: Clean architecture with separation of concerns

## Timeline

- **Recommended**: Migrate immediately (simple, low-risk)
- **Required**: Before next major release
- **Support**: Old system removed, use git history if needed

## Questions?

See the comprehensive documentation in `java/codegen/README.md` or file an issue.
