project(renderer_test)

file(GLOB_RECURSE SRC_FILES *.cpp CMAKE_CONFIGURE_DEPENDS)
add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} directx_renderer)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/includes)

add_definitions(-DUNICODE)