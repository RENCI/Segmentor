cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

if(APPLE)
#  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13.0" CACHE STRING "Minimum OS X deployment version")
endif()

project(Segmentor)

if(WIN32)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Install)
else()
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Install)

  #set(CMAKE_SKIP_BUILD_RPATH FALSE)
  #set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  #list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  #if("${isSystemDir}" STREQUAL "-1")
  #  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  #endif("${isSystemDir}" STREQUAL "-1")

  #set(CMAKE_SKIP_RPATH TRUE)
	
endif()


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
install(TARGETS Segmentor 
  RUNTIME DESTINATION bin COMPONENT Segmentor
  BUNDLE DESTINATION . COMPONENT Segmentor
)

if(WIN32)
  # For visual studio debugging
  set(DEBUG_PATH "${VTK_DIR}/bin/Debug;PATH=%PATH%")
  set_target_properties(Segmentor PROPERTIES VS_DEBUGGER_ENVIRONMENT "${DEBUG_PATH}")

  set(plugin_dest_dir bin)
  set(qtconf_dest_dir bin)
  set(APPS \${CMAKE_INSTALL_PREFIX}/bin/Segmentor.exe)
  set(QVTK \${CMAKE_INSTALL_PREFIX}/bin/QVTKWidgetPlugin.dll)
  
  install(FILES ${VTK_DIR}/bin/Release/QVTKWidgetPlugin.dll DESTINATION bin COMPONENT Segmentor)
 
  install(CODE "
    file(GLOB PLUGINS ${QVTK})
    include(BundleUtilities) 
    fixup_bundle(${APPS} \"\${PLUGINS}\" \"\")
    execute_process(COMMAND windeployqt.exe --release ${APPS})
  " COMPONENT Segmentor)
  
  set(CPACK_GENERATOR NSIS)
elseif(APPLE)
  set(plugin_dest_dir Segmentor.app/Contents/PlugIns)
  set(APPS \${CMAKE_INSTALL_PREFIX}/Segmentor.app)
  set(QVTK \${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/libQVTKWidgetPlugin.dylib)

  install(FILES ${VTK_DIR}/lib/libQVTKWidgetPlugin.dylib DESTINATION ${plugin_dest_dir} COMPONENT Segmentor)

  get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)
  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS ${QT_BIN_DIR})

  install(CODE "
    file(GLOB PLUGINS \"${QVTK}\")
    include(BundleUtilities) 
#    fixup_bundle(${APPS} \"\${PLUGINS}\" \"\")
    execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} ${APPS} -verbose=2 -always-overwrite)
  " COMPONENT Segmentor)

  set(CPACK_GENERATOR DragNDrop)
endif()

# CPack
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Installer)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Segmentor")
set(CPACK_PACKAGE_EXECUTABLES Segmentor;Segmentor)

include(CPack)
