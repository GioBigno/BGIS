cmake_minimum_required(VERSION 3.22)

project(BGIS VERSION 6.0 LANGUAGES CXX)

add_compile_definitions(USE_UNSTABLE_GEOS_CPP_API)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_BUILD)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

set(QT_PATH "/home/giorgio/Qt/6.5.2/gcc_64" CACHE PATH "Path of Qt library package")
set(GEOS_PATH "/home/giorgio/dev/lib/bin/geos" CACHE PATH "Path of geos library package")
set(SHAPELIB_PATH "/home/giorgio/dev/lib/bin/shapelib" CACHE PATH "Path of shapelib library package")

list(APPEND CMAKE_PREFIX_PATH "${QT_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${GEOS_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${SHAPELIB_PATH}")

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(Qt6 6.5 REQUIRED COMPONENTS Qml)
find_package(GEOS REQUIRED)
find_package(shapelib REQUIRED)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")
# set up a mapping so that the Release configuration for the Qt imported target is
# used in the COVERAGE CMake configuration.
set_target_properties(Qt6::Quick PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE")

qt_standard_project_setup()

qt_add_executable(BGIS
    src/main.cpp
    src/Scene.h
    src/Scene.cpp
    src/shpformat.h
    src/shpformat.cpp
    src/shpreader.h
    src/shpreader.cpp

    src/res.qrc
)

#qt_add_qml_module(BGIS
#    URI BGIS
#    VERSION 1.0
#    QML_FILES Main.qml
#)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(BGIS PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest6
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(BGIS
    PUBLIC Qt6::Quick
    PUBLIC ${shapelib_LIBRARIES}
    PUBLIC GEOS::geos
)

set_target_properties(BGIS PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

include(GNUInstallDirs)
install(TARGETS BGIS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt6_generate_deploy_qml_app_script(
    TARGET BGIS
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    MACOS_BUNDLE_POST_BUILD
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
