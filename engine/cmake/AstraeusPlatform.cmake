include_guard(GLOBAL)

function(astraeus_apply_platform target)
    # Keep Windows.h from poisoning code
    if(WIN32)
        target_compile_definitions(${target} PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
        target_sources(${target} PRIVATE
                ${ASTRAEUS_ROOT_DIR}/platform/Platform.hpp
                ${ASTRAEUS_ROOT_DIR}/platform/Win32/Win32Platform.cpp
                ${ASTRAEUS_ROOT_DIR}/platform/Win32/Win32Headers.hpp
        )
        target_link_libraries(${target} PUBLIC winmm)
    elseif(UNIX)
        target_sources(${target} PRIVATE
                ${ASTRAEUS_ROOT_DIR}/platform/Platform.hpp
                ${ASTRAEUS_ROOT_DIR}/platform/Linux/LinuxPlatform.cpp
                ${ASTRAEUS_ROOT_DIR}/platform/Linux/X11Headers.hpp
        )
    else()
        target_sources(${target} PRIVATE
                ${ASTRAEUS_ROOT_DIR}/platform/Platform.hpp
        )
    endif()
endfunction()
