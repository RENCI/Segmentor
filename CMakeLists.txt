cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

project(Segmentor)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(NOT(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4"))
  message(FATAL_ERROR "Requires VTK version >= 6.0 and Qt version >= 4.0")
endif()

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets REQUIRED QUIET)

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

file(GLOB UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qt/*.ui)
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

# Create executable
qt5_wrap_ui(UISrcs ${UI_FILES} )
# CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
add_executable(Segmentor MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${QT_WRAP} ${UI_RESOURCES})
qt5_use_modules(Segmentor Core Gui)
target_link_libraries(Segmentor ${VTK_LIBRARIES})
install(TARGETS Segmentor RUNTIME DESTINATION bin BUNDLE DESTINATION .)

# For visual studio debugging
set(DEBUG_PATH "${VTK_DIR}/bin/Debug;PATH=%PATH%")
set_target_properties(Segmentor PROPERTIES VS_DEBUGGER_ENVIRONMENT "${DEBUG_PATH}")

# CPack
if(WIN32)
	set(CPACK_GENERATOR NSIS)
elseif(APPLE)
	set(CPACK_GENERATOR DragNDrop)
endif()

set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Installer)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Segmentor")
set(CPACK_PACKAGE_EXECUTABLES Segmentor;Segmentor)

include(CPack)