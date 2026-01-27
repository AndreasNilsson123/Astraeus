@echo off
setlocal EnableDelayedExpansion

REM ================================================
REM Build script for Astraeus project (Windows)
REM ================================================

echo ================================================
echo Building Astraeus Project
echo ================================================

REM ------------------------------------------------
REM Check prerequisites
REM ------------------------------------------------
echo Checking prerequisites...

REM ---- Check CMake ----
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Please install CMake 3.15 or later.
    exit /b 1
)

REM ---- Check C++ compiler (MSVC via cl.exe OR clang++) ----
where cl >nul 2>&1
if errorlevel 1 (
    where clang++ >nul 2>&1
    if errorlevel 1 (
        echo ERROR: No C++ compiler found.
        echo Please install Visual Studio with "Desktop development with C++"
        echo or install LLVM/Clang.
        exit /b 1
    )
)

REM ---- Check Java ----
where java >nul 2>&1
if errorlevel 1 (
    echo ERROR: Java not found. Please install JDK 21 or later.
    exit /b 1
)

REM ---- Check Maven ----
where mvn >nul 2>&1
if errorlevel 1 (
    echo ERROR: Maven not found. Please install Maven 3.6 or later.
    exit /b 1
)

echo All prerequisites found.
echo.

REM ------------------------------------------------
REM Build C++ engine
REM ------------------------------------------------
echo Building C++ engine...

if not exist build (
    mkdir build
)

pushd build

REM You can optionally force a generator, e.g.:
REM cmake .. -G "Visual Studio 17 2022"
cmake ..
if errorlevel 1 exit /b 1

cmake --build . --config Release
if errorlevel 1 exit /b 1

popd

echo C++ engine built successfully.
echo.

REM ------------------------------------------------
REM Build Java frontend
REM ------------------------------------------------
echo Building Java frontend...

mvn clean package -DskipTests
if errorlevel 1 exit /b 1

echo Java frontend built successfully.
echo.

echo ================================================
echo Build completed successfully!
echo ================================================
echo.

echo To run the application:
echo   set PATH=%%PATH%%;%%CD%%\build\lib
echo   mvn javafx:run

endlocal
