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

// Debug when TPS is not yet soldered
//#define SKIP_TPS_INIT
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
    ets_printf("INT low\n");
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
    // Why it was doing this in V6?
    //pca9555_read_input(EPDIY_I2C_PORT, 1);
	ival = tps_read_register(EPDIY_I2C_PORT, TPS_REG_INT1);
    
    int int2 = tps_read_register(EPDIY_I2C_PORT, TPS_REG_INT2);
    printf("INT1: %d\n", ival);
    printf("INT2: %d\n", int2);
	ival |= int2 << 8;
    while (!gpio_get_level(TPS_INTERRUPT)) {vTaskDelay(1); }
    return ival;
}

void config_reg_init(epd_config_register_t* reg) {

    printf("config_reg_init start\n");
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
    /*  
     TPS_VCOM_CTRL     Output 
     TPS_WAKEUP        Output
     TPS_PWRUP         Output
     TPS_INTERRUPT     Input
     TPS_PWRGOOD       Input
     */
    // Control lines are now controlled directly  (1->4) by S2
    // Also TPS65185 control pins are set up here (4->8)
    gpio_num_t EP_CONTROL[] = {STH, EPD_STV, EPD_MODE, EPD_OE, TPS_WAKEUP, TPS_PWRUP, TPS_VCOM_CTRL};
   
    for (int x = 0; x < 7; x++) {
        printf("IO %d to LOW (loop:%d)\n", (int)EP_CONTROL[x], x);
        gpio_set_direction(EP_CONTROL[x], GPIO_MODE_OUTPUT);
        gpio_set_level(EP_CONTROL[x], 0);
    }
    gpio_set_direction(TPS_PWRGOOD, GPIO_MODE_INPUT);
    // TODO: Interrupts should be added for incoming signals from TPS65185
    gpio_set_direction(TPS_INTERRUPT, GPIO_MODE_INPUT);
    gpio_set_intr_type(TPS_INTERRUPT, GPIO_INTR_NEGEDGE);
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TPS_INTERRUPT, interrupt_handler, (void *) TPS_INTERRUPT));
}

/*
 Important: Removing PCA9555 means that we should push this config differently
*/
static void push_cfg(epd_config_register_t* reg) {

    #ifndef SILENT_CFG
        //printf("OE:%d MODE:%d STV:%d\n", reg->ep_output_enable,reg->ep_mode,reg->ep_stv);
    #endif
    
    if (reg->ep_output_enable) {
    gpio_set_level(EPD_OE, reg->ep_output_enable);
    }
    
    gpio_set_level(EPD_MODE, reg->ep_mode);
    
    gpio_set_level(EPD_STV, reg->ep_stv);

    // This is signal for the PMIC tps65185
    gpio_set_level(TPS_PWRUP, reg->pwrup);
    #ifndef SILENT_CFG
    printf(" CFG_PIN_PWRUP %d", reg->pwrup);
    #endif
    
    // This is signal for the PMIC tps65185
    gpio_set_level(TPS_VCOM_CTRL, reg->vcom_ctrl);
    #ifndef SILENT_CFG
    printf(" CFG_PIN_VCOM_CTRL %d", reg->vcom_ctrl);
    #endif
    
    // This is signal for the PMIC tps65185
    gpio_set_level(TPS_WAKEUP, reg->wakeup);
    #ifndef SILENT_CFG
    printf(" CFG_PIN_WAKEUP %d ", reg->wakeup);
    #endif
}

static void cfg_poweron(epd_config_register_t* reg) {
    printf("cfg_poweron!\n");
    reg->ep_stv = true;
    reg->wakeup = true;
    push_cfg(reg);
    vTaskDelay(2);
    printf("wake INT:%d\n\n", v6_wait_for_interrupt(10));
    vTaskDelay(10);

    reg->pwrup = true;
    push_cfg(reg);
        vTaskDelay(2);
    printf("pwr INT:%d\n\n", v6_wait_for_interrupt(10));

    reg->vcom_ctrl = true;
    push_cfg(reg);

    // give the IC time to powerup and set lines
    vTaskDelay(10);

    printf("\nThermistor °C: %d\n", tps_read_thermistor(EPDIY_I2C_PORT));
    printf("Waiting for TPS_PWRGOOD...\n");
    // while (!(pca9555_read_input(reg->port, 1) & CFG_PIN_PWRGOOD)) {
    while (gpio_get_level(TPS_PWRGOOD) == 0) {
        vTaskDelay(5);
    }
    printf("TPS_PWRGOOD ok\n");

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
