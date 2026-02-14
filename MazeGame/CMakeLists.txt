# CMakeList.txt : CMake project for MazeGame, include source and define
# project specific logic here.
#
find_package(SFML 3 REQUIRED COMPONENTS Graphics System Window Audio Network)

# Add source to this project's executable.
add_executable (MazeGame "MazeGame.cpp" "MazeGame.h" "Menu.cpp" "Menu.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MazeGame PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(MazeGame PRIVATE SFML::Graphics SFML::System SFML::Window SFML::Audio SFML::Network)
# TODO: Add tests and install targets if needed.
