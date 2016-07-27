cmake_minimum_required(VERSION 3.3)
project(river_raid)

find_package(SDL REQUIRED)

add_executable(river_raid
	src/controller.cpp
	src/controller.hpp
	src/display.cpp
	src/display.hpp
	src/game.cpp
	src/game.hpp
	src/main.cpp
	src/utilities.cpp
	src/utilities.hpp)
target_link_libraries(river_raid ${SDL_LIBRARY})
target_compile_features(river_raid
PUBLIC
	cxx_lambdas
	cxx_auto_type
	cxx_override
	cxx_range_for
	cxx_right_angle_brackets
	cxx_static_assert)

file(GLOB ASSET_FILES "assets/*")
install(
	TARGETS river_raid
	RUNTIME DESTINATION river_raid)
install(
	FILES ${ASSET_FILES}
	DESTINATION river_raid)
