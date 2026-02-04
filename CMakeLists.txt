cmake_minimum_required(VERSION 3.16)
project(MappingGeometry)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Pera CFD SDK paths
set(PERA_CFD_SDK_DIR "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20260128")
set(THIRD_PARTY_SDK_DIR "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20260128")

# Find VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PERA_CFD_SDK_DIR}/include
    ${THIRD_PARTY_SDK_DIR}/include
)

# Library directories
link_directories(
    ${PERA_CFD_SDK_DIR}/lib
    ${THIRD_PARTY_SDK_DIR}/lib
)

# Print configuration information
message(STATUS "Pera CFD SDK path: ${PERA_CFD_SDK_DIR}")
message(STATUS "Third-party SDK path: ${THIRD_PARTY_SDK_DIR}")
message(STATUS "VTK found: ${VTK_FOUND}")
message(STATUS "VTK version: ${VTK_VERSION}")

# Add subdirectories
add_subdirectory(src)
# add_subdirectory(tests)  # 注释掉测试项目 