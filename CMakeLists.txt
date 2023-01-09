cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)

project(dx_test VERSION 0.0.1)

file(GLOB_RECURSE SRC_FILES *.cpp CMAKE_CONFIGURE_DEPENDS)
add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})
target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.h)

target_link_libraries(${PROJECT_NAME} d3dcompiler d3d12 dxgi dxguid DirectXTex_d)
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/windows/libs)
target_link_directories(${PROJECT_NAME} PUBLIC C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.19041.0/um/x64)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_definitions(-DUNICODE)