//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32G030
// ##
// #### TIM.H ################################
//---------------------------------------------

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H
#define __TIM_H

// Exported Types Constants and Macros -----------------------------------------
#include "hard.h"    // get freq configuration

#ifdef USE_FREQ_16KHZ
#define DUTY_100_PERCENT	3000
#endif

#ifdef USE_FREQ_12KHZ
#define DUTY_100_PERCENT	4000
#endif

#ifdef USE_FREQ_8KHZ
#define DUTY_100_PERCENT	3000
#endif

#ifdef USE_FREQ_6KHZ
#define DUTY_100_PERCENT	4000
#endif

#ifdef USE_FREQ_4KHZ
#define DUTY_100_PERCENT	4000
#endif


//--- Exported constants ---//

//--- Exported macro ---//

// Module Exported Functions ---------------------------------------------------
// void TIM3_IRQHandler (void);
void TIM_3_Init(void);
void TIM_6_Init (void);
void TIM14_IRQHandler (void);
void TIM_14_Init(void);
void TIM16_IRQHandler (void);
void TIM_16_Init(void);
void TIM17_IRQHandler (void);
void TIM_17_Init(void);
void Update_TIM3_CH1 (unsigned short);
void Update_TIM3_CH2 (unsigned short);
void Update_TIM3_CH3 (unsigned short);
void Update_TIM3_CH4 (unsigned short);

void OneShootTIM16 (unsigned short);
void TIM16Enable (void);
void TIM16Disable (void);

void Wait_ms (unsigned short);
void TIM_Timeouts (void);


#endif
//--- End ---//


//--- END OF FILE ---//
