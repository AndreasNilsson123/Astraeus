include_guard(GLOBAL)

install(TARGETS astraeus_engine
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(FILES ${ASTRAEUS_ROOT_DIR}/api/EngineAPI.h
        DESTINATION include/astraeus
)
