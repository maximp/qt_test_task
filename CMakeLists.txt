cmake_minimum_required(VERSION 3.0.2)

#project name
project(qt-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wpedantic)
add_compile_options(-Wfatal-errors)
add_compile_options(-pipe)
add_compile_options(-fPIC)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG -march=native -mtune=native")

if(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
elseif(WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(UNIX)

find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

#settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#build file list
file(GLOB HPP_FILES "${CMAKE_CURRENT_LIST_DIR}/*.hpp")
file(GLOB CPP_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
set(ALL_FILES ${HPP_FILES} ${CPP_FILES} ${MOC_LIST})

add_executable(${PROJECT_NAME} ${ALL_FILES})

#linking
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

