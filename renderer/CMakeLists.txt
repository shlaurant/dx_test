project(directx_renderer)

file(GLOB_RECURSE SRC_FILES *.cpp CMAKE_CONFIGURE_DEPENDS)
add_library(${PROJECT_NAME} ${SRC_FILES})

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/includes ${CMAKE_SOURCE_DIR}/includes/directx_renderer)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_directories(${PROJECT_NAME} PUBLIC C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.19041.0/um/x64)

target_link_libraries(${PROJECT_NAME} d3dcompiler d3d12 dxgi dxguid)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME} DirectXTex_d debug/libfbxsdk-md debug/libxml2-md debug/zlib-md)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(${PROJECT_NAME} DirectXTex release/libfbxsdk-md release/libxml2-md release/zlib-md)
endif ()

add_definitions(-DUNICODE)