cmake_minimum_required(VERSION 3.16)

project(CarouselViewer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/glfw)

include_directories(include)
include_directories(external/glfw/include)
include_directories(external/glad/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(assimp CONFIG REQUIRED)

add_executable(CarouselViewer ${SOURCES})

target_include_directories(CarouselViewer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glm
)

target_link_libraries(CarouselViewer PRIVATE
    assimp::assimp
    glfw
    glad
    ${CMAKE_DL_LIBS}
)