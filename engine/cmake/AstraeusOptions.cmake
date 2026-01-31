include_guard(GLOBAL)

option(ASTRAEUS_BUILD_SHARED      "Build shared library (DLL/.so)" ON)
option(ASTRAEUS_BUILD_EXAMPLES    "Build example programs"         ON)
option(ASTRAEUS_BUILD_TESTS       "Build tests"                    OFF)
option(ASTRAEUS_ENABLE_VALIDATION "Enable validation layers"       ON)

if(WIN32)
    set(_ASTRAEUS_DEFAULT_WGL ON)
    set(_ASTRAEUS_DEFAULT_EGL OFF)
else()
    set(_ASTRAEUS_DEFAULT_WGL OFF)
    set(_ASTRAEUS_DEFAULT_EGL ON)
endif()

option(ASTRAEUS_ENABLE_WGL "Enable WGL backend" ${_ASTRAEUS_DEFAULT_WGL})

# Note: On Windows EGL typically means ANGLE; your current code uses pkg-config, so we keep EGL off by default on WIN32.
option(ASTRAEUS_ENABLE_EGL "Enable EGL backend" ${_ASTRAEUS_DEFAULT_EGL})
