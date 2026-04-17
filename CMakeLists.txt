cmake_minimum_required(VERSION 3.16)

project(BDraw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

find_package(spdlog CONFIG REQUIRED)
find_package(Armadillo REQUIRED) # Rimosso CONFIG per maggiore compatibilità


include(FetchContent)
FetchContent_Declare(
  libassert
  GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert.git
  GIT_TAG        v2.2.1 # <HASH or TAG>
)
FetchContent_MakeAvailable(libassert)


set(TS_FILES BDraw_it_IT.ts)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/cartesianView.cpp
    include/mainwindow.h
    include/cartesianView.h
    mainwindow.ui
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BDraw
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(BDraw SHARED ${PROJECT_SOURCES})
    else()
        add_executable(BDraw ${PROJECT_SOURCES})
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(BDraw PRIVATE include)

# --- Linking ---
target_link_libraries(BDraw PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    spdlog::spdlog
    libassert::assert
)


# Gestione flessibile per Armadillo (target o variabile)
if(TARGET Armadillo::Armadillo)
    target_link_libraries(BDraw PRIVATE Armadillo::Armadillo)
else()
    target_link_libraries(BDraw PRIVATE ${ARMADILLO_LIBRARIES})
    target_include_directories(BDraw PRIVATE ${ARMADILLO_INCLUDE_DIRS})
endif()

if(APPLE)
    # Su macOS usiamo il framework nativo Accelerate per prestazioni ottimali con Armadillo
    find_library(ACCELERATE_FRAMEWORK Accelerate REQUIRED)
    target_link_libraries(BDraw PRIVATE ${ACCELERATE_FRAMEWORK})
    message(STATUS "macOS detected: Linking Accelerate framework")
else()
    # Su Windows e Linux, VCPKG fornisce OpenBLAS (come specificato nel vcpkg.json)
    find_package(OpenBLAS CONFIG REQUIRED)
    target_link_libraries(BDraw PRIVATE OpenBLAS::OpenBLAS)
    message(STATUS "Windows/Linux detected: Linking OpenBLAS from VCPKG")
endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BDraw)
endif()
set_target_properties(BDraw PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BDraw
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BDraw)
endif()



# On windows copy libassert.dll to the same directory as the executable for your_target
if(WIN32)
  add_custom_command(
    TARGET BDraw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:libassert::assert>
    $<TARGET_FILE_DIR:BDraw>
  )
endif()

# Create a .dSYM file on macos. Currently required, but hopefully not for long
if(APPLE)
  add_custom_command(
    TARGET BDraw
    POST_BUILD
    COMMAND dsymutil $<TARGET_FILE:BDraw>
  )
endif()