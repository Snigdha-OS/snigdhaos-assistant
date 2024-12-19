# Specify the minimum CMake version required for the project.
cmake_minimum_required(VERSION 3.5)

# Define the project name, version, and the programming language used.
project(snigdhaos-assistant VERSION 0.1 LANGUAGES CXX)

# Include the current directory in the list of include directories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable automatic handling of Qt-specific features (UIC, MOC, RCC).
# AUTOUIC processes the .ui file (qt/snigdhaosassistant.ui) to generate the corresponding 
# header file, enabling seamless integration of the GUI layout into the application.
set(CMAKE_AUTOUIC ON)
# AUTOMOC processes the Q_OBJECT macro in header files (like qt/snigdhaosassistant.h) 
# to enable Qt's signal-slot mechanism and other meta-object features.
set(CMAKE_AUTOMOC ON)
# AUTORCC (though not explicitly used here) would manage resources if a .qrc file were included 
# in PROJECT_SOURCES.
set(CMAKE_AUTORCC ON)

# Specify the version of the C++ standard to use.
# Here, we are setting it to C++17 to enable modern language features such as std::optional,
# std::filesystem, structured bindings, and more.
set(CMAKE_CXX_STANDARD 17)

# Enforce the requirement for the specified C++ standard.
# If the compiler does not support C++17, the build process will fail with an error.
# This ensures compatibility and prevents fallback to an older C++ standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This line explicitly searches for either Qt6 or Qt5 and requires the Widgets and Network components.
# The NAMES argument tells CMake to look for these specific versions (Qt6 and Qt5).
# If neither version is found, an error will be raised due to the REQUIRED keyword.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)

# This line dynamically selects the Qt version based on the value of the variable QT_VERSION_MAJOR.
# If QT_VERSION_MAJOR is 6, it will search for Qt6; if QT_VERSION_MAJOR is 5, it will search for Qt5.
# It also requires the Widgets and Network components, and will raise an error if they are not found.
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        qt/main.cpp
        qt/snigdhaosassistant.cpp
        qt/snigdhaosassistant.h
        qt/snigdhaosassistant.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(snigdhaos-assistant
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET snigdhaos-assistant APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
# else()
#     if(ANDROID)
#         add_library(snigdhaos-assistant SHARED
#             ${PROJECT_SOURCES}
#         )
# # Define properties for Android with Qt 5 after find_package() calls as:
# #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#     else()
#         add_executable(snigdhaos-assistant
#             ${PROJECT_SOURCES}
#         )
#     endif()
# endif()

else()
    add_executable(snigdhaos-assistant
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(snigdhaos-assistant PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
# if(${QT_VERSION} VERSION_LESS 6.1.0)
#   set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.snigdhaos-assistant)
# endif()
# set_target_properties(snigdhaos-assistant PROPERTIES
#     ${BUNDLE_ID_OPTION}
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )

# include(GNUInstallDirs)
install(TARGETS snigdhaos-assistant
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(snigdhaos-assistant)
endif()
