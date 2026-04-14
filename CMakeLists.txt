cmake_minimum_required(VERSION 3.20)
project(HallReverb_NOGUI LANGUAGES CXX)

add_executable(HallReverb_NOGUI)
target_compile_features(HallReverb_NOGUI PRIVATE cxx_std_17)
target_compile_definitions(HallReverb_NOGUI PRIVATE LIBFV3_FLOAT)

target_include_directories(HallReverb_NOGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Freeverb3
)

add_subdirectory(Source)
add_subdirectory(Libs/Freeverb3)
