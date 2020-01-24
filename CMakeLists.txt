cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

PROJECT(Segmentor)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  # Instruct CMake to run moc automatically when needed.
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  find_package(Qt5Widgets REQUIRED QUIET)
  find_package(Qt5Svg REQUIRED)
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/interaction
  ${CMAKE_CURRENT_SOURCE_DIR}/qt
  ${CMAKE_CURRENT_SOURCE_DIR}/region
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/visualization
  ${CMAKE_CURRENT_SOURCE_DIR}/vtk
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/interaction/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/qt/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/region/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/visualization/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/vtk/*.h
)
file(GLOB CXX_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/interaction/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/qt/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/region/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/visualization/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vtk/*.cxx
)

set(UI_RESOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Segmentor.qrc 
  ${CMAKE_CURRENT_SOURCE_DIR}/Segmentor.rc
)

if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  qt5_wrap_ui(UISrcs ${UI_FILES} )
  # CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
  add_executable(Segmentor MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${QT_WRAP} ${UI_RESOURCES})
  qt5_use_modules(Segmentor Core Gui Svg)
  target_link_libraries(Segmentor ${VTK_LIBRARIES})
else()
  QT4_WRAP_UI(UISrcs ${UI_FILES})
  QT4_WRAP_CPP(MOCSrcs ${QT_WRAP})
  add_executable(Segmentor MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${MOCSrcs} ${UI_RESOURCES})
  target_link_libraries(Segmentor ${VTK_LIBRARIES})
endif()

set(DEBUG_PATH "${VTK_DIR}/bin/Debug;PATH=%PATH%")
set_target_properties(Segmentor PROPERTIES VS_DEBUGGER_ENVIRONMENT "${DEBUG_PATH}")
