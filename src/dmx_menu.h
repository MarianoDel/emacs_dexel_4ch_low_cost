//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_MENU.H ##############################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _DMX_MENU_H_
#define _DMX_MENU_H_

//-- includes to help configurations --//
#include "switches_answers.h"
#include "parameters.h"


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void Dmx_Menu_Timeout (void);
void Dmx_Menu_Reset (void);
resp_t Dmx_Menu (parameters_typedef *, sw_actions_t);

#endif    /* _DMX_MENU_H_ */

//--- end of file ---//

