# This CMake script is designed for CLion IDE and QTCreator projects on Windows
# It should find bundled binary dependencies automatically when using MinGW compiler
# On Linux and Mac the dependencies (glm, glew, glfw) need to be installed manually

cmake_minimum_required(VERSION 3.5)
project(ppgso CXX)

#
# CONFIGURATION
#

# Basic CMake settings
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/_install)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Use add_resources function to convert .glsl to a .cpp files which will be compiled into a separate static library
include(add_resources)

#
# DEPENDENCIES
#

# Set up external dependencies for Windows users
if (MINGW)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/dependencies/include/")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/mingw")
elseif (MSVC)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/dependencies/include/")

    # Edit by: Samuel Zaprazny
    # EQUAL & GREATER_EQUAL instead of == & >=
    # == & >= was causing errors in the latest Visual Studio
    if(MSVC_VERSION EQUAL 1500)
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/vc2015")
    elseif(MSVC_VERSION GREATER_EQUAL 1900)
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/dependencies/lib/vc2019")
    endif()
    set(USE_STRICT_COMPILE_WARNINGS OFF CACHE BOOL "" FORCE)
endif ()

# Warnings for Debug mode
option(USE_STRICT_COMPILE_WARNINGS "Use strict compilation warnings in debug mode." ON)
# These compile flags should apply for GCC and Clang compilers
set(STRICT_COMPILE_FLAGS "-Wpedantic -Wall -Wno-c++98-compat -Wextra -Wno-sign-conversion -Wno-unused-parameter")
if (USE_STRICT_COMPILE_WARNINGS)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${STRICT_COMPILE_FLAGS}")
endif ()

# Find required packages
find_package(GLFW3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)

# Edit by: Samuel Zaprazny
# Finding ASSIMP
find_package(ASSIMP)

if (ASSIMP_FOUND)
    add_definitions(-DUSE_ASSIMP=1)

    if (WIN32 AND MINGW)
        set(ASSIMP_ROOT_DIR "${ASSIMP_ROOT_DIR}/bin/mingw/")
    elseif (WIN32 AND MSVC)
        set(ASSIMP_ROOT_DIR "${ASSIMP_ROOT_DIR}/bin/vc/")
    endif ()

    # Installing Assimp binaries to project binary directory if using WINDOWS
    if (WIN32)
        file(COPY ${ASSIMP_ROOT_DIR} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
        install(DIRECTORY ${ASSIMP_ROOT_DIR} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif ()

endif ()

# Optional packages
find_package(OpenMP)
if(OPENMP_FOUND)
    list(APPEND CMAKE_CXX_FLAGS ${OpenMP_CXX_FLAGS})
endif()

# Set default installation destination
if (NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "../_install")
endif ()

#
# Shaders / Materials
#

set(PPGSO_SHADER_SRC
        shader/phong_vert.glsl
        shader/phong_frag.glsl
        shader/instanced_vert.glsl
        shader/instanced_frag.glsl
        shader/particle_vert.glsl
        shader/particle_frag.glsl
        shader/shadow_depth_vert.glsl
        shader/shadow_depth_frag.glsl
        shader/skybox_vert.glsl
        shader/skybox_frag.glsl
        shader/postprocess/bloom_composite_frag.glsl
        shader/postprocess/blur_frag.glsl
        shader/postprocess/bright_extract_frag.glsl
        shader/postprocess/fullscreen_vert.glsl

)
add_resources(shaders ${PPGSO_SHADER_SRC})

# PPGSO library
if (ASSIMP_FOUND)
    message(STATUS "Using ASSIMP object loader")
    add_library(ppgso STATIC
            ppgso/Mesh_Assimp.cpp
            ppgso/tiny_obj_loader.cpp
            ppgso/shader.cpp
            ppgso/image.cpp
            ppgso/image_bmp.cpp
            ppgso/image_raw.cpp
            ppgso/texture.cpp
            ppgso/window.cpp
    )
else ()
    message(STATUS "Using TINY object loader")
    add_library(ppgso STATIC
            ppgso/Mesh_Tiny.cpp
            ppgso/tiny_obj_loader.cpp
            ppgso/shader.cpp
            ppgso/image.cpp
            ppgso/image_bmp.cpp
            ppgso/image_raw.cpp
            ppgso/texture.cpp
            ppgso/window.cpp
    )
endif ()

# Make sure GLM uses radians and GLEW is a static library
target_compile_definitions(ppgso PUBLIC -DGLM_FORCE_RADIANS -DGLEW_STATIC)

# Edit by: Samuel Zaprazny
# Linking assimp library
if (ASSIMP_FOUND)
    # Link to GLFW, GLEW. OpenGL and ASSIMP
    target_link_libraries(ppgso PUBLIC ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES})
    install(TARGETS ppgso DESTINATION .)
else ()
    # Link to GLFW, GLEW. OpenGL without ASSIMP
    # Using Tiny Obj Loader
    target_link_libraries(ppgso PUBLIC ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES})
    install(TARGETS ppgso DESTINATION .)
endif ()

# Pass on include directories
target_include_directories(ppgso PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${GLFW_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

#
# TARGETS
#

# project
add_executable(main
        src/main.cpp
        src/window/ClubbingWindow.cpp
        src/objects/Club.cpp
        src/scene/camera.cpp
        src/scene/Scene.cpp
        src/scene/Object.cpp
        src/objects/entrance/Door.cpp
        src/objects/entrance/SecurityCamera.cpp
        src/objects/inside/TableSet.cpp
        src/objects/inside/BarSet.cpp
        src/objects/djSet/DjSet.cpp
        src/objects/inside/TileGrid.cpp
        src/scene/Light.cpp
        src/objects/LightObjects/LightEmitter.cpp
        src/objects/inside/lamps/Lamp.cpp
        src/objects/lightObjects/LightBulb.cpp
        src/objects/inside/lamps/LavaLamp.cpp
        src/objects/inside/lamps/Buble.cpp
        src/objects/djSet/ConfettiCanon.cpp
        src/objects/djSet/ConfettiSystem.cpp
        src/objects/entrance/SkyBox.cpp
        src/objects/djSet/DiscoBall.cpp
        src/scene/KeyFrames.cpp
        src/scene/KeyFrames.h
)
target_link_libraries(main PUBLIC ppgso shaders winmm)

install(TARGETS main DESTINATION .)

add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data/ ${CMAKE_CURRENT_BINARY_DIR}
)
#
# INSTALLATION
#

file(COPY "data/" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(DIRECTORY data/ DESTINATION .)