cmake_minimum_required(VERSION 3.0)
project(FastNoiseLite)

add_library(${PROJECT_NAME} include/FastNoiseLite.h src/FastNoiseLite.c)
target_include_directories(${PROJECT_NAME} PUBLIC include/)