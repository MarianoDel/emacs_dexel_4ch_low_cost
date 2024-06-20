//----------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### HARD.H #######################################
//----------------------------------------------------

// Define to prevent recursive inclusion ---------------------------------------
#ifndef HARD_H_
#define HARD_H_

#include "switches_answers.h"

//----------- Defines For Configuration -------------
//----------- Hardware Board Version -------------
#define HARDWARE_VERSION_2_1    // control board with encoder
// #define HARDWARE_VERSION_2_0    // new control board version


//----------- Firmware Version -------------------
#define FIRMWARE_VERSION_2_1    // current in decimal steps
// #define FIRMWARE_VERSION_2_0    // new control board version


//-------- Clock Frequency ------------------------------------
#define CLOCK_FREQ_64_MHZ    //ojo modificar a mano system_stm32g0xx.c SystemInit
// #define CLOCK_FREQ_16_MHZ    //ojo modificar a mano system_stm32g0xx.c SystemInit

//---- Features Configuration --------------------
#define USE_TEMP_PROT

// #define USE_CTROL_FAN_ALWAYS_ON    //fan always
#define USE_SW_UP_FOR_SW_DWN    // front panel error, change UP for DWN 28-04-2023

// --- Encoder Motion Direction --- //
// #define USE_ENCODER_DIRECT    //dt one on rising clk is CW (clockwise)
#define USE_ENCODER_INVERT    //dt one on rising clk is CCW (counter-clockwise)

//---- End of Features Configuration -------------

//-------- End Of Defines For Configuration ------


//--------- Sanity Checks ----------
#if (!defined HARDWARE_VERSION_2_1) && \
    (!defined HARDWARE_VERSION_2_0)
#error "define hardware version on hard.h"
#endif

#if (!defined FIRMWARE_VERSION_2_1) && \
    (!defined FIRMWARE_VERSION_2_0)
#error "define firmware version on hard.h"
#endif



// Gpios Configuration ------------------------------
#ifdef HARDWARE_VERSION_2_1
//GPIOA pin0    V_Sense_Ntc

//GPIOA pin1
#define CTRL_BKL    ((GPIOA->ODR & 0x0002) != 0)
#define CTRL_BKL_ON    (GPIOA->BSRR = 0x00000002)
#define CTRL_BKL_OFF    (GPIOA->BSRR = 0x00020000)

//GPIOA pin2    Usart2 tx
//GPIOA pin3	Usart2 rx

//GPIOA pin4
//GPIOA pin5
//GPIOA pin6
//GPIOA pin7    Lcd interface

//GPIOB pin0
#define LCD_E    ((GPIOB->ODR & 0x0001) != 0)
#define LCD_E_ON    (GPIOB->BSRR = 0x00000001)
#define LCD_E_OFF    (GPIOB->BSRR = 0x00010000)

//GPIOB pin1
#define LCD_RS    ((GPIOB->ODR & 0x0002) != 0)
#define LCD_RS_ON    (GPIOB->BSRR = 0x00000002)
#define LCD_RS_OFF    (GPIOB->BSRR = 0x00020000)

//GPIOB pin2
#define CTRL_FAN    ((GPIOB->ODR & 0x0004) != 0)
#define CTRL_FAN_ON    (GPIOB->BSRR = 0x00000004)
#define CTRL_FAN_OFF    (GPIOB->BSRR = 0x00040000)

//GPIOA pin8    Exti input
#define DMX_INPUT    ((GPIOA->IDR & 0x0100) != 0)

//GPIOA pin9
//GPIOC pin6    nc

//GPIOA pin10	Usart1 rx

//GPIOA pin11    nc

//GPIOA pin12
#define LED    ((GPIOA->ODR & 0x1000) != 0)
#define LED_ON    (GPIOA->BSRR = 0x00001000)
#define LED_OFF    (GPIOA->BSRR = 0x10000000)

//GPIOA pin13    swdio
//GPIOA pin14    swclk
//GPIOA pin15    nc

//GPIOB pin3
#define EN_CLK    ((GPIOB->IDR & 0x0008) == 0)

//GPIOB pin4
#define EN_DT    ((GPIOB->IDR & 0x0010) == 0)

//GPIOB pin5
#define EN_SW    ((GPIOB->IDR & 0x0020) == 0)

//GPIOB pin6    Usart1 Tx double function SELECT COLOR
//GPIOB pin7    Usart1 Rx double function SELECT COLOR

#endif     //#ifdef HARDWARE_VERSION_2_0


#ifdef HARDWARE_VERSION_2_0
//GPIOA pin0    V_Sense_Ntc

