qt_add_executable(tst_XmlToPropertiesConverter
    tst_XmlToPropertiesConverter.cpp
)

target_link_libraries(tst_XmlToPropertiesConverter PRIVATE
    xml2prop_core
    Qt6::Test
)

if (WIN32)
    # Qt's DLL directory is not necessarily on PATH in a clean shell. Keep
    # the normal ctest invocation self-contained for MSVC and MinGW builds.
    get_filename_component(XML2PROP_COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_test(NAME XmlToPropertiesConverter
        COMMAND ${CMAKE_COMMAND} -E env
            "PATH=$<TARGET_FILE_DIR:Qt6::Core>;${XML2PROP_COMPILER_BIN_DIR};$ENV{PATH}"
            $<TARGET_FILE:tst_XmlToPropertiesConverter>
    )
else()
    add_test(NAME XmlToPropertiesConverter COMMAND tst_XmlToPropertiesConverter)
endif()

qt_add_executable(tst_Xml2PropQmlSmoke
    tst_Xml2PropQmlSmoke.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/resources.qrc
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../qml/Main.qml
    PROPERTIES QT_RESOURCE_ALIAS "qml/Main.qml"
)

qt_add_qml_module(tst_Xml2PropQmlSmoke
    URI Xml2Prop
    VERSION 1.0
    QML_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../qml/Main.qml
)

target_link_libraries(tst_Xml2PropQmlSmoke PRIVATE
    xml2prop_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2
    Qt6::Svg
    Qt6::Test
)

if (WIN32)
    add_test(NAME Xml2PropQmlSmoke
        COMMAND ${CMAKE_COMMAND} -E env
            "QT_QPA_PLATFORM=offscreen"
            "PATH=$<TARGET_FILE_DIR:Qt6::Core>;${XML2PROP_COMPILER_BIN_DIR};$ENV{PATH}"
            $<TARGET_FILE:tst_Xml2PropQmlSmoke>
    )
else()
    add_test(NAME Xml2PropQmlSmoke
        COMMAND ${CMAKE_COMMAND} -E env
            "QT_QPA_PLATFORM=offscreen"
            $<TARGET_FILE:tst_Xml2PropQmlSmoke>
    )
endif()
