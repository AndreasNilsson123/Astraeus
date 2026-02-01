cmake_minimum_required(VERSION 3.15)
project(Astraeus VERSION 0.1.0 LANGUAGES C CXX)

set(ASTRAEUS_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusOptions.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusOutput.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusThirdParty.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusPlatform.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusWarnings.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusEngine.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusExamples.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusTests.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusInstall.cmake)
include(${ASTRAEUS_ROOT_DIR}/cmake/AstraeusSummary.cmake)
