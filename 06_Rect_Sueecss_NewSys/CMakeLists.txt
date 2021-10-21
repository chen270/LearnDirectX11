CMAKE_MINIMUM_REQUIRED(VERSION 3.8.0)


PROJECT(testDemo)
SET(TARGET "testDemo")

add_compile_definitions(UNICODE _UNICODE)

IF(MSVC)
set(CMAKE_EXE_LINKER_FLAGS"${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

    IF (WIN7_SYSTEM_SUPPORT MATCHES ON)
	    add_compile_definitions(_WIN32_WINNT=0x601)
    ENDIF()
ENDIF(MSVC)

SET(INSTALL_PATH "${PROJECT_SOURCE_DIR}/bin")

LINK_DIRECTORIES("${PROJECT_SOURCE_DIR}/bin") #指定链接路径
# SET(CMAKE_CXX_FLAGS "-std=c++11 -lmpv-1")

# add_definitions("-I<path to directx headers>")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")
# INCLUDE_DIRECTORIES("C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um")


FILE(GLOB SRC_LIST "./*.cpp")
# file(GLOB  SRC_LIST "./protoc/*.cc")
SOURCE_GROUP("source" FILES ${SRC_LIST})


FILE(GLOB  HEAD_LIST "./*.h")
SOURCE_GROUP("include" FILES ${HEAD_LIST})


SET(ALL_SOURCES
${SRC_LIST}
${HEAD_LIST}
)


SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${INSTALL_PATH})


# ADD_LIBRARY(${TARGET} ${ALL_SOURCES})
ADD_EXECUTABLE(${TARGET} ${ALL_SOURCES}) #只有在 add_executable() 中的文件才会被 source_group 分组


TARGET_LINK_LIBRARIES(${TARGET} "d3d11.lib" "dxgi.lib" "dxguid.lib" "D3DCompiler.lib" "winmm.lib")
# TARGET_LINK_LIBRARIES(${TARGET} "${PROJECT_SOURCE_DIR}/protoc/libprotobufd.lib")