
#define D7 GPIO_NUM_40
#define D6 GPIO_NUM_39
#define D5 GPIO_NUM_38
#define D4 GPIO_NUM_37
#define D3 GPIO_NUM_36
#define D2 GPIO_NUM_35
#define D1 GPIO_NUM_34
#define D0 GPIO_NUM_33
/* CFG lines */
#define CFG_DATA GPIO_NUM_41
#define CFG_CLK GPIO_NUM_18
#define CFG_STR GPIO_NUM_19

/* Control Lines */
#define CKV GPIO_NUM_42
#define STH GPIO_NUM_45

/* TPS65185 lines */
#define TPS_PIN_PWRUP     GPIO_NUM_17
#define TPS_PIN_PWRGOOD   GPIO_NUM_20
// All the rest is I2C communication

// LE 
#define V4_LATCH_ENABLE GPIO_NUM_14

/* I2C to talk with PMIC. todo: Find right pins for this in S2 */
#define CFG_SCL GPIO_NUM_18
#define CFG_SDA GPIO_NUM_19
#define CFG_INTR GPIO_NUM_1
#define EPDIY_I2C_PORT I2C_NUM_0


/* Edges */
#define CKH GPIO_NUM_5
