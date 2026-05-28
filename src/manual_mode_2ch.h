//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANUAL_MODE_2CH.H #########################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _MANUAL_MODE_2CH_H_
#define _MANUAL_MODE_2CH_H_

//-- includes to help configurations --//
#include "switches_answers.h"
#include "parameters.h"


// Exported Types Constants and Macros -----------------------------------------



// Exported Module Functions ---------------------------------------------------
void ManualMode_2Ch_UpdateTimers (void);
void ManualMode_2Ch_Reset (void);
resp_t ManualMode_2Ch (parameters_typedef * mem, sw_actions_t action);



#endif    /* _MANUAL_MODE_2CH_H_ */

//--- end of file ---//
