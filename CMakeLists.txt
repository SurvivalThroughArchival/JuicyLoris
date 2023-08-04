cmake_minimum_required (VERSION 3.17 FATAL_ERROR)
set (CMAKE_CXX_STANDARD 17)
project(JuicyLorry
VERSION 0.0.0
LANGUAGES CXX
DESCRIPTION "Spectral Processing Library")

add_subdirectory(Source)

add_executable(HelloLorris)

target_sources(HelloLorris PRIVATE main.cpp)

#target_compile_options(AudioPluginExample PRIVATE -Wall)
#target_compile_options(AudioPluginExample PRIVATE -Werror -Wextra)

target_link_libraries(HelloLorris PRIVATE Loris)

set_target_properties(
    HelloLorris PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
)
