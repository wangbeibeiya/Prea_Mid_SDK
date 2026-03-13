cmake_minimum_required(VERSION 3.16)
project(SocketTestClient)

set(CMAKE_CXX_STANDARD 17)

# Qt5 Widgets
find_package(Qt5 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)

# JSON (use nlohmann from parent include/)
set(PARENT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(SocketTestClient
    main.cpp
    MainWindow.cpp
    MainWindow.h
    SearchableComboBox.cpp
    SearchableComboBox.h
    SocketClient.cpp
)

target_include_directories(SocketTestClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PARENT_INCLUDE}
)

target_link_libraries(SocketTestClient
    Qt5::Widgets
)

# Windows socket
if(WIN32)
    target_link_libraries(SocketTestClient ws2_32)
endif()

set_target_properties(SocketTestClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSVC)
    target_compile_options(SocketTestClient PRIVATE /utf-8)
endif()
