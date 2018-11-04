cmake_minimum_required(VERSION 3.10)
project(LTBL2)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
#if(WIN32)
    set(SFML_ROOT "C:\\SFML")
#endif()
find_package(SFML COMPONENTS system window graphics)
include_directories(${SFML_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
set(SOURCES 
source/LightDirectionEmission.cpp
source/LightPointEmission.cpp
source/LightShape.cpp
source/LightSystem.cpp
source/Sprite.cpp)
add_library(LTBL2 ${SOURCES})
target_link_libraries(LTBL2 ${SFML_LIBRARIES})
