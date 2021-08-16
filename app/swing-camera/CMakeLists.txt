cmake_minimum_required (VERSION 2.8)
project (swing-camera)

file(GLOB_RECURSE SRC_CPP *.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME}
    ${SRC_CPP}
)

target_link_libraries(${PROJECT_NAME}
    ${CONAN_LIBS}
    ${BUILT_LIBS}
)