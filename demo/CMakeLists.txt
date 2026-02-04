cmake_minimum_required(VERSION 3.8.0)

set(SDK_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(COMMON_CMAKE_FILES_DIR "${DEMO_DIR}/cmake")


project(SDK VERSION 1.0.0 LANGUAGES C CXX)

# global settings and functions
# set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

# set default properties: output directories and ect.
set(PFSDK_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
function(SetDefaultTargetProperties target)
	set_target_properties(${target} PROPERTIES
		CXX_STANDARD 14
	)

	set_target_properties(${target} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PFSDK_LIB_DIR}"
		LIBRARY_OUTPUT_DIRECTORY "${PFSDK_LIB_DIR}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	)
endfunction()


set(VTK_DIR "D:/ws/3rdparty/vtk8.2.0")

message("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi  /Od") 
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

include("${COMMON_CMAKE_FILES_DIR}/vtk_headers.cmake")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}") 

set(CURRENT_TARGET_NAME demo)
add_executable(${CURRENT_TARGET_NAME} "")

include_directories("${SDK_ROOT_DIR}/include")

file(GLOB ROOT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB_RECURSE POST_PROCESS_DEMO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/postProcessDemoImpl/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/postProcessDemoImpl/*.cpp
)

set(ALL_SOURCES ${ROOT_SOURCES} ${POST_PROCESS_DEMO_SOURCE})
list(REMOVE_DUPLICATES ALL_SOURCES)

target_sources(${CURRENT_TARGET_NAME} PRIVATE ${ALL_SOURCES})

set_target_properties(${CURRENT_TARGET_NAME} PROPERTIES
    VS_DEBUGGER_COMMAND "${SDK_ROOT_DIR}/bin/${CURRENT_TARGET_NAME}.exe"
)

include("${COMMON_CMAKE_FILES_DIR}/vtk_libs.cmake")
IF (MSVC)
	target_link_libraries(${CURRENT_TARGET_NAME} debug "${SDK_ROOT_DIR}/lib/PFBase.lib")
	target_link_libraries(${CURRENT_TARGET_NAME} debug "${SDK_ROOT_DIR}/lib/PFGeometry.lib")
    target_link_libraries(${CURRENT_TARGET_NAME} debug "${SDK_ROOT_DIR}/lib/PFMesh.lib")
    target_link_libraries(${CURRENT_TARGET_NAME} debug "${SDK_ROOT_DIR}/lib/PFPostProcess.lib")
	target_link_libraries(${CURRENT_TARGET_NAME} debug DbgHelp.lib)

	target_link_libraries(${CURRENT_TARGET_NAME} optimized "${SDK_ROOT_DIR}/lib/PFBase.lib")
	target_link_libraries(${CURRENT_TARGET_NAME} optimized "${SDK_ROOT_DIR}/lib/PFGeometry.lib")
    target_link_libraries(${CURRENT_TARGET_NAME} optimized "${SDK_ROOT_DIR}/lib/PFMesh.lib")
    target_link_libraries(${CURRENT_TARGET_NAME} optimized "${SDK_ROOT_DIR}/lib/PFPostProcess.lib")
	target_link_libraries(${CURRENT_TARGET_NAME} general opengl32.lib)
ENDIF()

add_custom_command(
    TARGET ${CURRENT_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/$(Configuration)/${CURRENT_TARGET_NAME}.exe" "${SDK_ROOT_DIR}/bin/${CURRENT_TARGET_NAME}.exe"
)

SetDefaultTargetProperties(${CURRENT_TARGET_NAME})