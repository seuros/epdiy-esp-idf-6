#include "esp_adc/adc_oneshot.h"
#include "epd_board.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

static const adc_channel_t channel = ADC_CHANNEL_7;
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle;

#define NUMBER_OF_SAMPLES 100

void epd_board_temperature_init_v2() {
    // ADC1 Init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_6,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

    // ADC1 Calibration
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_6,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));
}

float epd_board_ambient_temperature_v2() {
    uint32_t value = 0;
    for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &raw));
        value += raw;
    }
    value /= NUMBER_OF_SAMPLES;
    // voltage in mV
    int voltage;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, value, &voltage));
    return (voltage - 500.0) / 10.0;
}
