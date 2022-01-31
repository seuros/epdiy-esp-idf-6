#if defined(CONFIG_EPD_BOARD_REVISION_V5) || defined(CONFIG_EPD_BOARD_REVISION_V6)
#define D7 GPIO_NUM_23
#define D6 GPIO_NUM_22
#define D5 GPIO_NUM_21
#define D4 GPIO_NUM_19
#define D3 GPIO_NUM_18
#define D2 GPIO_NUM_5
#define D1 GPIO_NUM_4
#define D0 GPIO_NUM_25

/* Control Lines */
#define CKV GPIO_NUM_26
#define STH GPIO_NUM_27

#define V4_LATCH_ENABLE GPIO_NUM_2

/* Edges */
#define CKH GPIO_NUM_15

#if defined(CONFIG_EPD_BOARD_REVISION_V5)
/* Config Reggister Control */
#define CFG_DATA GPIO_NUM_33
#define CFG_CLK GPIO_NUM_32
#define CFG_STR GPIO_NUM_0
#endif
#if defined(CONFIG_EPD_BOARD_REVISION_V6)
#define CFG_SCL GPIO_NUM_33
#define CFG_SDA GPIO_NUM_32
#define CFG_INTR GPIO_NUM_35

#define EPDIY_I2C_PORT I2C_NUM_0
#endif

#else
#define D7 GPIO_NUM_22
#define D6 GPIO_NUM_21
#define D5 GPIO_NUM_27
#define D4 GPIO_NUM_2
#if defined(CONFIG_EPD_BOARD_REVISION_LILYGO_T5_47)
#define D3 GPIO_NUM_19
#else
#define D3 GPIO_NUM_0
#endif
#define D2 GPIO_NUM_4
#define D1 GPIO_NUM_32
#define D0 GPIO_NUM_33

#define CFG_DATA GPIO_NUM_23
#define CFG_CLK GPIO_NUM_18
#if defined(CONFIG_EPD_BOARD_REVISION_LILYGO_T5_47)
#define CFG_STR GPIO_NUM_0
#else
#define CFG_STR GPIO_NUM_19
#endif

/* Control Lines */
#define CKV GPIO_NUM_25
#define STH GPIO_NUM_26

#define V4_LATCH_ENABLE GPIO_NUM_15

/* Edges */
#define CKH GPIO_NUM_5

#endif