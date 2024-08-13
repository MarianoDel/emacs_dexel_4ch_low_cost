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
#define HARDWARE_VERSION_1_0    // prototipe board


//----------- Firmware Version -------------------
#define FIRMWARE_VERSION_1_0


//-------- Clock Frequency ------------------------------------
#define CLOCK_FREQ_64_MHZ    // beware! modify by hand system_stm32g0xx.c SystemInit
// #define CLOCK_FREQ_16_MHZ    // beware! modify by hand system_stm32g0xx.c SystemInit

//---- Features Configuration --------------------
#define USE_TEMP_PROT

// #define USE_CTROL_FAN_ALWAYS_ON    //fan always

// --- PWM Frequency --- //
// #define USE_FREQ_16KHZ    //48MHz / 3000pts
// #define USE_FREQ_12KHZ    //48MHz / 4000pts
#define USE_FREQ_8KHZ    //24MHz / 3000pts
// #define USE_FREQ_6KHZ    //24MHz / 4000pts
// #define USE_FREQ_4KHZ    //16MHz / 4000pts
// #define USE_FREQ_4_8KHZ    //4.8MHz / 1000pts

// --- Usart2 Modes --- //
#define USART2_DEBUG_MODE

//---- End of Features Configuration -------------

//-------- End Of Defines For Configuration ------


//--------- Sanity Checks ----------
#if (!defined HARDWARE_VERSION_1_0) && \
    (!defined HARDWARE_VERSION_2_0)
#error "define hardware version on hard.h"
#endif

#if (!defined FIRMWARE_VERSION_1_0) && \
    (!defined FIRMWARE_VERSION_2_0)
#error "define firmware version on hard.h"
#endif



// Gpios Configuration ------------------------------
#ifdef HARDWARE_VERSION_1_0
//GPIOA pin0    LM335

//GPIOA pin1
#define S2    ((GPIOA->IDR & 0x0002) == 0)

//GPIOA pin2    Usart2 tx
//GPIOA pin3	Usart2 rx

//GPIOA pin4
//GPIOA pin5    NC

//GPIOA pin6    TIM3_CH1
//GPIOA pin7    TIM3_CH2
//GPIOB pin0    TIM3_CH3
//GPIOB pin1    TIM3_CH4

//GPIOB pin2
#define CTRL_FAN    ((GPIOB->ODR & 0x0004) != 0)
#define CTRL_FAN_ON    (GPIOB->BSRR = 0x00000004)
#define CTRL_FAN_OFF    (GPIOB->BSRR = 0x00040000)

//GPIOA pin8    Exti input
#define DMX_INPUT    ((GPIOA->IDR & 0x0100) != 0)

//GPIOA pin9    NC

//GPIOC pin6
#define S1    ((GPIOC->IDR & 0x0040) == 0) 

#define CTRL_BKL    ((GPIOA->ODR & 0x0002) != 0)
#define CTRL_BKL_ON    (GPIOA->BSRR = 0x00000002)
#define CTRL_BKL_OFF    (GPIOA->BSRR = 0x00020000)

//GPIOA pin10	Usart1 rx

//GPIOA pin11    NC

//GPIOA pin12
#define LED    ((GPIOA->ODR & 0x1000) != 0)
#define LED_ON    (GPIOA->BSRR = 0x00001000)
#define LED_OFF    (GPIOA->BSRR = 0x10000000)

//GPIOA pin13    swdio
//GPIOA pin14    swclk
//GPIOA pin15    nc

//GPIOB pin3
//GPIOB pin4
//GPIOB pin5
//GPIOB pin6

//GPIOB pin7    Usart1 Rx double function SELECT COLOR

#endif     //#ifdef HARDWARE_VERSION_1_0





// Exported Types Constants & Macros  ------------------------------------------


//--- Stringtify Utils -----------------------
#define str_macro(s) #s


// Module Exported Functions ---------------------------------------------------
void HARD_Timeouts (void);

resp_sw_t Check_S1 (void);
resp_sw_t Check_S2 (void);
void Check_S1_Accel_Fast (void);
void Check_S1_Accel_Slow (void);
void Check_S2_Accel_Fast (void);
void Check_S2_Accel_Slow (void);
sw_actions_t CheckActions (void);

void HARD_UpdateSwitches (void);
char * HARD_GetHardwareVersion (void);
char * HARD_GetSoftwareVersion (void);

#endif    /* HARD_H_ */
