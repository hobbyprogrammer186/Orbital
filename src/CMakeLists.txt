add_executable(orbital
    main.cpp
    Lexer.cpp
    Parser.cpp
)

target_link_libraries(orbital PRIVATE
    orbtlio
)

target_include_directories(orbital PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(orbital PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)