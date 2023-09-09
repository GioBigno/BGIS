cmake_minimum_required(VERSION 3.16)

project(test5 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_PATH "/home/bigno/Qt/6.5.2/gcc_64" CACHE STRING "Path of Qt library package")
set(SHAPELIB_PATH "/home/bigno/dev/lib/shapelib/bin/share/shapelib/" CACHE STRING "Path of shapelib library package")

set(CMAKE_PREFIX_PATH "${QT_PATH}"
                      "${SHAPELIB_PATH}")

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)
find_package(shapelib REQUIRED)
qt_standard_project_setup()

qt_add_executable(apptest5
    main.cpp
)

qt_add_qml_module(apptest5
    URI test5
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES scene.h
    SOURCES scene.cpp
    SOURCES Shape.h
    SOURCES Shape.cpp
    SOURCES Part.h
    SOURCES Part.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apptest5 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest5
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apptest5
    PRIVATE Qt6::Quick
    PUBLIC ${shapelib_LIBRARIES}
)

include(GNUInstallDirs)
install(TARGETS apptest5
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
