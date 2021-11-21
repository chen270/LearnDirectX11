CMAKE_MINIMUM_REQUIRED(VERSION 3.8.0)


PROJECT(CompileShader)
SET(TARGET "CompileShader")

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

FILE(GLOB  SHADER_LIST "./shader/*.*")
SOURCE_GROUP("shader" FILES ${SHADER_LIST})

FILE(GLOB  MISC_LIST "./misc/*.*")
SOURCE_GROUP("misc" FILES ${MISC_LIST})

# FILE(GLOB  EXCEP_LIST "./misc/exception/*.*")
# SOURCE_GROUP("misc" FILES ${EXCEP_LIST})

# 设置着色器类型
foreach(v ${SHADER_LIST})
    STRING( REGEX MATCH  ".*ps.hlsl" prjNamePS ${v} )
    STRING( REGEX MATCH  ".*Ps.hlsl" prjNamePS2 ${v})
    STRING( REGEX MATCH  ".*PS.hlsl" prjNamePS3 ${v})
    IF(NOT(prjNamePS STREQUAL "") OR NOT(prjNamePS2 STREQUAL "") OR NOT(prjNamePS3 STREQUAL ""))
        message("--${v} set PROPERTIES - Pixel")
        set_source_files_properties(${v} PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 4.0)
    ENDIF()

    STRING( REGEX MATCH  ".*vs.hlsl" prjNameVS ${v} )
    STRING( REGEX MATCH  ".*Vs.hlsl" prjNameVS2 ${v} )
    STRING( REGEX MATCH  ".*VS.hlsl" prjNameVS3 ${v} )
    IF(NOT(prjNameVS STREQUAL "") OR NOT(prjNameVS2 STREQUAL "") OR NOT(prjNameVS3 STREQUAL ""))
        message("--${v} set PROPERTIES - Vertex")
        set_source_files_properties(${v} PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 4.0)
    ENDIF()
endforeach()

SET(ALL_SOURCES
${SRC_LIST}
${HEAD_LIST}
${SHADER_LIST}
${MISC_LIST}
# ${EXCEP_LIST}
)


SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${INSTALL_PATH})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${INSTALL_PATH})


# ADD_LIBRARY(${TARGET} ${ALL_SOURCES})
ADD_EXECUTABLE(${TARGET} ${ALL_SOURCES}) #只有在 add_executable() 中的文件才会被 source_group 分组


TARGET_LINK_LIBRARIES(${TARGET} "d3d11.lib" "dxgi.lib" "dxguid.lib" "D3DCompiler.lib" "winmm.lib")
# TARGET_LINK_LIBRARIES(${TARGET} "${PROJECT_SOURCE_DIR}/protoc/libprotobufd.lib")