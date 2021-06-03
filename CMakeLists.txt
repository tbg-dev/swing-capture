cmake_minimum_required(VERSION 3.15.0)

project(tbg-swing-capture)

# -----------------------------------------------------------------------------
# Common build setup
# -----------------------------------------------------------------------------
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/build/cmake
)
include(BuildCommon)

# -----------------------------------------------------------------------------
# Add all the things
# -----------------------------------------------------------------------------
add_libs(PREFIX tbg LIBS gstreamer)
add_tests()
add_apps(APPS swing-capture)