//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_RECEIVER.H ########################
//---------------------------------------------

#ifndef _DMX_RECEIVER_H_
#define _DMX_RECEIVER_H_


#define SIZEOF_DMX_BUFFER_DATA    6    //max hardware channels + 1 dmx/rdm packet + 1 grandmaster





// Module Exported Functions ---------------------------------------------------
void DMX_Int_Break_Handler (void);
void DMX_Int_Serial_Receiver_Handler (unsigned char);
void DMX_Int_Millis_Handler (void);

void DMX_EnableRx (void);
void DMX_DisableRx (void);

#endif    /* DMX_RECEIVER_H_ */

//--- end of file ---//

