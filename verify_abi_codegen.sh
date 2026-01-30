#!/bin/bash
# Script to verify that ABI struct generated files are up-to-date
# Usage: ./verify_abi_codegen.sh

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "================================================"
echo "Verifying ABI Struct Code Generation"
echo "================================================"

# Check if schema file exists
SCHEMA_FILE="$PROJECT_ROOT/engine/api/abi_structs_schema.yaml"
if [ ! -f "$SCHEMA_FILE" ]; then
    echo -e "${RED}ERROR: Schema file not found: $SCHEMA_FILE${NC}"
    exit 1
fi

# Check if code generator exists
CODEGEN_CLASS="com.astraeus.tools.ABICodeGenerator"
CODEGEN_FILE="$PROJECT_ROOT/java/src/main/java/com/astraeus/tools/ABICodeGenerator.java"
if [ ! -f "$CODEGEN_FILE" ]; then
    echo -e "${RED}ERROR: Code generator not found: $CODEGEN_FILE${NC}"
    exit 1
fi

# Read current schema hash from generated files
CPP_HEADER="$PROJECT_ROOT/engine/api/EngineABI_Structs.gen.h"
JAVA_LAYOUTS="$PROJECT_ROOT/java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java"

if [ ! -f "$CPP_HEADER" ]; then
    echo -e "${YELLOW}WARNING: C++ header not found. Run code generation first.${NC}"
    exit 1
fi

if [ ! -f "$JAVA_LAYOUTS" ]; then
    echo -e "${YELLOW}WARNING: Java layouts not found. Run code generation first.${NC}"
    exit 1
fi

# Extract hash from generated file
CURRENT_HASH=$(grep "Schema hash:" "$CPP_HEADER" | head -1 | awk '{print $4}')
echo "Current generated hash: $CURRENT_HASH"

# Calculate current schema hash
SCHEMA_HASH=$(sha256sum "$SCHEMA_FILE" | awk '{print $1}' | cut -c1-16)
echo "Current schema hash:    $SCHEMA_HASH"

# Compare hashes
if [ "$CURRENT_HASH" != "$SCHEMA_HASH" ]; then
    echo ""
    echo -e "${RED}================================================${NC}"
    echo -e "${RED}ERROR: Generated files are OUT OF DATE!${NC}"
    echo -e "${RED}================================================${NC}"
    echo ""
    echo "The ABI struct schema has changed but generated files have not been updated."
    echo ""
    echo "To regenerate:"
    echo "  1. Using Maven: mvn generate-sources"
    echo "  2. Manually: java com.astraeus.tools.ABICodeGenerator engine/api/abi_structs_schema.yaml ."
    echo ""
    exit 1
fi

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}✓ Generated files are up-to-date${NC}"
echo -e "${GREEN}================================================${NC}"
exit 0
