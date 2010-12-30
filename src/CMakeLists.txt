cmake_minimum_required(VERSION 2.6)

function(add_public_header oldname newname)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${oldname}"
    "${CMAKE_BINARY_DIR}/include/qtsparkle/${newname}"
    COPY_ONLY)
endfunction(add_public_header)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES
  appcast.cpp
  compareversions.cpp
  followredirects.cpp
  uicontroller.cpp
  updatechecker.cpp
  updatedialog.cpp
  updater.cpp
)

set(MOC_SOURCES
  followredirects.h
  uicontroller.h
  updatechecker.h
  updatedialog.h
  updater.h
)

set(UI_SOURCES
  updatedialog.ui
)

add_public_header(updater.h Updater)

qt4_wrap_cpp(MOC ${MOC_SOURCES})
qt4_wrap_ui(UIC ${UI_SOURCES})

add_library(qtsparkle SHARED
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(qtsparkle
  ${QT_LIBRARIES}
)
