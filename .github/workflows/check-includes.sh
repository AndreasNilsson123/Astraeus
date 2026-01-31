#!/bin/bash
# Include hygiene checker for Astraeus
# Validates that platform-specific code is properly isolated

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ENGINE_DIR="$REPO_ROOT/engine"

echo "=== Astraeus Include Hygiene Check ==="
echo "Checking for platform-specific code outside engine/platform/..."
echo

ERRORS=0

# Check for <windows.h> outside engine/platform/Win32/
echo "Checking for <windows.h> includes..."
if grep -r "#include <windows.h>" "$ENGINE_DIR" \
   --include="*.hpp" --include="*.cpp" --include="*.h" \
   --exclude-dir=platform 2>/dev/null; then
    echo "❌ ERROR: Found <windows.h> includes outside engine/platform/"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ PASS: No <windows.h> includes outside engine/platform/"
fi
echo

# Check for platform ifdefs outside engine/platform/ and CMakeLists.txt
echo "Checking for platform #ifdef statements..."
if grep -r "#ifdef _WIN32\|#if defined(_WIN32)\|#ifdef WIN32\|#ifdef __linux__" "$ENGINE_DIR" \
   --include="*.hpp" --include="*.cpp" --include="*.h" \
   --exclude-dir=platform --exclude="CMakeLists.txt" 2>/dev/null; then
    echo "❌ ERROR: Found platform #ifdef statements outside engine/platform/"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ PASS: No platform #ifdef statements outside engine/platform/"
fi
echo

# Check for X11 headers outside engine/platform/Linux/
echo "Checking for X11 header includes..."
if grep -r "#include <X11/" "$ENGINE_DIR" \
   --include="*.hpp" --include="*.cpp" --include="*.h" \
   --exclude-dir=platform 2>/dev/null; then
    echo "❌ ERROR: Found X11 header includes outside engine/platform/"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ PASS: No X11 header includes outside engine/platform/"
fi
echo

# Check for direct GLAD includes outside renderer backend and platform
echo "Checking for scattered GLAD includes..."
if grep -r "#include <glad/glad" "$ENGINE_DIR" \
   --include="*.hpp" --include="*.h" \
   --exclude-dir=platform --exclude-dir=backend --exclude-dir=third_party 2>/dev/null; then
    echo "⚠️  WARNING: Found GLAD includes in public headers (prefer isolating to backend)"
    echo "   (This is a warning, not a hard error)"
else
    echo "✅ PASS: GLAD includes properly isolated"
fi
echo

# Summary
echo "=== Summary ==="
if [ $ERRORS -eq 0 ]; then
    echo "✅ All include hygiene checks passed!"
    exit 0
else
    echo "❌ Include hygiene check failed with $ERRORS error(s)"
    echo
    echo "To fix:"
    echo "1. Move platform-specific includes to engine/platform/"
    echo "2. Use Platform.hpp API instead of direct platform calls"
    echo "3. See docs/DEPENDENCIES.md for detailed rules"
    exit 1
fi
