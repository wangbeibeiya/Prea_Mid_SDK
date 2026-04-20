cmake_minimum_required(VERSION 3.16)

add_executable(StpVolumeRenameTest main.cpp)

target_include_directories(StpVolumeRenameTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PERA_CFD_SDK_DIR}/include
    ${THIRD_PARTY_SDK_DIR}/include
)

target_link_libraries(StpVolumeRenameTest
    debug "${PERA_CFD_SDK_DIR}/lib/PFBase.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFGeometry.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFMesh.lib"
    debug "${PERA_CFD_SDK_DIR}/lib/PFPostProcess.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFBase.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFGeometry.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFMesh.lib"
    optimized "${PERA_CFD_SDK_DIR}/lib/PFPostProcess.lib"
)

if(MAPPING_GEOMETRY_NEEDS_QT5_CORE)
    target_link_libraries(StpVolumeRenameTest Qt5::Core)
    target_compile_definitions(StpVolumeRenameTest PRIVATE MAPPING_GEOMETRY_HAS_QT5_CORE)
endif()

set_target_properties(StpVolumeRenameTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSVC)
    target_compile_options(StpVolumeRenameTest PRIVATE /utf-8)
endif()

if(MSVC)
    function(stp_test_copy_dll dll_name)
        set(dll_path "${PERA_CFD_SDK_DIR}/bin/${dll_name}")
        if(EXISTS "${dll_path}")
            add_custom_command(TARGET StpVolumeRenameTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll_path}"
                "$<TARGET_FILE_DIR:StpVolumeRenameTest>"
                COMMENT "Copying ${dll_name} for StpVolumeRenameTest..."
            )
        endif()
    endfunction()
    stp_test_copy_dll("PFBase.dll")
    stp_test_copy_dll("PFGeometry.dll")
    stp_test_copy_dll("PFMesh.dll")
    stp_test_copy_dll("PFPostProcess.dll")
    stp_test_copy_dll("jsoncpp.dll")
    stp_test_copy_dll("LicenseDll.dll")
    stp_test_copy_dll("pthreads.dll")
    stp_test_copy_dll("xyssl.dll")
    stp_test_copy_dll("zlib.dll")
    stp_test_copy_dll("Qt5Core.dll")
    stp_test_copy_dll("fluidModel.dll")
    stp_test_copy_dll("model_database.dll")
    stp_test_copy_dll("openNURBS3.dll")
endif()
