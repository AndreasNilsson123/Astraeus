#!/bin/bash
# Comprehensive test of the refactored codegen infrastructure
# Tests all major functionality and validates outputs

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

TESTS_PASSED=0
TESTS_FAILED=0

test_passed() {
    echo -e "${GREEN}✓ $1${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

test_failed() {
    echo -e "${RED}✗ $1${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

echo "================================================"
echo "Codegen Infrastructure Test Suite"
echo "================================================"
echo ""

# Test 1: Gradle compilation
echo -e "${YELLOW}Test 1: Gradle compilation${NC}"
cd "$PROJECT_ROOT/java"
if gradle :codegen:compileJava --console=plain > /dev/null 2>&1; then
    test_passed "Codegen module compiles"
else
    test_failed "Codegen module compilation failed"
fi

# Test 2: CLI --list-targets
echo -e "${YELLOW}Test 2: CLI --list-targets${NC}"
cd "$PROJECT_ROOT"
JAVA_BIN="/usr/lib/jvm/temurin-25-jdk-amd64/bin/java"
if [ ! -f "$JAVA_BIN" ]; then
    JAVA_BIN="java"
fi

OUTPUT=$($JAVA_BIN -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --list-targets 2>&1)
if echo "$OUTPUT" | grep -q "java-layouts" && echo "$OUTPUT" | grep -q "cpp-header"; then
    test_passed "CLI lists targets correctly"
else
    test_failed "CLI --list-targets failed"
fi

# Test 3: CLI --help
echo -e "${YELLOW}Test 3: CLI --help${NC}"
OUTPUT=$($JAVA_BIN -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --help 2>&1)
if echo "$OUTPUT" | grep -q "Usage:" && echo "$OUTPUT" | grep -q "Options:"; then
    test_passed "CLI --help works"
else
    test_failed "CLI --help failed"
fi

# Test 4: Generate from repo root
echo -e "${YELLOW}Test 4: Generate from repo root${NC}"
cd "$PROJECT_ROOT"
rm -rf engine/generated java/frontend/build/generated
if $JAVA_BIN -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli > /dev/null 2>&1; then
    test_passed "Generation from repo root works"
else
    test_failed "Generation from repo root failed"
fi

# Test 5: Check C++ output exists
echo -e "${YELLOW}Test 5: Check C++ output${NC}"
if [ -f "$PROJECT_ROOT/engine/generated/EngineABI_Structs.h" ]; then
    SIZE=$(wc -c < "$PROJECT_ROOT/engine/generated/EngineABI_Structs.h")
    if [ "$SIZE" -gt 1000 ]; then
        test_passed "C++ header generated (${SIZE} bytes)"
    else
        test_failed "C++ header too small (${SIZE} bytes)"
    fi
else
    test_failed "C++ header not found"
fi

# Test 6: Check Java output exists
echo -e "${YELLOW}Test 6: Check Java output${NC}"
JAVA_OUTPUT="$PROJECT_ROOT/java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java"
if [ -f "$JAVA_OUTPUT" ]; then
    SIZE=$(wc -c < "$JAVA_OUTPUT")
    if [ "$SIZE" -gt 1000 ]; then
        test_passed "Java layouts generated (${SIZE} bytes)"
    else
        test_failed "Java layouts too small (${SIZE} bytes)"
    fi
else
    test_failed "Java layouts not found"
fi

# Test 7: Generate from java/ directory
echo -e "${YELLOW}Test 7: Generate from java/ directory${NC}"
cd "$PROJECT_ROOT/java"
if $JAVA_BIN -cp codegen/build/classes/java/main com.astraeus.codegen.CodegenCli > /dev/null 2>&1; then
    test_passed "Generation from java/ directory works"
else
    test_failed "Generation from java/ directory failed"
fi

# Test 8: Gradle task
echo -e "${YELLOW}Test 8: Gradle generateAbi task${NC}"
cd "$PROJECT_ROOT/java"
if gradle :codegen:generateAbi --console=plain > /dev/null 2>&1; then
    test_passed "Gradle generateAbi task works"
else
    test_failed "Gradle generateAbi task failed"
fi

# Test 9: Up-to-date check
echo -e "${YELLOW}Test 9: Gradle up-to-date check${NC}"
OUTPUT=$(gradle :codegen:generateAbi --console=plain 2>&1)
if echo "$OUTPUT" | grep -q "UP-TO-DATE"; then
    test_passed "Gradle up-to-date check works"
else
    test_failed "Gradle up-to-date check failed"
fi

# Test 10: Frontend compilation
echo -e "${YELLOW}Test 10: Frontend compilation with generated sources${NC}"
if gradle :frontend:compileJava --console=plain > /dev/null 2>&1; then
    test_passed "Frontend compiles with generated sources"
else
    test_failed "Frontend compilation failed"
fi

# Test 11: Single target generation
echo -e "${YELLOW}Test 11: Single target generation${NC}"
cd "$PROJECT_ROOT"
rm -f engine/generated/EngineABI_Structs.h
if $JAVA_BIN -cp java/codegen/build/classes/java/main com.astraeus.codegen.CodegenCli --targets cpp-header > /dev/null 2>&1; then
    if [ -f "engine/generated/EngineABI_Structs.h" ]; then
        test_passed "Single target generation works"
    else
        test_failed "Single target did not generate file"
    fi
else
    test_failed "Single target generation failed"
fi

# Test 12: Shell script
echo -e "${YELLOW}Test 12: regenerate_abi.sh script${NC}"
cd "$PROJECT_ROOT"
if bash regenerate_abi.sh > /dev/null 2>&1; then
    test_passed "regenerate_abi.sh works"
else
    test_failed "regenerate_abi.sh failed"
fi

# Summary
echo ""
echo "================================================"
echo "Test Summary"
echo "================================================"
echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
if [ $TESTS_FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
fi
