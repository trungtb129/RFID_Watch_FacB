set(app_src UNIT_UHF_RFID.c)
set(req driver)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "." "../Center"
                    REQUIRES ${req} )

