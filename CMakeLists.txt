# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.2)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

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

project ("Orbital")

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Include Dependencies
if(WIN32)
    # Use the target name vcpkg suggested
    find_package(unofficial-pdcurses CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::pdcurses::pdcurses)
else()
    find_package(Curses REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
    include_directories(${CURSES_INCLUDE_DIRS})
endif()

# Include sub-projects.
include_directories("include")
add_subdirectory(docs)
add_subdirectory(io)
add_subdirectory(src)
