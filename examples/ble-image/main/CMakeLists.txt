idf_component_register(SRCS "gatt_server_receive.cpp"
    REQUIRES epd_driver 
    # bluetooth & nvs
    bt nvs_flash
    # jpeg decoder
    jpegdec
    # timer
    esp_timer
  )
