//-----------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MENU_OPTIONS_OLED.H #####################
//-----------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _MENU_OPTIONS_OLED_H_
#define _MENU_OPTIONS_OLED_H_

#include "switches_answers.h"


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void Options_Up_Dwn_Select_Reset (void);
resp_t Options_Up_Dwn_Select (sw_actions_t actions);
resp_t Options_Up_Dwn_Next (sw_actions_t actions);

    
#endif    /* _MENU_OPTIONS_OLED_H_ */

//--- end of file ---//

