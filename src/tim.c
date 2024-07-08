//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32G030
// ##
// #### TIM.C ################################
//---------------------------------------------

// Includes --------------------------------------------------------------------
#include "tim.h"
#include "stm32g0xx.h"
#include "hard.h"


// Module Private Types Constants and Macros -----------------------------------
#define RCC_TIM1_CLK    (RCC->APBENR2 & 0x00000800)
#define RCC_TIM1_CLK_ON    (RCC->APBENR2 |= 0x00000800)
#define RCC_TIM1_CLK_OFF    (RCC->APBENR2 &= ~0x00000800)

#define RCC_TIM3_CLK    (RCC->APBENR1 & 0x00000002)
#define RCC_TIM3_CLK_ON    (RCC->APBENR1 |= 0x00000002)
#define RCC_TIM3_CLK_OFF    (RCC->APBENR1 &= ~0x00000002)

#define RCC_TIM6_CLK    (RCC->APBENR1 & 0x00000010)
#define RCC_TIM6_CLK_ON    (RCC->APBENR1 |= 0x00000010)
#define RCC_TIM6_CLK_OFF    (RCC->APBENR1 &= ~0x00000010)

#define RCC_TIM7_CLK    (RCC->APBENR1 & 0x00000020)
#define RCC_TIM7_CLK_ON    (RCC->APBENR1 |= 0x00000020)
#define RCC_TIM7_CLK_OFF    (RCC->APBENR1 &= ~0x00000020)

#define RCC_TIM14_CLK    (RCC->APBENR2 & 0x00008000)
#define RCC_TIM14_CLK_ON    (RCC->APBENR2 |= 0x00008000)
#define RCC_TIM14_CLK_OFF    (RCC->APBENR2 &= ~0x00008000)

#define RCC_TIM15_CLK    (RCC->APBENR2 & 0x00010000)
#define RCC_TIM15_CLK_ON    (RCC->APBENR2 |= 0x00010000)
#define RCC_TIM15_CLK_OFF    (RCC->APBENR2 &= ~0x00010000)

#define RCC_TIM16_CLK    (RCC->APBENR2 & 0x00020000)
#define RCC_TIM16_CLK_ON    (RCC->APBENR2 |= 0x00020000)
#define RCC_TIM16_CLK_OFF    (RCC->APBENR2 &= ~0x00020000)

#define RCC_TIM17_CLK    (RCC->APBENR2 & 0x00040000)
#define RCC_TIM17_CLK_ON    (RCC->APBENR2 |= 0x00040000)
#define RCC_TIM17_CLK_OFF    (RCC->APBENR2 &= ~0x00040000)

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
volatile unsigned short wait_ms_var = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void Update_TIM3_CH1 (unsigned short a)
{
    TIM3->CCR1 = a;
}

void Update_TIM3_CH2 (unsigned short a)
{
    TIM3->CCR2 = a;
}

void Update_TIM3_CH3 (unsigned short a)
{
    TIM3->CCR3 = a;
}

void Update_TIM3_CH4 (unsigned short a)
{
    TIM3->CCR4 = a;
}

void Wait_ms (unsigned short wait)
{
    wait_ms_var = wait;
    while (wait_ms_var);
}

//-------------------------------------------//
// @brief  TIM configure.
// @param  None
// @retval None
//------------------------------------------//
void TIM3_IRQHandler (void)	//1 ms
{
    if (TIM3->SR & 0x01)	//bajo el flag
        TIM3->SR = 0x00;
}


