//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32G030
// ##
// #### GPIO.C ################################
//---------------------------------------------

// Includes --------------------------------------------------------------------
#include "stm32g0xx.h"
#include "gpio.h"
#include "hard.h"


// Module Private Types Constants and Macros -----------------------------------
// - Port enables
#define GPIOA_ENABLE
#define GPIOB_ENABLE
#define GPIOC_ENABLE

#define WITH_EXTI

// - Ports Clocks
#define GPIOA_CLK    (RCC->IOPENR & 0x00000001)
#define GPIOA_CLK_ON    (RCC->IOPENR |= 0x00000001)
#define GPIOA_CLK_OFF    (RCC->IOPENR &= ~0x00000001)

#define GPIOB_CLK    (RCC->IOPENR & 0x00000002)
#define GPIOB_CLK_ON    (RCC->IOPENR |= 0x00000002)
#define GPIOB_CLK_OFF    (RCC->IOPENR &= ~0x00000002)

#define GPIOC_CLK    (RCC->IOPENR & 0x00000004)
#define GPIOC_CLK_ON    (RCC->IOPENR |= 0x00000004)
#define GPIOC_CLK_OFF    (RCC->IOPENR &= ~0x00000004)

#define GPIOD_CLK    (RCC->IOPENR & 0x00000008)
#define GPIOD_CLK_ON    (RCC->IOPENR |= 0x00000008)
#define GPIOD_CLK_OFF    (RCC->IOPENR &= ~0x00000008)

#define GPIOF_CLK    (RCC->IOPENR & 0x00000020)
#define GPIOF_CLK_ON    (RCC->IOPENR |= 0x00000020)
#define GPIOF_CLK_OFF    (RCC->IOPENR &= ~0x00000020)

// #define SYSCFG_CLK    (RCC->APBENR2 & 0x00000001)
// #define SYSCFG_CLK_ON    (RCC->APBENR2 |= 0x00000001)
// #define SYSCFG_CLK_OFF    (RCC->APBENR2 &= ~0x00000001)


//-------------------------------------------//
// @brief  GPIO configure.
// @param  None
// @retval None
//------------------------------------------//
void GPIO_Config (void)
{
    //--- MODER ---//
    //00: Input mode 
    //01: General purpose output mode
    //10: Alternate function mode
    //11: Analog mode (reset state)

    //--- OTYPER ---//
    //These bits are written by software to configure the I/O output type.
    //0: Output push-pull (reset state)
    //1: Output open-drain

    //--- ORSPEEDR ---//
    //These bits are written by software to configure the I/O output speed.
    //x0: Low speed.
    //01: Medium speed.
    //11: High speed.
    //Note: Refer to the device datasheet for the frequency.

    //--- PUPDR ---//
    //These bits are written by software to configure the I/O pull-up or pull-down
    //00: No pull-up, pull-down
    //01: Pull-up
    //10: Pull-down
    //11: Reserved

    unsigned long temp;

    //--- GPIO A ---//
#ifdef GPIOA_ENABLE    
    if (!GPIOA_CLK)
        GPIOA_CLK_ON;
    
    temp = GPIOA->MODER;    // 2 bits per pin
    temp &= 0xFCCC0F00;    // PA0 analog; PA1 input; PA2 - PA3 alternative; PA6 - PA7 alternative
    temp |= 0x0120A5A3;    // PA8 input exti; PA10 alternative; PA12 output
    GPIOA->MODER = temp;

    temp = GPIOA->OTYPER;    // 1 bit per pin
    temp &= 0xFFFFFFFF;    //
    temp |= 0x00000000;
    GPIOA->OTYPER = temp;
    
    temp = GPIOA->OSPEEDR;	//2 bits per pin
    temp &= 0xFCFF000F;    // PA2 - PA7 low speed
    temp |= 0x00000000;    // PA12 low speed
    GPIOA->OSPEEDR = temp;

    temp = GPIOA->PUPDR;	//2 bits per pin
    temp &= 0xFFFFFF33;    // PA1 pullup; PA3 pullup
    temp |= 0x00000044;
    GPIOA->PUPDR = temp;
#endif    //GPIOA_ENABLE
    
    //--- GPIO B ---//
#ifdef GPIOB_ENABLE    
    if (!GPIOB_CLK)
        GPIOB_CLK_ON;
    
    temp = GPIOB->MODER;    // 2 bits per pin
    temp &= 0xFFFC3FC0;    // PB0 - PB1 alternative; PB2 output; PB7 alternative
    temp |= 0x0002801A;    // PB8 alternative
    GPIOB->MODER = temp;

    temp = GPIOB->OTYPER;	//1 bit per pin
    temp &= 0xFFFFFE7F;    // PB8 PB7 open drain
    temp |= 0x00000180;
    GPIOB->OTYPER = temp;

    temp = GPIOB->OSPEEDR;    // 2 bits per pin
    temp &= 0xFFFC3FC0;    // PB0 PB1 PB2 low speed; PB7 low speed
    temp |= 0x00000000;    // PB8 low speed
    GPIOB->OSPEEDR = temp;

    temp = GPIOB->PUPDR;    // 2 bits per pin
    temp &= 0xFFFC3FFF;    // PB7 PB8 pullup
    temp |= 0x00014000;
    GPIOB->PUPDR = temp;
#endif    //GPIOB_ENABLE
    
    //--- GPIO C ---//
#ifdef GPIOC_ENABLE
    if (!GPIOC_CLK)
        GPIOC_CLK_ON;

    temp = GPIOC->MODER;    // 2 bits per pin
    temp &= 0xFFFFCFFF;    // PC6 input
    temp |= 0x00000000;
    GPIOC->MODER = temp;

    temp = GPIOC->OTYPER;    //1 bit per pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOC->OTYPER = temp;

    temp = GPIOC->OSPEEDR;    //2 bits per pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOC->OSPEEDR = temp;

    temp = GPIOC->PUPDR;    //2 bits per pin
    temp &= 0xFFFFCFFF;    // PC6 pullup
    temp |= 0x00001000;
    GPIOC->PUPDR = temp;
#endif    // GPIOC_ENABLE
    
    
#ifdef WITH_EXTI
    //Interrupt on PA8
    // if (!SYSCFG_CLK)
    //     SYSCFG_CLK_ON;

    // SYSCFG->EXTICR[0] = 0x00000000; //Select Port A
    // SYSCFG->EXTICR[1] = 0x00000000; //Select Port A
    EXTI->IMR1 |= 0x0100;    //Corresponding mask bit for interrupts PA8
    EXTI->EMR1 |= 0x0000;    //Corresponding mask bit for events
    EXTI->RTSR1 |= 0x0100;    //Pin Interrupt line on rising edge PA8
    EXTI->FTSR1 |= 0x0100;    //Pin Interrupt line on falling edge PA8

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 3);
#endif
}


inline void EXTIOff (void)
{
    EXTI->IMR1 &= ~0x00000100;
}


inline void EXTIOn (void)
{
    EXTI->IMR1 |= 0x00000100;
}

//--- end of file ---//
