SET(SOURCES screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)
#set(req driver lvgl)

idf_component_register(SRCS "spi_lcd_touch_example_main.c" "lvgl_demo_ui.c" ${SOURCES}
                       	INCLUDE_DIRS "."
			REQUIRES ${req})

#add_library(ui ${SOURCES})