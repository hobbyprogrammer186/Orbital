find_program(SPHINXBUILD NAMES sphinx-build sphinx-build.exe)

if(SPHINXBUILD)
    add_custom_target(docs
        COMMAND ${SPHINXBUILD} -M html "${CMAKE_SOURCE_DIR}/docs" "${CMAKE_BINARY_DIR}/docs"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/docs"
        COMMENT "Generating HTML documentation with Sphinx"
    )
else()
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E echo "Error: Sphinx not found! Documentation cannot be generated."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Sphinx is missing - build will fail"
    )
    message(STATUS "Sphinx not found. The 'docs' target will show an error if run.")
endif()