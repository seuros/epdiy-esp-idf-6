#if defined(CONFIG_EPD_BOARD_REVISION_V5)
#include "driver/gpio.h"

#define D7 GPIO_NUM_23
#define D6 GPIO_NUM_22
#define D5 GPIO_NUM_21
#define D4 GPIO_NUM_19
#define D3 GPIO_NUM_18
#define D2 GPIO_NUM_5
#define D1 GPIO_NUM_4
#define D0 GPIO_NUM_25


/* Config Reggister Control */
#define CFG_DATA GPIO_NUM_33
#define CFG_CLK GPIO_NUM_32
#define CFG_STR GPIO_NUM_0

/* Control Lines */
#define CKV GPIO_NUM_26
#define STH GPIO_NUM_27


#define V4_LATCH_ENABLE GPIO_NUM_2

/* Edges */
#define CKH GPIO_NUM_15

#else
#define D7 GPIO_NUM_42
#define D6 GPIO_NUM_21
#define D5 GPIO_NUM_35
#define D4 GPIO_NUM_2
#if defined(CONFIG_EPD_BOARD_REVISION_LILYGO_T5_47)
#define D3 GPIO_NUM_19
#else
#define D3 GPIO_NUM_0
#endif
#define D2 GPIO_NUM_37
#define D1 GPIO_NUM_36
#define D0 GPIO_NUM_33

#define CFG_DATA GPIO_NUM_38
#define CFG_CLK GPIO_NUM_18
#if defined(CONFIG_EPD_BOARD_REVISION_LILYGO_T5_47)
#define CFG_STR GPIO_NUM_1
#else
#define CFG_STR GPIO_NUM_19
#endif

/* Control Lines */
#define CKV GPIO_NUM_45
#define STH GPIO_NUM_20

#define V4_LATCH_ENABLE GPIO_NUM_14

/* Edges */
#define CKH GPIO_NUM_5

#endif