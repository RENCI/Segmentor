cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

set(SEGMENTOR_VERSION 0.3.1)

project(Segmentor VERSION ${SEGMENTOR_VERSION})

add_definitions(-DSEGMENTOR_VERSION="${SEGMENTOR_VERSION}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Install)

if(APPLE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)	
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
# CMAKE_AUTOMOC is ON so the MOC headers will be automatically wrapped.
#add_executable(Segmentor MACOSX_BUNDLE WIN32 ${CXX_FILES} ${UISrcs} ${QT_WRAP} ${UI_RESOURCES})
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

  set(APPS \${CMAKE_INSTALL_PREFIX}/bin/Segmentor.exe)
  
  install(FILES ${VTK_DIR}/bin/Release/QVTKWidgetPlugin.dll DESTINATION bin COMPONENT Segmentor)
 
  install(CODE "
    include(BundleUtilities) 
    fixup_bundle(${APPS} \"\" \"\")
    execute_process(COMMAND windeployqt.exe --release ${APPS})
  " COMPONENT Segmentor)
  
  set(CPACK_GENERATOR NSIS)
elseif(APPLE)
  set(PLUGIN_DEST_DIR Segmentor.app/Contents/PlugIns)
  set(APPS \${CMAKE_INSTALL_PREFIX}/Segmentor.app)

  # Adjust Info.plist
  add_custom_command(
    TARGET Segmentor
    POST_BUILD
    COMMAND plutil -replace NSRequiresAquaSystemAppearance -bool true Segmentor.app/Contents/Info.plist
  )

  install(FILES ${VTK_DIR}/lib/libQVTKWidgetPlugin.dylib DESTINATION ${PLUGIN_DEST_DIR} COMPONENT Segmentor)

  get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)
  find_program(MACDEPLOYQT macdeployqt HINTS ${QT_BIN_DIR})

  install(CODE "
    execute_process(COMMAND ${MACDEPLOYQT} ${APPS} -always-overwrite)
  " COMPONENT Segmentor)

  set(CPACK_GENERATOR DragNDrop)
  set(CPACK_BUNDLE_PLIST \$CMAKE_INSTALL_PREFIX}/Segmentor.app/Contents/Info.plist) 
  set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/Segmentor.icns)
elseif(UNIX) 
  set(APPS \${CMAKE_INSTALL_PREFIX}/bin/Segmentor)

  set_target_properties(Segmentor PROPERTIES INSTALL_RPATH $ORIGIN)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags" )

  install(CODE "
    include(BundleUtilities) 
    fixup_bundle(${APPS} \"\" ${VTK_DIR}/lib)
  " COMPONENT Segmentor)

  set(CPACK_GENERATOR STGZ)
endif()

# CPack
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Installer)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Segmentor")
set(CPACK_PACKAGE_EXECUTABLES Segmentor;Segmentor)

include(CPack)
