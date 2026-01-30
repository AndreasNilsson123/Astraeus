#!/bin/bash
# Regenerate ABI struct code from schema
# Run this script whenever you modify abi_structs_schema.yaml

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "================================================"
echo "Regenerating ABI Struct Code"
echo "================================================"

SCHEMA_FILE="$PROJECT_ROOT/engine/api/abi_structs_schema.yaml"
CODEGEN_CLASS="com.astraeus.tools.ABICodeGenerator"
CODEGEN_SRC="$PROJECT_ROOT/java/src/main/java/com/astraeus/tools/ABICodeGenerator.java"

# Compile code generator
echo -e "${YELLOW}Compiling code generator...${NC}"
TEMP_DIR=$(mktemp -d)
javac -d "$TEMP_DIR" --source 17 --target 17 "$CODEGEN_SRC"

# Run code generator
echo -e "${YELLOW}Running code generator...${NC}"
java -cp "$TEMP_DIR" "$CODEGEN_CLASS" "$SCHEMA_FILE" "$PROJECT_ROOT"

# Cleanup
rm -rf "$TEMP_DIR"

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}✓ Code generation completed${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "Generated files:"
echo "  - engine/api/EngineABI_Structs.gen.h"
echo "  - java/src/main/java/com/astraeus/native_api/StructLayouts.gen.java"
echo ""
echo "Run './verify_abi_codegen.sh' to verify the generated files."
