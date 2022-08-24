/* If not S3 this will fail since has no LCD module*/
#if CONFIG_IDF_TARGET_ESP32S3

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

static const char *TAG = "I80";

/// DMA descriptors for front and back line buffer.
/// We use two buffers, so one can be filled while the other
/// is transmitted.
typedef struct
{
    volatile lldesc_t *dma_desc_a;
    volatile lldesc_t *dma_desc_b;

    /// Front and back line buffer.
    uint8_t *buf_a;
    uint8_t *buf_b;
} i2s_parallel_state_t;

static esp_lcd_panel_io_handle_t io_handle = NULL;

/******************************************************************************/
/***        local function prototypes                                       ***/
/******************************************************************************/

/**
 * @brief Initializes a DMA descriptor.
 */
static void fill_dma_desc(volatile lldesc_t *dmadesc, uint8_t *buf, i2s_bus_config *cfg);

/**
 * @brief Address of the currently front DMA descriptor, which uses only the
 *        lower 20bits (according to TRM)
 */
static uint32_t dma_desc_addr();

/**
 * @brief Set up a GPIO as output and route it to a signal.
 */
static void gpio_setup_out(int32_t gpio, int32_t sig, bool invert);

/**
 * @brief Resets "Start Pulse" signal when the current row output is done.
 */
static void IRAM_ATTR i2s_int_hdl(void *arg);

/******************************************************************************/
/***        exported variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        local variables                                                 ***/
/******************************************************************************/

/**
 * @brief Indicates which line buffer is currently back / front.
 */
static int32_t current_buffer = 0;

/**
 * @brief The I2S state instance.
 */
static i2s_parallel_state_t i2s_state;

static intr_handle_t gI2S_intr_handle = NULL;

/**
 * @brief Indicates the device has finished its transmission and is ready again.
 */
static volatile bool output_done = true;

/**
 * @brief The start pulse pin extracted from the configuration for use in
 *        the "done" interrupt.
 */
static gpio_num_t start_pulse_pin;

static uint8_t buffer[(EPD_WIDTH + 32) / 4] = { 0 };

/******************************************************************************/
/***        exported functions                                              ***/
/******************************************************************************/

volatile uint8_t IRAM_ATTR *i2s_get_current_buffer()
{
    return buffer;
}

bool IRAM_ATTR i2s_is_busy()
{
    return !output_done;
}

void IRAM_ATTR i2s_switch_buffer()
{
}

void IRAM_ATTR i2s_start_line_output()
{
    output_done = false;

    esp_lcd_panel_io_tx_color(io_handle, 0, buffer, (960 + 32) / 4);
}

static bool notify_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    // gpio_set_level(start_pulse_pin, 1);
    output_done = true;
    return output_done;
}


void i2s_bus_init(i2s_bus_config *cfg)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
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
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = -1,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 10,
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
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
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

/******************************************************************************/
/***        local functions                                                 ***/
/******************************************************************************/

/// Initializes a DMA descriptor.
static void fill_dma_desc(volatile lldesc_t *dmadesc, uint8_t *buf,
                          i2s_bus_config *cfg)
{
    dmadesc->size = cfg->epd_row_width / 4;
    dmadesc->length = cfg->epd_row_width / 4;
    dmadesc->buf = buf;
    dmadesc->eof = 1;
    dmadesc->sosf = 1;
    dmadesc->owner = 1;
    dmadesc->qe.stqe_next = 0;
    dmadesc->offset = 0;
}

/// Address of the currently front DMA descriptor,
/// which uses only the lower 20bits (according to TRM)
static uint32_t dma_desc_addr()
{
    return (uint32_t)(current_buffer ? i2s_state.dma_desc_a : i2s_state.dma_desc_b) & \
                     0x000FFFFF;
}


/// Set up a GPIO as output and route it to a signal.
static void gpio_setup_out(int32_t gpio, int32_t sig, bool invert)
{
    if (gpio == -1) return;

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction(gpio, GPIO_MODE_DEF_OUTPUT);
    gpio_matrix_out(gpio, sig, invert, false);
}

#endif