add_executable(orbital
    main.cpp
    Lexer.cpp
    Parser.cpp
)

# Link Runtime As Statically
if(MSVC)
    set_property(TARGET orbital PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(orbital PRIVATE -static-libgcc -static-libstdc++)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(orbital PRIVATE -static-libgcc -static-libstdc++)
endif()

target_link_libraries(orbital PRIVATE
    orbtlio
    ${CURSES_LIBRARIES}
)

target_include_directories(orbital PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(orbital PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)