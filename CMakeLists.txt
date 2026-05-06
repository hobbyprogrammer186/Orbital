# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

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
#[[
if(WIN32)
    # Use dlfcn-win32 to provide dlopen/dlsym on Windows
    find_package(dlfcn-win32 REQUIRED)
    set(DL_LIBRARY dlfcn-win32::dl)
else()
    # Use standard POSIX dlfcn
    set(DL_LIBRARY ${CMAKE_DL_LIBS})
endif()
]]
project ("Orbital")
set(VERSION_STRING "1.0.0-alpha")
string(TOLOWER ${PROJECT_NAME} EXECUTEABLE_NAME)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Include Dependencies
if(WIN32)
    find_package(unofficial-pdcurses CONFIG REQUIRED)
else()
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIRS})
endif()

# Add Compile Definations
add_compile_definitions(VERSION_STRING="${VERSION_STRING}")
add_compile_definitions(NAME_STRING="${PROJECT_NAME}")

# Include sub-projects.
include_directories("include")
add_subdirectory(docs)
add_subdirectory(io)
add_subdirectory(src)
