project (tbg_gstreamer)

include_directories(../../include)
file(GLOB_RECURSE SRC_CPP *.cpp)

add_library(${PROJECT_NAME}
    ${SRC_CPP}
)

message(STATUS "CONAN_LIBS=${CONAN_LIBS}")
target_link_libraries(${PROJECT_NAME}
    ${CONAN_LIBS}
)