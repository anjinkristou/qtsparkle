cmake_minimum_required(VERSION 2.6)

include(cmake/Translations.cmake)

find_package(Qt4 REQUIRED QtCore QtGui QtNetwork QtXml)
add_definitions(${QT_DEFINITIONS})
include(${QT_USE_FILE})

SET(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -Wall")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Produce a .dll or .so")

add_subdirectory(src)
add_subdirectory(exampleapp)
