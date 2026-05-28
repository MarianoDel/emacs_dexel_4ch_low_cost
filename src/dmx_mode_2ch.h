//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_MODE_2CH.H ############################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _DMX_MODE_2CH_H_
#define _DMX_MODE_2CH_H_

//-- includes to help configurations --//
#include "switches_answers.h"
#include "parameters.h"


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void Dmx_Mode_2Ch_UpdateTimers (void);
void Dmx_Mode_2Ch_Reset (void);
resp_t Dmx_Mode_2Ch (parameters_typedef *, sw_actions_t);
unsigned short Dmx_Mode_2Ch_GetPacketsTimer (void);

#endif    /* _DMX_MODE_2CH_H_ */

//--- end of file ---//

