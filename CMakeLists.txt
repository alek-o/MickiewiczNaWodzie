cmake_minimum_required(VERSION 3.10)
project(MickiewiczNaWodzie)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

# === External Dependecies ===

# GLFW
add_subdirectory("vendor/glfw")

# GLM
add_subdirectory("vendor/glm")

# Assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_DRACO_DECODER OFF)
set(ASSIMP_BUILD_ZLIB ON)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory("vendor/assimp")

# === Project Source Files ===
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.c")


add_executable(MickiewiczNaWodzie ${SOURCES})

# === Include Paths ===
target_include_directories(MickiewiczNaWodzie PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLAD"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/"
)

# stb_image

set(STB_IMAGE_SRC "vendor/stb_image/stb_image.cpp")

add_library(stb_image STATIC ${STB_IMAGE_SRC})
set_target_properties(stb_image PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/vendor/stb_image"
)

# === Copy Resources ===
add_custom_command(
    TARGET MickiewiczNaWodzie POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:MickiewiczNaWodzie>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            "${CMAKE_SOURCE_DIR}/resources" 
            "$<TARGET_FILE_DIR:MickiewiczNaWodzie>/resources"
    COMMENT "Copying resources to executable directory"
)

# === Linking ===
target_link_libraries(MickiewiczNaWodzie PUBLIC
	glfw
	stb_image
	assimp
)

find_package(OpenGL REQUIRED)
target_link_libraries(MickiewiczNaWodzie PRIVATE OpenGL::GL)