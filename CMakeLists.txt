cmake_minimum_required(VERSION 3.20)
project(CYK_Parser)

add_compile_options(-Wall -Wextra -Wpedantic)

# Uncomment to print out generated CNF and tables
# add_compile_definitions(DEBUG)

add_library(${PROJECT_NAME} SHARED src/CYK_Parser.cpp src/Grammar.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)


add_executable(CYK_Tests tests/test.cpp)
target_link_libraries(CYK_Tests ${PROJECT_NAME})