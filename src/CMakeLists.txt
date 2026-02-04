# Source files
set(SOURCES
    Logger.cpp
    ModelProcessor.cpp
    GeometryProcessor.cpp
    MeshProcessor.cpp
    main.cpp
    RenderProcessor.cpp
    MeshVisualizationServer.cpp
    commonMethods.cpp
    GeometryAPI.cpp
    GeometryDataStructures.cpp
    ProjectModelData.cpp
)

# Header files (for IDE visibility only; not required for compilation)
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/include/*.hpp
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
    ${CMAKE_SOURCE_DIR}/demo/*.h
    ${CMAKE_SOURCE_DIR}/demo/*.hpp
)

# Create main executable with all source files (only one project)
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${PROJECT_HEADERS}
)

# 添加 JSON 库支持 (nlohmann/json)
# 使用单头文件版本（json.hpp 已在 include 目录中）
# 添加 include 目录到包含路径，这样可以直接 #include <json.hpp>
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
message(STATUS "Using nlohmann/json single-header version from include/json.hpp")

# Set source file properties for debugging
# Note: -g flag is for GCC, MSVC uses /Zi which is set below

# Link Pera CFD SDK libraries
target_link_libraries(${PROJECT_NAME} 
    debug "${PERA_CFD_SDK_DIR}/lib/PFBase.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFGeometry.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFMesh.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFPostProcess.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFBase.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFGeometry.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFMesh.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFPostProcess.lib"
)

# Link VTK libraries
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

# Link third-party SDK libraries (adjust library filename as needed)
# target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_SDK_DIR}/lib/your_sdk_library.lib)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy SDK DLLs to output directory
if(MSVC)
    # Function to copy DLL if it exists
    function(copy_dll_if_exists dll_name)
        set(dll_path "${PERA_CFD_SDK_DIR}/bin/${dll_name}")
        if(EXISTS "${dll_path}")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll_path}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                COMMENT "Copying ${dll_name}..."
            )
        else()
            message(STATUS "Warning: ${dll_name} not found at ${dll_path}, skipping...")
        endif()
    endfunction()
    
    # Main SDK DLLs
    copy_dll_if_exists("PFBase.dll")
    copy_dll_if_exists("PFGeometry.dll")
    copy_dll_if_exists("PFMesh.dll")
    copy_dll_if_exists("PFPostProcess.dll")
    
    # Dependencies
    copy_dll_if_exists("jsoncpp.dll")
    copy_dll_if_exists("LicenseDll.dll")
    copy_dll_if_exists("pthreads.dll")
    copy_dll_if_exists("xyssl.dll")
    copy_dll_if_exists("zlib.dll")
    copy_dll_if_exists("Qt5Core.dll")
    copy_dll_if_exists("fluidModel.dll")
    copy_dll_if_exists("model_database.dll")
    copy_dll_if_exists("openNURBS3.dll")
endif()

# Debug test properties removed

# Set UTF-8 encoding for MSVC
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# Set debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_MODE)

    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zi /Od /FS)
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG:FULL)
    endif()
endif() 