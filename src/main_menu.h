//-----------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MAIN_MENU.H ################################
//-----------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

#include "switches_answers.h"
#include "parameters.h"

// Exported Types --------------------------------------------------------------
#define mem_bkp_t parameters_typedef

// Exported Constants or Macros ------------------------------------------------
// #define MINIMUN_TIME_ALLOWED    1
// #define MAXIMUN_TIME_ALLOWED    100


// Exported Functions ----------------------------------------------------------
void Main_Menu_Reset (void);
resp_t Main_Menu (mem_bkp_t *, sw_actions_t);
// unsigned char ConvertCurrentFromMemory (mem_bkp_t * config);
    
#endif    /* _MAIN_MENU_H_ */

//--- end of file ---//

