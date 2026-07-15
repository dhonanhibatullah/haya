cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME "haya")
set(PROJECT_VERSION "v1.0.0.1-development")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(haya)
