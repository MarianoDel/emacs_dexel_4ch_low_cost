//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_COMMS.C #########################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "comms_power.h"

//helper modules
#include "tests_ok.h"
#include "tests_mock_usart.h"

#include <stdio.h>
#include <string.h>


#define COMM_FLAG_OK    0x0001

#define comms_flag_ok_check    (comms_flags & COMM_FLAG_OK)
#define comms_flag_ok_set     (comms_flags |= COMM_FLAG_OK)
#define comms_flag_ok_reset    (comms_flags &= ~(COMM_FLAG_OK))

// Externals - Globals on Module to Test ---------------------------------------
extern volatile unsigned short comms_timeout;
extern unsigned short comms_flags;


// Globals ---------------------------------------------------------------------
int cb_usart_value = 0;


// Module Auxialiary Functions -------------------------------------------------
void CB_Usart (char * s);


// Module Functions for testing ------------------------------------------------
void Test_Comms_Bright (void);
void Test_Comms_Current (void);
void Test_Comms_Power_Version (void);
void Test_Comms_Power_Version_Inner (void);


// Module Functions ------------------------------------------------------------
int main(int argc, char *argv[])
{

    // Test_Comms_Bright ();

    // Test_Comms_Current ();

    // Test_Comms_Power_Version ();

    Test_Comms_Power_Version_Inner ();    

    return 0;
}


void Test_Comms_Bright (void)
{
    printf("Test Comms -- Send_Bright\n");

    unsigned char dmx_data[2] = { 0 };

    printf(" -- min on bright\n");

    dmx_data[0] = 1;
    dmx_data[1] = 0;    
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);

    dmx_data[0] = 1;
    dmx_data[1] = 1;
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);

    dmx_data[0] = 255;
    dmx_data[1] = 1;
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);

    dmx_data[0] = 255;
    dmx_data[1] = 0;
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);

    dmx_data[0] = 255;
    dmx_data[1] = 254;
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);

    dmx_data[0] = 255;
    dmx_data[1] = 255;
    printf("dmx0: %d dmx1: %d\n", dmx_data[0] ,dmx_data[1]);
    Comms_Power_Send_Bright(dmx_data);
    
    // printf(" -- all brights 0 - 255\n");
    // for (int i = 0; i < 255; i++)
    // {
    //     dmx_data[0] = (unsigned char) i;
    //     Comms_Power_Send_Bright(dmx_data);
    // }
    
}


void Test_Comms_Current (void)
{
    resp_t resp = resp_continue;
    unsigned char cint = 1;
    unsigned char cdec = 0;
    
    printf("Test Comms -- Send_Current\n");

    printf("current to send: %01d.%01dA\n", cint, cdec);

    resp = Comms_Power_Send_Current_Conf (cint, cdec);
}


void Test_Comms_Power_Version (void)
{
    resp_t resp = resp_continue;

    printf("Test Comms -- Get_Version\n");

    printf("current check version conf: %s\n", Comms_Power_Check_Version());
    resp = Comms_Power_Get_Version ();
}


void Test_Comms_Power_Version_Inner (void)
{
    printf("Test Comms -- Get_Version Inner\n");

    char msg [] = {"Hrd 2.0 Soft 1.1"};
    // char msg [] = {"Hrd 2.0 Soft 1.1\n"};    
    
    comms_flag_ok_reset;
    Comms_Power_Rx_Messages (msg);

    printf("Result: ");
    if (comms_flag_ok_check)
        PrintOK();
    else
        PrintERR();
            
}


// Module Auxiliary Functions --------------------------------------------------
void CB_Usart (char * s)
{
    if (strncmp(s, "version", sizeof("version") - 1) == 0)
        cb_usart_value = 1;

    // if (strncmp(s, "err ch2", sizeof("err ch2") - 1) == 0)
    //     cb_usart_value = 2;

    // if (strncmp(s, "err verif", sizeof("err verif") - 1) == 0)
    //     cb_usart_value = 3;
    
    // if (strncmp(s, "ok", sizeof("ok") - 1) == 0)
    //     cb_usart_value = 4;

    // if (strncmp(s, "Hrd 2.0 Soft 1.1", sizeof("Hrd 2.0 Soft 1.1") - 1) == 0)
    //     cb_usart_value = 5;
}


// Module Mocked Functions -----------------------------------------------------


//--- end of file ---//


