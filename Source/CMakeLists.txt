add_library(Loris STATIC)

file(
GLOB_RECURSE CppSourceFiles CONFIGURE_DEPENDS
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(
GLOB_RECURSE HeaderSourceFiles CONFIGURE_DEPENDS
"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

target_sources(Loris PRIVATE ${CppSourceFiles})

target_sources(Loris PUBLIC ${HeaderSourceFiles})

target_include_directories(Loris PUBLIC "${CMAKE_CURRENT_LIST_DIR}")