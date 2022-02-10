#pragma once

#include "display_ops.h"

#include "tps65185.h"
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <sys/time.h>
// PCA IO expander pins won't be used anymore in S2
#define CFG_PIN_OE          10
#define CFG_PIN_MODE        11
#define CFG_PIN_STV         12
#define CFG_PIN_PWRUP       13
#define CFG_PIN_VCOM_CTRL   14
#define CFG_PIN_WAKEUP      15
#define CFG_PIN_PWRGOOD     16
#define CFG_PIN_INT         17

#define SKIP_TPS_INIT
#define SILENT_CFG

typedef struct {
    i2c_port_t port;
    bool ep_output_enable;
    bool ep_mode;
    bool ep_stv;
    bool pwrup;
    bool vcom_ctrl;
    bool wakeup;
    bool others[8];
} epd_config_register_t;

static bool interrupt_done = false;

static void IRAM_ATTR interrupt_handler(void* arg) {
    interrupt_done = true;
}

int v6_wait_for_interrupt(int timeout) {

    int tries = 0;
    // Before read CFG_INTR. Other pins should have interrupts too
    while (!interrupt_done && gpio_get_level(TPS_INTERRUPT) == 1) {
        if (tries >= 500) {
            return -1;
        }
        tries++;
        vTaskDelay(1);
    }
    int ival = 0;
    interrupt_done = false;
    //pca9555_read_input(EPDIY_I2C_PORT, 1);
	ival = tps_read_register(EPDIY_I2C_PORT, TPS_REG_INT1);
	ival |= tps_read_register(EPDIY_I2C_PORT, TPS_REG_INT2) << 8;
    while (!gpio_get_level(TPS_INTERRUPT)) {vTaskDelay(1); }
    return ival;
}

void config_reg_init(epd_config_register_t* reg) {

    printf("I2C SCL:%d SDA:%d\n", CFG_SCL, CFG_SDA);
    reg->ep_output_enable = false;
    reg->ep_mode = false;
    reg->ep_stv = false;
    reg->pwrup = false;
    reg->vcom_ctrl = false;
    reg->wakeup = false;
    for (int i=0; i<8; i++) {
        reg->others[i] = false;
    }

    // There are no more CFG_INTR interrupts from PCA9555
    // Control lines are now controlled directly by S2 and should be set as Output
    gpio_num_t EP_CONTROL[] = {STH, EPD_STV, EPD_MODE, EPD_OE};
   
    for (int x = 0; x < 4; x++) {
        printf("IO %d to LOW (loop:%d)\n", (int)EP_CONTROL[x], x);
        gpio_set_direction(EP_CONTROL[x], GPIO_MODE_OUTPUT);
        gpio_set_level(EP_CONTROL[x], 0);
        
    }
    
    // TODO: Interrupts should be added for incoming signals from TPS65185
    /* gpio_set_direction(TPS_INTERRUPT, GPIO_MODE_INPUT);
    gpio_set_intr_type(TPS_INTERRUPT, GPIO_INTR_NEGEDGE);
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TPS_INTERRUPT, interrupt_handler, (void *) TPS_INTERRUPT));

    // set all epdiy lines to output except TPS interrupt + PWR good - No more PCA9555
    ESP_ERROR_CHECK(pca9555_set_config(reg->port, CFG_PIN_PWRGOOD | CFG_PIN_INT, 1)); */
}

