//-----------------------------------------------
// #### DEXEL 2CH PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MENUES.H ################################
//-----------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _MENUES_H_
#define _MENUES_H_

#include "switches_answers.h"
#include "parameters.h"

// Exported Types --------------------------------------------------------------
#define mem_bkp_t parameters_typedef

// Exported Constants or Macros ------------------------------------------------
#define MINIMUN_TIME_ALLOWED    1
#define MAXIMUN_TIME_ALLOWED    100


// Exported Functions ----------------------------------------------------------
void MENU_Main_Reset (void);
resp_t MENU_Main (mem_bkp_t *, sw_actions_t);
unsigned char ConvertCurrentFromMemory (mem_bkp_t * config);
    
#endif    /* _MENUES_H_ */

//--- end of file ---//

