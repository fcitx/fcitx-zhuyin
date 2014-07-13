foreach(icon zhuyin)
    install(FILES ${icon}.png RENAME fcitx-${icon}.png
            DESTINATION share/icons/hicolor/48x48/status/)
    install(FILES ${icon}.png
            DESTINATION share/fcitx/imicon)
endforeach()
