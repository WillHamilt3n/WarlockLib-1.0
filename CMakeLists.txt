cmake_minimum_required(VERSION 3.15)
project(WarlockLib VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Access
    ${CMAKE_SOURCE_DIR}/Evasion
    ${CMAKE_SOURCE_DIR}/Utils
    ${CMAKE_SOURCE_DIR}/UI
)

# Core library sources
set(CORE_SOURCES
    Core/Memory.cpp
    Core/Process.cpp
    Core/Scanner.cpp
)

set(CORE_HEADERS
    Core/Memory.h
    Core/Process.h
    Core/Scanner.h
)

# Access layer sources
set(ACCESS_SOURCES
    Access/Driver.cpp
    Access/DMA.cpp
    Access/Hypervisor.cpp
)

set(ACCESS_HEADERS
    Access/Driver.h
    Access/DMA.h
    Access/Hypervisor.h
)

# Evasion sources
set(EVASION_SOURCES
    Evasion/Evasion.cpp
)

set(EVASION_HEADERS
    Evasion/Evasion.h
)

# Utils sources
set(UTILS_SOURCES
    Utils/IO.cpp
)

set(UTILS_HEADERS
    Utils/IO.h
)

# UI sources (optional)
set(UI_SOURCES
    UI/Overlay.cpp
)

set(UI_HEADERS
    UI/Overlay.h
    UI/ImGuiMenu.h
    UI/XAMLMenu.h
)

# Public headers
set(PUBLIC_HEADERS
    Include/MemoryLib.h
    Include/SimpleCheat.h
)

# Create library
add_library(WarlockLib STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${ACCESS_SOURCES}
    ${ACCESS_HEADERS}
    ${EVASION_SOURCES}
    ${EVASION_HEADERS}
    ${UTILS_SOURCES}
    ${UTILS_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${PUBLIC_HEADERS}
)

# Set target properties
set_target_properties(WarlockLib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(WarlockLib PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(WarlockLib PRIVATE
        psapi
        ntdll
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(WarlockLib PRIVATE /W4)
else()
    target_compile_options(WarlockLib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install rules
install(TARGETS WarlockLib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/WarlockLib
)

install(FILES ${CORE_HEADERS} DESTINATION include/WarlockLib/Core)
install(FILES ${ACCESS_HEADERS} DESTINATION include/WarlockLib/Access)
install(FILES ${EVASION_HEADERS} DESTINATION include/WarlockLib/Evasion)
install(FILES ${UTILS_HEADERS} DESTINATION include/WarlockLib/Utils)
install(FILES ${UI_HEADERS} DESTINATION include/WarlockLib/UI)

# Export package
export(TARGETS WarlockLib FILE WarlockLibConfig.cmake)
