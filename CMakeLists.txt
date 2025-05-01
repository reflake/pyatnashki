cmake_minimum_required(VERSION 3.10)

# Set the project name
project(pyatnashki VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(pyatnashki
	main.cpp
	field.cpp
	shuffler.cpp
)

# Include directories
target_include_directories(pyatnashki PRIVATE include)

# Enable warnings
if (MSVC)
	target_compile_options(pyatnashki PRIVATE /W3)
else()
	target_compile_options(pyatnashki PRIVATE -Wall -Wextra -pedantic)
endif()

# Find SDL2 package
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# Link SDL2 library
target_link_libraries(pyatnashki 
	PRIVATE 
		SDL3::SDL3
		SDL3_image
		SDL3_ttf
	)

file(COPY font/RobotoCondensed-Light.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/font)