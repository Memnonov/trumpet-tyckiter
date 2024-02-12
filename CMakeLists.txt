cmake_minimum_required(VERSION 3.28.3)

project(trumpet_tyckiter)

# INCLUDE(FindPkgConfig)
# PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
# PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_image SDL2_mixer)

# Include these directories?
include_directories(
  ${PROJECT_SOURCE_DIR}/src
  )

file(GLOB SOURCES
  "${PROJECT_SOURCE_DIR}/src/*.c"
  "./src/*.c"
  )

# This adds a (target) executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link necessary libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${SDL2_CFLAGS_OTHER})
