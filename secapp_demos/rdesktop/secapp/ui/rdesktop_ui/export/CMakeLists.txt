SET(SOURCES screens/ui_ScreenMain.c
    screens/ui_ScreenLogin.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c)

add_library(ui ${SOURCES})
