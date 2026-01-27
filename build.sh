#!/bin/bash
# Build script for Astraeus project

set -e

echo "================================"
echo "Building Astraeus Project"
echo "================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake not found. Please install CMake 3.15 or later.${NC}"
    exit 1
fi

# Check C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}C++ compiler not found. Please install GCC or Clang.${NC}"
    exit 1
fi

# Check Java
if ! command -v java &> /dev/null; then
    echo -e "${RED}Java not found. Please install JDK 21 or later.${NC}"
    exit 1
fi

# Check Maven
if ! command -v mvn &> /dev/null; then
    echo -e "${RED}Maven not found. Please install Maven 3.6 or later.${NC}"
    exit 1
fi

echo -e "${GREEN}All prerequisites found.${NC}"
echo ""

# Build C++ engine
echo -e "${YELLOW}Building C++ engine...${NC}"
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
cd ..
echo -e "${GREEN}C++ engine built successfully.${NC}"
echo ""

# Build Java frontend
echo -e "${YELLOW}Building Java frontend...${NC}"
mvn clean package -DskipTests
echo -e "${GREEN}Java frontend built successfully.${NC}"
echo ""

echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================${NC}"
echo ""
echo "To run the application:"
echo "  export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$(pwd)/build/lib"
echo "  mvn javafx:run"
