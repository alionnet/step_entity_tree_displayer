cmake_minimum_required(VERSION 3.12)

project(step_entity_tree_displayer)

add_executable(setd "code/main.cpp")
target_compile_definitions(setd PUBLIC "-DDEBUG")