/*
 Important: Removing PCA9555 means that we should push this config differently
*/
static void push_cfg(epd_config_register_t* reg) {
    uint8_t value = 0x00;
    #ifndef SILENT_CFG
        printf("OE:%d MODE:%d STV:%d\n", reg->ep_output_enable,reg->ep_mode,reg->ep_stv);
    #endif
    gpio_set_level(EPD_OE, reg->ep_output_enable);
    gpio_set_level(EPD_MODE, reg->ep_mode);
    gpio_set_level(EPD_STV, reg->ep_stv);
    if (reg->ep_output_enable) {
        value |= CFG_PIN_OE;
    }
    if (reg->ep_mode) {
        value |= CFG_PIN_MODE;
    }
    if (reg->ep_stv) {
        value |= CFG_PIN_STV;
    }
    if (reg->pwrup) {
        // This is signal for the PMIC tpd65185
        value |= CFG_PIN_PWRUP;
        #ifndef SILENT_CFG
        printf(" CFG_PIN_PWRUP");
        #endif
    }
    if (reg->vcom_ctrl) {
        // This is signal for the PMIC tpd65185
        value |= CFG_PIN_VCOM_CTRL;
        #ifndef SILENT_CFG
        printf(" CFG_PIN_VCOM_CTRL");
        #endif
    }
    if (reg->wakeup) {
        // This is signal for the PMIC tpd65185
        value |= CFG_PIN_WAKEUP;
        #ifndef SILENT_CFG
        printf(" CFG_PIN_WAKEUP");
        #endif
    }

    //ESP_ERROR_CHECK(pca9555_set_value(reg->port, value, 1));
}

static void cfg_poweron(epd_config_register_t* reg) {
    reg->ep_stv = true;
    reg->wakeup = true;
    push_cfg(reg);
    reg->pwrup = true;
    push_cfg(reg);
    reg->vcom_ctrl = true;
    push_cfg(reg);

    // give the IC time to powerup and set lines
    vTaskDelay(100);

    /* while (!(pca9555_read_input(reg->port, 1) & CFG_PIN_PWRGOOD)) {
        vTaskDelay(1);
    } */
#ifndef SKIP_TPS_INIT
    ESP_ERROR_CHECK(tps_write_register(reg->port, TPS_REG_ENABLE, 0x3F));

#ifdef CONFIG_EPD_DRIVER_V6_VCOM
    tps_set_vcom(reg->port, CONFIG_EPD_DRIVER_V6_VCOM);
// Arduino IDE...
#else
    extern int epd_driver_v6_vcom;
    tps_set_vcom(reg->port, epd_driver_v6_vcom);
#endif

    gpio_set_level(STH, 1);

    int tries = 0;
    while (!((tps_read_register(reg->port, TPS_REG_PG) & 0xFA) == 0xFA)) {
        if (tries >= 500) {
            ESP_LOGE("epdiy", "Power enable failed! PG status: %X", tps_read_register(reg->port, TPS_REG_PG));
            return;
        }
        tries++;
        vTaskDelay(1);
    }
    #else
    printf("\n\nWarning: TPS initialization skipped using SKIP_TPS_INIT. Power lines will be 0 volt\n");
    #endif
}

static void cfg_poweroff(epd_config_register_t* reg) {
    reg->vcom_ctrl = false;
    reg->pwrup = false;
    reg->ep_stv = false;
    reg->ep_output_enable = false;
    reg->ep_mode = false;
    push_cfg(reg);
    vTaskDelay(1);
    reg->wakeup = false;
    push_cfg(reg);
}

static void cfg_deinit(epd_config_register_t* reg) {
    //ESP_ERROR_CHECK(pca9555_set_config(reg->port, CFG_PIN_PWRGOOD | CFG_PIN_INT | CFG_PIN_VCOM_CTRL | CFG_PIN_PWRUP, 1));

    /* 
    int tries = 0;
    while (!((pca9555_read_input(reg->port, 1) & 0xC0) == 0x80)) {
        if (tries >= 500) {
            ESP_LOGE("epdiy", "failed to shut down TPS65185!");
            break;
        }
        tries++;
        vTaskDelay(1);
        printf("%X\n", pca9555_read_input(reg->port, 1));
    } */
    // Not sure why we need this delay, but the TPS65185 seems to generate an interrupt after some time that needs to be cleared.
    vTaskDelay(500);
    //pca9555_read_input(reg->port, 0);
    //pca9555_read_input(reg->port, 1);
    ESP_LOGI("epdiy", "going to sleep.");
}
