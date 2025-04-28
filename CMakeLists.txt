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
	target_compile_options(pyatnashki PRIVATE /W4)
else()
	target_compile_options(pyatnashki PRIVATE -Wall -Wextra -pedantic)
endif()

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Link SDL2 library
target_link_libraries(pyatnashki 
	PRIVATE 
		SDL2::SDL2-static
		SDL2_image
		SDL2_ttf
	)