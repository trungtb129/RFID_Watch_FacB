set(app_src TCP_CLIENT_V4.c)
set(req driver esp_netif)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "." "../Center"
                    REQUIRES ${req})

