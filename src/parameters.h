//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PARAMETERS.H ##########################
//---------------------------------------------

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

//-- Configuration for Struct --------------------
typedef enum {
    // program types (modes)
    DMX_MODE = 0,
    MANUAL_MODE,
    AUTODETECT_MODE
    
} programs_type_e;


typedef enum {

    CCT1_MODE = 0,
    CCT2_MODE,
    ONECH_MODE
    
} channels_operation_mode_e;


//-- Memory Struct to Save --------------------
// Beware this struct must be 4 bytes align and be multiple of 8
typedef struct parameters {

    //-- Program Type ----
    unsigned char program_type;

    //-- DMX mode configs. ----                  
    unsigned char dmx_channel_quantity;
    unsigned char dummy1 [2];
    unsigned short dmx_first_channel;    // 2 bytes if its aligned to 4

    //-- For Manual mode settings ----   //6
    unsigned char fixed_channels [2];

    //-- For Max Current Channels settings ----   //8
    unsigned char max_current_channels [2];    
    
    //-- Current Set ----    //10
    unsigned char current_eight_amps;

    //-- Channels Operation Mode ----    //11
    unsigned char channels_operation_mode;

    //-- For Temperature Protection ----    //12
    unsigned short temp_prot;    // 2 bytes if its aligned to 4
    unsigned char temp_prot_deg;

    //-- For new channels current ---- //15
    unsigned char current_int;
    unsigned char current_dec;

    //-- Hardware config ---- //17
    unsigned char encoder_direction;
    unsigned char dummy2 [6];
    // unsigned char dummy2;          
    // unsigned char dummy3;       

    //-- End of Struct check for 8 bytes alignment ---- //24

} parameters_typedef;

//-- End of Memory Struct to Save --------------------

#endif    /* _PARAMETERS_H_ */

//--- end of file ---//

