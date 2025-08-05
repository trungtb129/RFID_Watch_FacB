idf_component_register(SRCS "Handle_NVS.c"
                            "Task_NVS.c"
                    INCLUDE_DIRS  "." "../Center"
                    REQUIRES nvs_flash driver)
