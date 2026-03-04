cmake_minimum_required(VERSION 3.16)
project(PhysicsEngine2D C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_EDITOR "Build DWrite interactive editor app" ON)
option(BUILD_SANDBOX "Deprecated alias for BUILD_EDITOR" ON)
option(BUILD_TESTS "Build regression test executable" OFF)
if(DEFINED BUILD_SANDBOX)
    set(BUILD_EDITOR ${BUILD_SANDBOX})
endif()

set(PHYSICS2D_SOURCES
    src/math.c
    src/shape.c
    src/body.c
    src/constraint.c
    src/collision.c
    src/physics.c
)

add_library(physics2d STATIC ${PHYSICS2D_SOURCES})
add_library(physics_core ALIAS physics2d)
target_include_directories(physics2d PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(NOT MSVC)
    target_link_libraries(physics2d PUBLIC m)
endif()

if(MSVC)
    target_compile_options(physics2d PRIVATE /W4)
else()
    target_compile_options(physics2d PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(BUILD_EDITOR)
    set(SANDBOX_SOURCES
        apps/sandbox_dwrite/main.c
    )
    if(WIN32)
        list(APPEND SANDBOX_SOURCES apps/sandbox_dwrite/app_icon.rc)
    endif()

    add_executable(physics_sandbox WIN32 ${SANDBOX_SOURCES})
    add_executable(physics_editor ALIAS physics_sandbox)
    target_link_libraries(physics_sandbox PRIVATE physics2d d2d1 dwrite windowscodecs ole32 uuid shcore gdi32 user32)
    if(MSVC)
        target_compile_options(physics_sandbox PRIVATE /W4)
    else()
        target_compile_options(physics_sandbox PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_sandbox PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

if(BUILD_TESTS)
    add_executable(physics_tests tests/regression_tests.c)
    target_link_libraries(physics_tests PRIVATE physics2d)
    if(MSVC)
        target_compile_options(physics_tests PRIVATE /W4)
    else()
        target_compile_options(physics_tests PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

set_target_properties(physics2d PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
