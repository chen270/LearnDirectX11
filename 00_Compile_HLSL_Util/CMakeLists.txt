CMAKE_MINIMUM_REQUIRED(VERSION 3.8.0)

PROJECT(Build_HLSL)
SET(TARGET "Build_HLSL")
SET(INSTALL_PATH "${PROJECT_SOURCE_DIR}/out")

IF(NOT MSVC)
    message(FATAL_ERROR "You are not in MSVC compile environment, can not compile hlsl, CMake exit.")
ENDIF()

add_compile_definitions(UNICODE _UNICODE)

FILE(GLOB SRC_LIST "./*.*")

foreach(v ${SRC_LIST})
    STRING(REGEX MATCH ".*ps.hlsl" prjNamePS ${v})

    IF(NOT(prjNamePS STREQUAL ""))
        message("----${v} set PROPERTIES - Pixel")

        # set_source_files_properties(${v} PROPERTIES VS_SHADER_TYPE Pixel)
        set_source_files_properties(${v} PROPERTIES ShaderModel "5_0")
        set_property(SOURCE ${v} PROPERTY VS_SHADER_TYPE Pixel)
        set_property(SOURCE ${v} PROPERTY VS_SHADER_ENTRYPOINT main)
        # set_property(SOURCE ${v} PROPERTY VS_SHADER_MODEL "5_0")
        set_property(SOURCE ${v} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
        set_property(SOURCE ${v} PROPERTY VS_SHADER_VARIABLE_NAME "kSubPixelShaderCode") # 生成变量名
        set_property(SOURCE ${v} PROPERTY VS_SHADER_OUTPUT_HEADER_FILE "$(OutDir)%(Filename).h")
    ENDIF()

    STRING(REGEX MATCH ".*vs.hlsl" prjNameVS ${v})

    IF(NOT(prjNameVS STREQUAL ""))
        message("----${v} set PROPERTIES - Vertex")

        # set_source_files_properties(${v} PROPERTIES VS_SHADER_TYPE Vertex)
        set_source_files_properties(${v} PROPERTIES ShaderModel "5_0")
        set_property(SOURCE ${v} PROPERTY VS_SHADER_TYPE Vertex)
        set_property(SOURCE ${v} PROPERTY VS_SHADER_ENTRYPOINT main)
        # set_property(SOURCE ${v} PROPERTY VS_SHADER_MODEL "5_0")
        set_property(SOURCE ${v} PROPERTY VS_SHADER_FLAGS $<1:/DFLAGS_ADDED>)
        set_property(SOURCE ${v} PROPERTY VS_SHADER_VARIABLE_NAME "kSubVertexShaderCode") # 生成变量名
        set_property(SOURCE ${v} PROPERTY VS_SHADER_OUTPUT_HEADER_FILE "$(OutDir)%(Filename).h")
    ENDIF()
endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${INSTALL_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${INSTALL_PATH})

# Set the startup project .
IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.6.0")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
ENDIF()

ADD_EXECUTABLE(${TARGET} ${SRC_LIST})

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${INSTALL_PATH}")
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${INSTALL_PATH}")