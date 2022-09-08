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
if (IDF_TARGET_ESP32) 
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
                "i2s_data_bus.c"
                "board/epd_board_lilygo_t5_47.c"
                "board/epd_board_v2_v3.c"
                "board/epd_board_v4.c"
                "board/epd_board_v5.c"
                "board/epd_board_v6.c"
)
endif()
# This does not work. Also not: if (IDF_TARGET_ESP32S3)
if(IDF_TARGET=="esp32s3")
    list(APPEND app_sources "board/epd_board_lilygo_s3_47.c")
endif()

if (IDF_VERSION_MAJOR GREATER_EQUAL 5)
  set(COMPONENT_REQUIRES esp_adc esp_timer driver esp_lcd)
  elseif (IDF_VERSION_MAJOR EQUAL 4)
  set(COMPONENT_REQUIRES esp_adc esp_timer driver)
endif()


idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "include" 
        REQUIRES ${COMPONENT_REQUIRES}
        )

set_source_files_properties("lut.c" PROPERTIES COMPILE_OPTIONS -mno-fix-esp32-psram-cache-issue)
