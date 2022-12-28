cmake_minimum_required(VERSION 3.11)
project(glad)

add_library(glad "src/vulkan.c")
target_include_directories(glad PUBLIC "include/")
