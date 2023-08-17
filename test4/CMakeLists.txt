cmake_minimum_required(VERSION 3.16)

project(test4 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(apptest4
    main.cpp
)

qt_add_qml_module(apptest4
    URI test4
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES whiteboard.cpp
    SOURCES whiteboard.h
    SOURCES figure.h
    SOURCES circle.h
    SOURCES circle.cpp
    SOURCES square.h
    SOURCES square.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apptest4 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest4
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apptest4
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS apptest4
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
