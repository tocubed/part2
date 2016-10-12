cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(part2 LANGUAGES CXX)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE 
	cxx_auto_type cxx_alias_templates cxx_return_type_deduction)

# Add source and header files
file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES src/*.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Add required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.4 REQUIRED system window graphics)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_DEPENDENCIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIR})
