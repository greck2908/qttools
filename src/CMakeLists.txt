# Generated from src.pro.

# special case begin
# Need to stop building these apps by default because they would fail
# in device_and_simulator builds.
if(IOS)
   set(_qt_additional_tools_to_exclude qtplugininfo qtpaths)
endif()
# special case end

qt_exclude_tool_directories_from_default_target(
    distancefieldgenerator
    pixeltool
    ${_qt_additional_tools_to_exclude} # special case
)

add_subdirectory(global) # special case add as first directory
add_subdirectory(linguist)
# add_subdirectory(global) # special case remove
if(QT_FEATURE_png AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton AND TARGET Qt::Widgets)
    add_subdirectory(assistant)
    add_subdirectory(designer)
    add_subdirectory(pixeltool)
endif()
if(QT_FEATURE_png AND QT_FEATURE_thread AND QT_FEATURE_toolbutton AND TARGET Qt::Quick AND TARGET Qt::Widgets)
    add_subdirectory(distancefieldgenerator)
endif()
if(QT_FEATURE_commandlineparser)
    add_subdirectory(qtattributionsscanner)
endif()
if(QT_FEATURE_commandlineparser AND QT_FEATURE_library AND (android_app OR NOT ANDROID))
    add_subdirectory(qtplugininfo)
endif()
if(QT_FEATURE_clang AND QT_FEATURE_commandlineparser AND QT_FEATURE_thread)
    add_subdirectory(qdoc)
endif()
if(android_app OR (QT_FEATURE_commandlineparser AND NOT ANDROID))
    add_subdirectory(qtpaths)
endif()
if(MACOS)
    add_subdirectory(macdeployqt)
endif()
if(TARGET Qt::DBus)
    add_subdirectory(qdbus)
endif()
if(WIN32)
    add_subdirectory(windeployqt)
endif()
if(WINRT)
    add_subdirectory(winrtrunner)
endif()
if(QT_FEATURE_commandlineparser AND TARGET Qt::Gui AND NOT ANDROID AND NOT QNX AND NOT UIKIT AND NOT WASM AND NOT WINRT)
    add_subdirectory(qtdiag)
endif()