//GPIOA pin1
#define CTRL_BKL    ((GPIOA->ODR & 0x0002) != 0)
#define CTRL_BKL_ON    (GPIOA->BSRR = 0x00000002)
#define CTRL_BKL_OFF    (GPIOA->BSRR = 0x00020000)

//GPIOA pin2    Usart2 tx
//GPIOA pin3	Usart2 rx

//GPIOA pin4
//GPIOA pin5
//GPIOA pin6
//GPIOA pin7    Lcd interface

//GPIOB pin0
#define LCD_E    ((GPIOB->ODR & 0x0001) != 0)
#define LCD_E_ON    (GPIOB->BSRR = 0x00000001)
#define LCD_E_OFF    (GPIOB->BSRR = 0x00010000)

//GPIOB pin1
#define LCD_RS    ((GPIOB->ODR & 0x0002) != 0)
#define LCD_RS_ON    (GPIOB->BSRR = 0x00000002)
#define LCD_RS_OFF    (GPIOB->BSRR = 0x00020000)

//GPIOB pin2
#define CTRL_FAN    ((GPIOB->ODR & 0x0004) != 0)
#define CTRL_FAN_ON    (GPIOB->BSRR = 0x00000004)
#define CTRL_FAN_OFF    (GPIOB->BSRR = 0x00040000)

//GPIOA pin8    Exti input
#define DMX_INPUT    ((GPIOA->IDR & 0x0100) != 0)

//GPIOA pin9
//GPIOC pin6    nc

//GPIOA pin10	Usart1 rx

//GPIOA pin11

//GPIOA pin12
#define LED    ((GPIOA->ODR & 0x1000) != 0)
#define LED_ON    (GPIOA->BSRR = 0x00001000)
#define LED_OFF    (GPIOA->BSRR = 0x10000000)

//GPIOA pin13    swdio
//GPIOA pin14    swclk
//GPIOA pin15    nc

//GPIOB pin3
#define SW_SEL    ((GPIOB->IDR & 0x0008) == 0)

#ifdef USE_SW_UP_FOR_SW_DWN
//GPIOB pin4
#define SW_DWN    ((GPIOB->IDR & 0x0010) == 0)

//GPIOB pin5
#define SW_UP    ((GPIOB->IDR & 0x0020) == 0)
#else
//GPIOB pin4
#define SW_UP    ((GPIOB->IDR & 0x0010) == 0)

//GPIOB pin5
#define SW_DWN    ((GPIOB->IDR & 0x0020) == 0)
#endif

//GPIOB pin6    Usart1 Tx double function SELECT COLOR
//GPIOB pin7    Usart1 Rx double function SELECT COLOR

#endif     //#ifdef HARDWARE_VERSION_2_0



// // Exported Types Constants & Macros  ------------------------------------------
// //Estados Externos de LED BLINKING
// #define LED_NO_BLINKING               0
// #define LED_STANDBY                   1
// #define LED_GSM_NETWORK_LOW_RSSI      2
// #define LED_GSM_NETWORK_HIGH_RSSI     3
// #define LED_LOW_VOLTAGE               4
// #define LED_GSM_CMD_ERRORS            20

// typedef enum
// {
//     main_init = 0,
//     main_wait_for_gsm_network,
//     main_ready,
//     main_report_alarm_input,
//     main_report_panel_input,
//     main_report_buffer,
//     main_report_buffer_not_sended,
//     main_report_buffer_sended,
//     main_enable_output,
//     main_sms_not_sended

// } main_state_t;

// typedef enum {
//     resp_ok = 0,
//     resp_continue,
//     resp_selected,
//     resp_change,
//     resp_change_all_up,
//     resp_working,
//     resp_error,
//     resp_need_to_save,
//     resp_finish,
//     resp_nok,
//     resp_timeout,
//     resp_ready,
//     resp_no_answer,
//     resp_save

// } resp_t;


//--- Stringtify Utils -----------------------
#define str_macro(s) #s


// Module Exported Functions ---------------------------------------------------
void HARD_Timeouts (void);

#ifdef HARDWARE_VERSION_2_0
resp_sw_t Check_SW_UP (void);
resp_sw_t Check_SW_DWN (void);
resp_sw_t Check_SW_SEL (void);
void UpdateSwitches (void);
#endif

#ifdef HARDWARE_VERSION_2_1
resp_sw_t CheckSET (void);
unsigned char CheckCCW (void);
unsigned char CheckCW (void);
void UpdateSwitches (void);
void UpdateEncoder (void);
#endif

sw_actions_t CheckActions (void);

char * HARD_GetHardwareVersion (void);
char * HARD_GetSoftwareVersion (void);

#endif /* HARD_H_ */
