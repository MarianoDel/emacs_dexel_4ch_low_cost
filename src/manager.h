//----------------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANAGER.H ##########################################
//----------------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef __MANAGER_H_
#define __MANAGER_H_

#include "parameters.h"

// Exported Types Constants and Macros -----------------------------------------
void Manager (parameters_typedef * pmem);
void Manager_Timeouts (void);
void Manager_Probe_Temp_Set (void);
void Manager_Probe_Temp_Reset (void);
unsigned char Manager_Probe_Temp_Get (void);


// Exported Functions ----------------------------------------------------------


#endif    /* __MANAGER_H_ */

//--- end of file ---//

