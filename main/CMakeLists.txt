set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../webpages")

idf_component_register(SRCS "app_http_server.c" 
                            "app_main.c"
                            "core_wifi_sta.c"
                            "core_ota.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "${WEB_SRC_DIR}/ota/ota.html"
                    )