void TIM_3_Init (void)
{
    if (!RCC_TIM3_CLK)
        RCC_TIM3_CLK_ON;

    //Configuracion del timer.
    TIM3->CR1 = 0x00;		//clk int / 1; upcounting
    TIM3->CR2 |= TIM_CR2_MMS_1;		//UEV -> TRG0

    TIM3->CCMR1 = 0x6060;    // CH2 CH1 pwm mode 2
    TIM3->CCMR2 = 0x6060;    // CH4 CH3 pwm mode 2
    TIM3->CCER |= TIM_CCER_CC4E | TIM_CCER_CC3E | TIM_CCER_CC2E | TIM_CCER_CC1E;    //CH1 enable on pin

    // TIM3->ARR = DUTY_100_PERCENT;
    TIM3->ARR = 4064;    
    TIM3->CNT = 0;

#if (defined USE_FREQ_16KHZ) || (defined USE_FREQ_12KHZ)
    TIM3->PSC = 0;
#elif (defined USE_FREQ_8KHZ) || (defined USE_FREQ_6KHZ)
    TIM3->PSC = 1;
#elif defined USE_FREQ_4KHZ
    TIM3->PSC = 2;
#elif defined USE_FREQ_4_8KHZ
    TIM3->PSC = 9;
#else
#error "set freq on hard.h"
#endif
    
    //Alternative pin config.
    //Alternate Fuction
    unsigned int temp;
    temp = GPIOA->AFR[0];
    temp &= 0x00FFFFFF;
    temp |= 0x11000000;    // PA7 -> AF1 PA6 -> AF1
    GPIOA->AFR[0] = temp;

    temp = GPIOB->AFR[0];
    temp &= 0xFFFFFF00;
    temp |= 0x00000011;    // PB1 -> AF1 PB0 -> AF1
    GPIOB->AFR[0] = temp;

    // Enable timer ver UDIS
    //TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;

}

#ifdef STM32G070xx
void TIM_6_Init (void)
{
    if (!RCC_TIM6_CLK)
        RCC_TIM6_CLK_ON;

    //Configuracion del timer.
    TIM6->CR1 = 0x00;		//clk int / 1; upcounting
    TIM6->PSC = 47;			//tick cada 1us
    TIM6->ARR = 0xFFFF;			//para que arranque
    //TIM6->CR1 |= TIM_CR1_CEN;
}
#endif

void TIM14_IRQHandler (void)	//100uS
{
    if (TIM14->SR & 0x01)
        TIM14->SR = 0x00;
}


void TIM_14_Init (void)
{
    if (!RCC_TIM14_CLK)
        RCC_TIM14_CLK_ON;

    // Timer Config.
    TIM14->CR1 = 0x00;    // clk int / 1; upcounting;
    TIM14->PSC = 63;    // tick 1us
    TIM14->ARR = 0xFFFF;    // for start, free running
    TIM14->EGR |= 0x0001;
}

void TIM16_IRQHandler (void)	//es one shoot
{
    if (TIM16->SR & 0x01)
        TIM16->SR = 0x00;
}


void TIM_16_Init (void)
{
    if (!RCC_TIM16_CLK)
        RCC_TIM16_CLK_ON;

    //Configuracion del timer.
    TIM16->CR1 = 0x00;		//clk int / 1; upcounting; uev
    TIM16->ARR = 0xFFFF;
    TIM16->CNT = 0;
    //TIM16->PSC = 7999;	//tick 1ms
    //TIM16->PSC = 799;	//tick 100us
    TIM16->PSC = 47;			//tick 1us
    TIM16->EGR = TIM_EGR_UG;

    // Enable timer ver UDIS
    //	TIM16->DIER |= TIM_DIER_UIE;
    //	TIM16->CR1 |= TIM_CR1_CEN;
}

void OneShootTIM16 (unsigned short a)
{
    TIM16->ARR = a;
    TIM16->CR1 |= TIM_CR1_CEN;
}

void TIM16Enable (void)
{
    TIM16->CR1 |= TIM_CR1_CEN;
}

void TIM16Disable (void)
{
    TIM16->CR1 &= ~TIM_CR1_CEN;
}


void TIM17_IRQHandler (void)	//200uS
{
    if (TIM17->SR & 0x01)
        TIM17->SR = 0x00;		//bajar flag
}


void TIM_17_Init (void)
{
    if (!RCC_TIM17_CLK)
        RCC_TIM17_CLK_ON;

    //Configuracion del timer.
    TIM17->ARR = 400;		//400us
    TIM17->CNT = 0;
    TIM17->PSC = 47;

    // Enable timer interrupt ver UDIS
    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->CR1 |= TIM_CR1_URS | TIM_CR1_CEN;	//solo int cuando hay overflow y one shot

    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, 8);
}


void TIM_Timeouts (void)
{
    if (wait_ms_var)
        wait_ms_var--;
}


//--- end of file ---//
