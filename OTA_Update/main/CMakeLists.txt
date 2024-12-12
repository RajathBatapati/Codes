idf_component_register(SRCS "OTA_update.c" "Wifi.c" "mqtt.c" "ota.c"
                    INCLUDE_DIRS "."
		    EMBED_TXTFILES "certs/server.pem")
