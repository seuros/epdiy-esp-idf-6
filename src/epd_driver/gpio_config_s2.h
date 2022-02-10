
#define D7 GPIO_NUM_45
#define D6 GPIO_NUM_42
#define D5 GPIO_NUM_41
#define D4 GPIO_NUM_40
#define D3 GPIO_NUM_39
#define D2 GPIO_NUM_38
#define D1 GPIO_NUM_37
#define D0 GPIO_NUM_36

/* EPD Control Lines */
#define CKV GPIO_NUM_35
#define STH GPIO_NUM_34

#define EPD_STV  GPIO_NUM_33
#define EPD_MODE GPIO_NUM_17
#define EPD_OE   GPIO_NUM_18
#define V4_LATCH_ENABLE GPIO_NUM_20

/* TPS65185 5 control lines */
#define TPS_PWRGOOD       GPIO_NUM_46
#define TPS_WAKEUP        GPIO_NUM_18
#define TPS_PWRUP         GPIO_NUM_16
#define TPS_INTERRUPT     GPIO_NUM_11
#define TPS_VCOM_CTRL     GPIO_NUM_10
// All the rest is I2C communication

/* I2C to talk with PMIC */
#define CFG_SCL GPIO_NUM_8
#define CFG_SDA GPIO_NUM_9
// CFG_INTR does not exist anymore since there is no more PCA9555 IO expander interrupts
#define EPDIY_I2C_PORT I2C_NUM_0

/* Edges */
#define CKH GPIO_NUM_0
