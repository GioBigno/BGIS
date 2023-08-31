cmake_minimum_required(VERSION 3.16)

project(test6 VERSION 0.1 LANGUAGES CXX)

add_compile_definitions(USE_UNSTABLE_GEOS_CPP_API)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

set(GEOS_PATH "/home/bigno/giorgio/bin/geos" CACHE STRING "Path of geos library package")
set(SHAPELIB_PATH "/home/bigno/giorgio/bin/shapelib/" CACHE STRING "Path of shapelib library package")

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)
find_package(GEOS REQUIRED PATHS "${GEOS_PATH}")
find_package(shapelib REQUIRED PATHS "${SHAPELIB_PATH}")

qt_standard_project_setup()

qt_add_executable(apptest6
    main.cpp
    Scene.h
    Scene.cpp
    Part.h
    Part.cpp
    Shape.h
    Shape.cpp
    shpformat.h
    shpformat.cpp
    shpreader.h
    shpreader.cpp
    esempio.txt
    res.qrc
)

#qt_add_qml_module(apptest6
#    URI test6
#    VERSION 1.0
#    QML_FILES Main.qml
#)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apptest6 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest6
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apptest6
    PRIVATE Qt6::Quick
    PUBLIC ${shapelib_LIBRARIES}
    PUBLIC GEOS::geos
)

include(GNUInstallDirs)
install(TARGETS apptest6
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
