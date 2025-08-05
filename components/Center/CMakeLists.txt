set(app_src Task_Center.c Button.c)
set(req driver esp_adc)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${req})

