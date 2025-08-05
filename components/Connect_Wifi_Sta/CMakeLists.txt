set(app_src CONNECT_WIFI_STA.c)
set(req driver esp_netif esp_wifi nvs_flash)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "." "../Center"
                    REQUIRES ${req})

