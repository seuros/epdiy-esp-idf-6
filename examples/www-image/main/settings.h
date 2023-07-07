// WiFi configuration:
#define ESP_WIFI_SSID     "sagemcom6AE0"
#define ESP_WIFI_PASSWORD "GTZZGJN52NXQZY"

// Affects the gamma to calculate gray (lower is darker/higher contrast)
// Nice test values: 0.9 1.2 1.4 higher and is too bright
//double gamma_value = 0.7;

// - - - - Display configuration - - - - - - - - -
// EPD Waveform should match your EPD for good grayscales
#define WAVEFORM EPD_BUILTIN_WAVEFORM
#define DISPLAY_ROTATION EPD_ROT_LANDSCAPE
// - - - - end of Display configuration  - - - - -

// Deepsleep configuration
#define MILLIS_DELAY_BEFORE_SLEEP 1000
#define DEEPSLEEP_MINUTES_AFTER_RENDER 16

// Image URL and jpg settings. Make sure to update WIDTH/HEIGHT if using loremflickr
// Note: Only HTTP protocol supported (Check README to use SSL secure URLs) loremflickr
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//#define IMG_URL ("https://loremflickr.com/"STR(EPD_WIDTH)"/"STR(EPD_HEIGHT))
#define IMG_URL "http://img.cale.es/jpg/fasani/5ef2ee980a4ec"
// idf >= 4.3 needs VALIDATE_SSL_CERTIFICATE set to true for https URLs
// Please check the README to understand how to use an SSL Certificate
// Note: This makes a sntp time sync query for cert validation  (It's slower)
// IMPORTANT: idf updated and now when you use Internet requests you need to server cert verification
//            heading ESP-TLS in https://newreleases.io/project/github/espressif/esp-idf/release/v4.3-beta1
#define VALIDATE_SSL_CERTIFICATE false
// To make an insecure request please check Readme

// Jpeg: Adds dithering to image rendering (Makes grayscale smoother on transitions)
#define JPG_DITHERING false

// As default is 512 without setting buffer_size property in esp_http_client_config_t
#define HTTP_RECEIVE_BUFFER_SIZE 1200

#define DEBUG_VERBOSE true