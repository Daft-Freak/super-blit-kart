# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.9...3.26)

# Replace "game" with a name for your project (this is used the name of the output)
project(blit-kart)

# Add your sources here (adding headers is optional, but helps some CMake generators)
set(PROJECT_SOURCE
  game.cpp
  kart.cpp
  kart-select.cpp
  main-menu.cpp
  menu.cpp
  minimap.cpp
  race.cpp
  sprite3d.cpp
  track.cpp
  track-select.cpp
)

# ... and any other files you want in the release here
set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk)

# track info generation
add_custom_command(COMMAND
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/extract_track_info.py "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
  OUTPUT track-info.cpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/extract_track_info.py ${CMAKE_CURRENT_BINARY_DIR}/assets.hpp
)
list(APPEND PROJECT_SOURCE track-info.cpp)

blit_executable (${PROJECT_NAME} ${PROJECT_SOURCE})
blit_assets_yaml (${PROJECT_NAME} assets.yml)
blit_metadata (${PROJECT_NAME} metadata.yml)
add_custom_target (flash DEPENDS ${PROJECT_NAME}.flash)

# setup release packages
install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)
