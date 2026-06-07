idf_component_register(SRCS "SemaphoreTake_driver_main.c"
                       INCLUDE_DIRS "." 
PRIV_REQUIRES esp_driver_ledc esp_driver_uart esp_driver_gpio
)
