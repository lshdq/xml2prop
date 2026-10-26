cmake_minimum_required(VERSION 3.21)

project(xml2prop VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.3 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    QuickDialogs2
    Svg
    Test
)

qt_standard_project_setup()

add_library(xml2prop_core STATIC
    src/XmlToPropertiesConverter.cpp
    src/XmlToPropertiesConverter.h
    src/PropertiesEscaper.h
)
target_link_libraries(xml2prop_core PUBLIC Qt6::Core)
target_include_directories(xml2prop_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(APP_RESOURCES
    assets/resources.qrc
)

if(WIN32)
    # Keep the ICO visible to IDEs and make it an explicit dependency of the
    # Windows resource compilation without attempting to compile it as code.
    set_source_files_properties(assets/xml2prop.ico PROPERTIES HEADER_FILE_ONLY TRUE)
    set_source_files_properties(assets/app_icon.rc PROPERTIES
        OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/xml2prop.ico"
    )
    list(APPEND APP_RESOURCES
        assets/app_icon.rc
        assets/xml2prop.ico
    )
endif()

qt_add_executable(xml2prop WIN32
    src/main.cpp
    ${APP_RESOURCES}
)

qt_add_qml_module(xml2prop
    URI Xml2Prop
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(xml2prop PRIVATE
    xml2prop_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2
    Qt6::Svg
)

enable_testing()
add_subdirectory(tests)
