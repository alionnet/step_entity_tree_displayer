cmake_minimum_required(VERSION 3.12)

project(step_entity_tree_viewer)

add_executable(setv "code/main.cpp")
target_compile_definitions(setv PUBLIC "-DDEBUG")