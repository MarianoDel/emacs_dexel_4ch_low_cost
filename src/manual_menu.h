//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANUAL_MENU.H ##############################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _MANUAL_MENU_H_
#define _MANUAL_MENU_H_

//-- includes to help configurations --//
#include "switches_answers.h"
#include "parameters.h"


// Module Exported Types Constants and Macros ----------------------------------
#define MANUAL_RED    0x01
#define MANUAL_GREEN    0x02
#define MANUAL_BLUE    0x04
#define MANUAL_WARM    0x08
#define MANUAL_COLD    0x10



// Module Exported Functions ---------------------------------------------------
void Manual_Menu_Timeouts (void);
void Manual_Menu_Reset (void);
resp_t Manual_Menu (parameters_typedef *, sw_actions_t);

#endif    /* _MANUAL_MENU_H_ */

//--- end of file ---//

