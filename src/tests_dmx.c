//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_DMX.C ###########################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "dmx_receiver.h"
#include "tests_ok.h"

#include <stdio.h>
// #include <stdlib.h>
#include <string.h>


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
// unsigned char usart1_have_data = 0;
// char new_uart_msg [200] = { 0 };
// char last_uart_sended [200] = { 0 };
int exti_is_on = 0;



// Module Auxialiary Functions -------------------------------------------------


// Module Functions for testing ------------------------------------------------
void Test_Comms (void);
    

// Module Functions ------------------------------------------------------------


int main(int argc, char *argv[])
{

    Test_Comms ();
    
}


// give externals inputs to the module
volatile unsigned char dmx_buff_data[3] = { 0 };
volatile unsigned char Packet_Detected_Flag = 0;
volatile unsigned short DMX_channel_selected = 0;
volatile unsigned char DMX_channel_quantity = 1;
// get globals from module
extern unsigned char dmx_receive_flag;

void Test_Comms (void)
{
    printf("Testing DMX receiver routines\n");
    
    // Test receiver routine
    dmx_receive_flag = 1;
    DMX_channel_selected = 1;
    DMX_channel_quantity = 2;
    for (int i = 0; i < 512; i++)
    {
        DMX_Int_Serial_Receiver_Handler((unsigned char) i);
    }

    printf("ch: %d buff: [%d] [%d] [%d]\n",
           DMX_channel_selected,
           dmx_buff_data[0],
           dmx_buff_data[1],
           dmx_buff_data[2]);

    printf("Testing first channels: ");
    if ((dmx_buff_data[0] == 0) &&
        (dmx_buff_data[1] == 1) &&
        (dmx_buff_data[2] == 2))
        PrintOK();
    else
        PrintERR();
    

    // test not rx
    dmx_receive_flag = 0;
    for (int i = 0; i < 512; i++)
    {
        DMX_Int_Serial_Receiver_Handler(0);
    }

    printf("ch: %d buff: [%d] [%d] [%d]\n",
           DMX_channel_selected,
           dmx_buff_data[0],
           dmx_buff_data[1],
           dmx_buff_data[2]);

    printf("Testing not DMX allowed: ");
    if ((dmx_buff_data[0] == 0) &&
        (dmx_buff_data[1] == 1) &&
        (dmx_buff_data[2] == 2))
        PrintOK();
    else
        PrintERR();
    

    // test last allowed channel
    dmx_receive_flag = 1;
    for (int i = 0; i < 512; i++)
    {
        DMX_Int_Serial_Receiver_Handler(0);
    }

    dmx_receive_flag = 1;    
    DMX_channel_selected = 510;
    DMX_channel_quantity = 2;
    unsigned char data = 0;
    for (int i = 0; i < 512; i++)
    {
        DMX_Int_Serial_Receiver_Handler(data);
        data++;
    }

    printf("ch: %d buff: [%d] [%d] [%d]\n",
           DMX_channel_selected,
           dmx_buff_data[0],
           dmx_buff_data[1],
           dmx_buff_data[2]);

    printf("Testing last channels: ");
    if ((dmx_buff_data[0] == 0) &&
        (dmx_buff_data[1] == 254) &&
        (dmx_buff_data[2] == 255))
        PrintOK();
    else
        PrintERR();
    
    
}


void EXTIOn (void)
{
    exti_is_on = 1;
    printf("EXTI is on\n");
}


void EXTIOff (void)
{
    exti_is_on = 0;
    printf("EXTI is off\n");
}


//--- end of file ---//


