
set(app_sources "epd_driver.c"
                "epd_board.c"
                "render.c"
                "display_ops.c"
                "tps65185.c"
                "pca9555.c"
                "font.c"
                "lut.c"
                "board_specific.c"
				"builtin_waveforms.c"
                
                "i2s_data_bus_s3.c"
                "rmt_pulse.c"
				"highlevel.c"
                "epd_temperature.c"
                "board/epd_board_common.c"
                "board/epd_board_lilygo_s3_47.c"
                
)
# Need to find how to add this when MCU is ESP32
# "i2s_data_bus.c"
# "board/epd_board_lilygo_t5_47.c"
# "board/epd_board_v2_v3.c"
# "board/epd_board_v4.c"
# "board/epd_board_v5.c"
# "board/epd_board_v6.c"
# And others only when MCU is ESP32S3
idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "include" 
        REQUIRES esp_adc esp_timer driver 
        PRIV_REQUIRES esp_lcd
        )

set_source_files_properties("lut.c" PROPERTIES COMPILE_OPTIONS -mno-fix-esp32-psram-cache-issue)
