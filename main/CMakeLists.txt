idf_component_register(SRCS  "main.c"   "ssd1306.c" "ssd1306_fonts.c" "connect_wifi.c" 
                             "dht.c" "yl69.c" "ultrasonic.c" "yl69_task.c" "dht_task.c" "ultrasonic_task.c"
                             "ssd1306_task.c" "send_data_task.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_wifi nvs_flash esp_http_client esp_timer
                    )
