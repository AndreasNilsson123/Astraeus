#!/bin/bash
# Regenerate ABI struct code from schema using the new extensible framework
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

# Use Gradle to run the new codegen framework
echo -e "${YELLOW}Running code generator via Gradle...${NC}"
cd "$PROJECT_ROOT/java"
./gradlew :codegen:generateAbi --console=plain

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}✓ Code generation completed${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "Generated files:"
echo "  - engine/generated/EngineABI_Structs.h"
echo "  - java/frontend/build/generated/sources/astraeusAbi/main/com/astraeus/generated/StructLayouts.java"
echo ""
echo "Run './verify_abi_codegen.sh' to verify the generated files."

