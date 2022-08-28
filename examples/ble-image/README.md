| Supported Targets | ESP32 | ESP32-S3 (Only when EPDiy is tested with new Lilygo |
| ----------------- | ----- | -------- |

# ESP-IDF Gatt Server Example

This example shows how create a GATT service by adding attributes one by one. However, this method is defined by Bluedroid and is difficult for users to use.
This branch incorporates the updates done in this [EPDiy pull request #180](https://github.com/vroland/epdiy/pull/180) to update the component so it builds in ESP-IDF Version 5.

Check Espressif GATT server example to see how all this started. 
Credits on this go to [ATC](https://twitter.com/atc1441) and for the BLE advice to Larry Bank

## How to Use Example

Before project configuration and build, be sure to set the correct chip target using:

```bash
idf.py set-target <chip_name>
```

### Hardware Required

* An EPDiy PCB development board with ESP32/ESP-S3 SoC (e.g., Lilygo old board or new one with ESP32S3)
* A USB cable for Power supply and programming

Please note that the [Lilygo ESP047 S3 version](https://www.tindie.com/products/lilygo/lilygo-t5-47-inch-e-paper-v23-esp32-s3/) that I'm starting to adapt in this branch is still not tested and we need to wait until the device arrives to our office to fully test it.
See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

### Settings for UUID128

This example works with UUID16 as default. To change to UUID128, follow this steps:

1. Change the UIID16 to UUID128. You can change the UUID according to your needs.

```c
// Create a new UUID128 (using random values for this example)
uint8_t gatts_service_uuid128_test_X[ESP_UUID_LEN_128] = {0x06, 0x18, 0x7a, 0xec, 0xbe, 0x11, 0x11, 0xea, 0x00, 0x16, 0x02, 0x42, 0x01, 0x13, 0x00, 0x04};
```

By adding this new UUID128, you can remove the `#define` macros with the old UUID16.

2. Add the new UUID128 to the profile.

```c
// Change the size of the UUID from 16 to 128
gl_profile_tab[PROFILE_X_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_128;
// Copy the new UUID128 to the profile
memcpy(gl_profile_tab[PROFILE_X_APP_ID].service_id.id.uuid.uuid.uuid128, gatts_service_uuid128_test_X, ESP_UUID_LEN_128);
```

3. Remove the following line(s)
```c
gl_profile_tab[PROFILE_X_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_X;
```

### How to send the BLE image

Just [create an account in CALE.es](https://cale.es) and add a 960*560 Screen. The LilyGo epaper should have this Firmware flashed.  
