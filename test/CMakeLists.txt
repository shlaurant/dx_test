project(renderer_test)

file(GLOB_RECURSE SRC_FILES src/*.cpp CMAKE_CONFIGURE_DEPENDS)

link_libraries(directx_renderer)
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/includes ${CMAKE_SOURCE_DIR}/includes/test)

add_executable(default_test WIN32 default_test.cpp ${SRC_FILES})
add_executable(memleak_test WIN32 memleak_test.cpp ${SRC_FILES})


add_definitions(-DUNICODE)