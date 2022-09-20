#include "epd_board.h"

#include "../display_ops.h"
#include "i2s_data_bus_s3.h"
#include "../rmt_pulse.h"
#include "epd_internals.h" // EPD_WIDTH

/* Config Register Control */
#define CFG_DATA GPIO_NUM_13
#define CFG_CLK GPIO_NUM_12
#define CFG_STR GPIO_NUM_0

/* Data Lines */
#define D7 GPIO_NUM_7
#define D6 GPIO_NUM_6
#define D5 GPIO_NUM_5
#define D4 GPIO_NUM_4
#define D3 GPIO_NUM_3
#define D2 GPIO_NUM_2
#define D1 GPIO_NUM_1
#define D0 GPIO_NUM_8

/* Control Lines */
#define CKV GPIO_NUM_38
#define STH GPIO_NUM_40

/* Edges */
#define CKH GPIO_NUM_41

typedef struct {
  bool ep_latch_enable : 1;
  bool power_disable : 1;
  bool pos_power_enable : 1;
  bool neg_power_enable : 1;
  bool ep_scan_direction : 1;
  bool ep_stv : 1;
  bool ep_mode : 1;
  bool ep_output_enable : 1;
} epd_config_register_t;

static i2s_bus_config i2s_config = {
  .clock = CKH,
  .start_pulse = STH,
  .data_0 = D0,
  .data_1 = D1,
  .data_2 = D2,
  .data_3 = D3,
  .data_4 = D4,
  .data_5 = D5,
  .data_6 = D6,
  .data_7 = D7,
  // add an offset off dummy bytes to allow for enough timing headroom
  .epd_row_width = EPD_WIDTH + 32
};

static void IRAM_ATTR push_cfg_bit(bool bit) {
    fast_gpio_set_lo(CFG_CLK);
    if (bit)
    {
        fast_gpio_set_hi(CFG_DATA);
    }
    else
    {
        fast_gpio_set_lo(CFG_DATA);
    }
    fast_gpio_set_hi(CFG_CLK);
}

static void IRAM_ATTR push_cfg(epd_config_register_t *cfg)
{
    fast_gpio_set_lo(CFG_STR);

    // push config bits in reverse order
    push_cfg_bit(cfg->ep_output_enable);
    push_cfg_bit(cfg->ep_mode);
    push_cfg_bit(cfg->ep_scan_direction);
    push_cfg_bit(cfg->ep_stv);

    push_cfg_bit(cfg->neg_power_enable);
    push_cfg_bit(cfg->pos_power_enable);
    push_cfg_bit(cfg->power_disable);
    push_cfg_bit(cfg->ep_latch_enable);

    fast_gpio_set_hi(CFG_STR);
}

static epd_config_register_t config_reg;

static void epd_board_init(uint32_t epd_row_width) {
  gpio_reset_pin(CFG_DATA);
  gpio_reset_pin(CFG_CLK);
  gpio_reset_pin(CFG_STR);
  gpio_set_direction(CFG_DATA, GPIO_MODE_OUTPUT);
  gpio_set_direction(CFG_CLK, GPIO_MODE_OUTPUT);
  gpio_set_direction(CFG_STR, GPIO_MODE_OUTPUT);

  /* Power Control Output/Off */
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CFG_DATA], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CFG_CLK], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CFG_STR], PIN_FUNC_GPIO);
 
  
  fast_gpio_set_lo(CFG_STR);

  config_reg.ep_latch_enable = false;
  config_reg.power_disable = true;
  config_reg.pos_power_enable = false;
  config_reg.neg_power_enable = false;
  config_reg.ep_stv = true;
  config_reg.ep_scan_direction = true;
  config_reg.ep_mode = false;
  config_reg.ep_output_enable = false;
  push_cfg(&config_reg);

  // i2s_data_bus_s3
  i2s_bus_init( &i2s_config );

  rmt_pulse_init(CKV);
}

static void epd_board_set_ctrl(epd_ctrl_state_t *state, const epd_ctrl_state_t * const mask) {
  if (state->ep_sth) {
    fast_gpio_set_hi(STH);
  } else {
    fast_gpio_set_lo(STH);
  }

  if (mask->ep_output_enable || mask->ep_mode || mask->ep_stv || mask->ep_latch_enable) {
    fast_gpio_set_lo(CFG_STR);

    // push config bits in reverse order
    push_cfg_bit(state->ep_output_enable);
    push_cfg_bit(state->ep_mode);
    push_cfg_bit(config_reg.ep_scan_direction);
    push_cfg_bit(state->ep_stv);

    push_cfg_bit(config_reg.neg_power_enable);
    push_cfg_bit(config_reg.pos_power_enable);
    push_cfg_bit(config_reg.power_disable);
    push_cfg_bit(state->ep_latch_enable);

    fast_gpio_set_hi(CFG_STR);
  }
}

static void epd_board_poweron(epd_ctrl_state_t *state) {
  //i2s_gpio_attach(&i2s_config);
    config_reg.ep_scan_direction = true;
    config_reg.power_disable = false;
    push_cfg(&config_reg);
    busy_delay(100 * 240);
    config_reg.neg_power_enable = true;
    push_cfg(&config_reg);
    busy_delay(500 * 240);
    config_reg.pos_power_enable = true;
    push_cfg(&config_reg);
    busy_delay(100 * 240);
    config_reg.ep_stv = true;
    push_cfg(&config_reg);
    fast_gpio_set_hi(STH);
}

static void epd_board_poweroff_common(epd_ctrl_state_t *state) {
  config_reg.pos_power_enable = false;
  push_cfg(&config_reg);
  busy_delay(10 * 240);
  config_reg.neg_power_enable = false;
  push_cfg(&config_reg);
  busy_delay(100 * 240);
  config_reg.power_disable = true;
  push_cfg(&config_reg);

  config_reg.ep_stv = false;
  push_cfg(&config_reg);
  //i2s_gpio_detach(&i2s_config);
}

static void epd_board_poweroff(epd_ctrl_state_t *state) {
  epd_board_poweroff_common(state);
}

static void epd_board_poweroff_touch(epd_ctrl_state_t *state) {
  // This was re-purposed as power enable.
  config_reg.ep_scan_direction = true;
  epd_board_poweroff_common(state);
}


const EpdBoardDefinition epd_board_lilygo_s3_47 = {
  .init = epd_board_init,
  .deinit = NULL,
  .set_ctrl = epd_board_set_ctrl,
  .poweron = epd_board_poweron,
  .poweroff = epd_board_poweroff,

  .temperature_init = NULL,
  .ambient_temperature = NULL,
};

const EpdBoardDefinition epd_board_lilygo_s3_47_touch = {
  .init = epd_board_init,
  .deinit = NULL,
  .set_ctrl = epd_board_set_ctrl,
  .poweron = epd_board_poweron,
  .poweroff = epd_board_poweroff_touch,

  .temperature_init = NULL,
  .ambient_temperature = NULL,
};
