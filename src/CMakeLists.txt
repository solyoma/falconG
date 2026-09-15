qt_add_executable(falconG PRIVATE
    albums.cpp albums.h
	common.h
    config.cpp config.h
    crc32.cpp crc32.h
    csscreator.cpp csscreator.h
	dragdrop.cpp dragdrop.h
    falcong.cpp falcong.h falcong.ui
	filereader.cpp filereader.h
    hidden.h hidden.ui
    imageviewer.cpp imageviewer.h imageviewer.ui
    languages.cpp languages.h
    main.cpp
    schemes.cpp schemes.h
	selectStruct.h
    sourcehistory.cpp sourcehistory.h sourcehistory.ui
    structwriter.cpp structwriter.h
    stylehandler.cpp stylehandler.h
    support.cpp support.h
    textEditor.cpp textEditor.h textEditor.ui
    thumbnailView.cpp thumbnailView.h
	TransferDialog.cpp TransferDialog.h TransferDialog.ui
    treeView.cpp treeView.h
	updownload.cpp updownload.h
	videoplayer.cpp videoplayer.h
)
target_link_libraries(falconG PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Multimedia
    Qt::Concurrent
    Qt::MultimediaWidgets
    Qt::WebEngineCore
    Qt::WebEngineWidgets
)

target_compile_options(falconG PRIVATE
    -Wno-unused-parameter
    -std=c++17
)


# Resources:
set(falcong_resource_files
    "Resources/aliasIcon.png"
    "Resources/blue-checked.png"
    "Resources/blue-unchecked.png"
    "Resources/broken-link.png"
    "Resources/content-icon.png"
    "Resources/delete.png"
    "Resources/falconG-icon.png"
    "Resources/falconG-splash.png"
    "Resources/folderIcon.png"
    "Resources/left-icon.png"
    "Resources/link.png"
    "Resources/movedown-icon.png"
    "Resources/moveup-icon.png"
    "Resources/nav-icon.png"
    "Resources/noresizeMark.png"
    "Resources/open-folder-with-document.png"
    "Resources/up-icon.png"
)

qt_add_resources(falconG "falcong"
    PREFIX
        "/icons"
    FILES
        ${falcong_resource_files}
)
set(falcong1_resource_files
    "en_US.ts"
    "hu_HU.ts"
)

qt_add_resources(falconG "falcong1"
    PREFIX
        "/translations"
    FILES
        ${falcong1_resource_files}
)
set(falcong2_resource_files
    "Resources/NoImage.jpg"
    "Resources/falconG.js"
    "Resources/falconGGen.js"
    "Resources/index_en.html"
    "Resources/index_hu.html"
    "Resources/placeholder.jpg"
    "Resources/placeholder2.jpg"
)

qt_add_resources(falconG "falcong2"
    PREFIX
        "/Preview"
    FILES
        ${falcong2_resource_files}
)

install(TARGETS falconG
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET falconG
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
