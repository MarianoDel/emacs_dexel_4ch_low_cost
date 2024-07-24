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
#define Options_Up_Dwn_Select(X)    Options_Up_Dwn_Select_Out((X),(0))
#define Options_Up_Dwn_Out(X)    Options_Up_Dwn_Select_Out((X),(1))
#define Options_Up_Dwn_Next(X)     Options_Up_Dwn_Select_Out((X),(2))

// Module Exported Functions ---------------------------------------------------
void Options_Up_Dwn_Select_Reset (void);
resp_t Options_Up_Dwn_Select_Out (sw_actions_t actions, unsigned char out);

    
#endif    /* _MENU_OPTIONS_OLED_H_ */

//--- end of file ---//

