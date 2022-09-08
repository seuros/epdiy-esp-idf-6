#include "i2s_data_bus_s3.h"

#include <driver/periph_ctrl.h>
#include <esp_heap_caps.h>
#include <rom/lldesc.h>
#include <soc/i2s_reg.h>
#include <soc/i2s_struct.h>
#include <soc/rtc.h>
#include "esp_lcd_panel_io.h"
#include "esp_err.h"
#include "esp_log.h"
#include <gpio_periph.c>  // Otherwise: 'PIN_FUNC_GPIO' undeclared
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_internals.h" // EPD_WIDTH

static const char *TAG = "I80";

/// DMA descriptors for front and back line buffer.
/// We use two buffers, so one can be filled while the other is transmitted.
typedef struct
{
    volatile lldesc_t *dma_desc_a;
    volatile lldesc_t *dma_desc_b;

    /// Front and back line buffer.
    uint8_t *buf_a;
    uint8_t *buf_b;
} i2s_parallel_state_t;

static esp_lcd_panel_io_handle_t io_handle = NULL;

/**
 * @brief The I2S state instance.
 */
static i2s_parallel_state_t i2s_state;

static intr_handle_t gI2S_intr_handle = NULL;

/**
 * @brief Indicates the device has finished its transmission and is ready again.
 */
static volatile bool output_done = true;


static uint8_t buffer[(EPD_WIDTH + 32) / 4] = { 0 };

volatile uint8_t IRAM_ATTR *i2s_get_current_buffer()
{
    return buffer;
}

bool IRAM_ATTR i2s_is_busy()
{
    return !output_done;
}

void IRAM_ATTR i2s_switch_buffer() {}

void IRAM_ATTR i2s_start_line_output()
{
    output_done = false;
    esp_lcd_panel_io_tx_color(io_handle, 0, buffer, (EPD_WIDTH + 32) / 4);
}


static bool notify_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    output_done = true;
    return output_done;
}


void i2s_bus_init(i2s_bus_config *cfg)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus\n6:%d 7:%d 4:%d 5:%d 2:%d 3:%d 0:%d 1:%d\nstart_pulse:%d clock:%d\nepd_row_width:%d",
    cfg->data_6,cfg->data_7,cfg->data_4,cfg->data_5,cfg->data_2,cfg->data_3,cfg->data_0,
    cfg->data_1, cfg->start_pulse, cfg->clock, cfg->epd_row_width);

    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        // Withouth clk_src parameter in V5 it just hangs on esp_lcd_new_i80_bus instantiation
        .clk_src = LCD_CLK_SRC_PLL160M,
        .dc_gpio_num = cfg->start_pulse,
        .wr_gpio_num = cfg->clock,
        .data_gpio_nums = {
            cfg->data_6,
            cfg->data_7,
            cfg->data_4,
            cfg->data_5,
            cfg->data_2,
            cfg->data_3,
            cfg->data_0,
            cfg->data_1,
        },
        .bus_width = 8,
        .max_transfer_bytes = (cfg->epd_row_width + 32)/4
    };
    
    esp_err_t err = esp_lcd_new_i80_bus(&bus_config, &i80_bus);
    if (err) {
        ESP_LOGI(TAG, "ERROR: %d", err);
    }
    // pclk_hz = 10 * 1000 * 1000 -> What Lilygo uses
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = -1,
        .pclk_hz = 5 * 1000 * 1000,
        .trans_queue_depth = 4,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 1,
            .dc_dummy_level = 0,
            .dc_data_level = 0,
        },
        .on_color_trans_done = notify_trans_done,
        .user_ctx = NULL,
        .lcd_cmd_bits = 10,
        .lcd_param_bits = 0,
        // .flags.reverse_color_bits = 1
    };
    esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);
    ESP_LOGI(TAG, "8080 esp_lcd_new_panel_io_i80 done");
}


void i2s_deinit()
{
    esp_intr_free(gI2S_intr_handle);

    free(i2s_state.buf_a);
    free(i2s_state.buf_b);
    free((void *)i2s_state.dma_desc_a);
    free((void *)i2s_state.dma_desc_b);

    periph_module_disable(PERIPH_I2S1_MODULE);
}
