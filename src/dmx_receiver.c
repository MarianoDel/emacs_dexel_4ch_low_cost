//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_RECEIVER.C ########################
//---------------------------------------------
#include "dmx_receiver.h"

#include "hard.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

#ifdef STM32F030
#include "stm32f0xx.h"
#endif

#ifdef STM32G030xx
#include "stm32g0xx.h"
#endif

#define USE_LED_FOR_BREAK_DETECT
// Internal Configs ------------------------------------------------------------
// TIM used for break detect
#define TIM_BREAK    TIM14
// GPIO for break detect
#define GPIO_BREAK    DMX_INPUT
// LED for break detect
#ifdef USE_LED_FOR_BREAK_DETECT
#define LED_BREAK    LED
#define LED_BREAK_OFF    LED_ON
#define LED_BREAK_ON    LED_OFF
#endif


#define SIZEOF_DMX_DATA512       512
#define LAST_DMX_CHANNEL_512    (SIZEOF_DMX_DATA512 - 1)
#define DMX_TIMEOUT	20
// End of Internal Configs -----------------------------------------------------


// Externals -------------------------------------------------------------------
extern volatile unsigned char dmx_buff_data[];
extern volatile unsigned char Packet_Detected_Flag;
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char dmx_receive_flag;


// Globals ---------------------------------------------------------------------
volatile unsigned short current_channel_index = 0;
volatile unsigned char data512[SIZEOF_DMX_DATA512];
volatile unsigned char dmx_timeout_timer = 0;
volatile unsigned char dmx_exti_enable_flag = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void DMX_EnableRx (void)
{
    //enable the Rx int break detect
    EXTIOn ();
    //enable flag for timeouts
    dmx_exti_enable_flag = 1;
    //enable usart and Rx int
    Usart1Enable ();
}


void DMX_DisableRx (void)
{
    //disable flag for timeouts
    // dmx_exti_enable_flag = 0;
    dmx_exti_enable_flag = 1;    //enable for not in dmx automatic detection  
    //disable the Rx int break detect
    EXTIOff ();
    //disable int and driver on usart
    Usart1Disable ();
}


// void UpdatePackets (void)
// {
//     if (Packet_Detected_Flag)
//     {
//         if (dmx_buff_data[0] == 0x00)
//             DMX_packet_flag = 1;

//         if (dmx_buff_data[0] == 0xCC)
//             RDM_packet_flag = 1;

//         Packet_Detected_Flag = 0;
//     }
// }


void DMX_Int_Serial_Receiver_Handler (unsigned char dummy)
{
    if (dmx_receive_flag)
    {
        if (current_channel_index <= LAST_DMX_CHANNEL_512)    //else discard silently
        {
            data512[current_channel_index] = dummy;            

            if (current_channel_index >= (DMX_channel_selected + DMX_channel_quantity - 1))
            {
                // first copy the init of buffer and then the selected channels
                dmx_buff_data[0] = data512[0];
                for (unsigned char i = 0; i < DMX_channel_quantity; i++)
                    dmx_buff_data[i + 1] = data512[(DMX_channel_selected) + i];

                //--- Reception end ---//
                current_channel_index = 0;
                dmx_receive_flag = 0;
                Packet_Detected_Flag = 1;
#ifdef USE_LED_FOR_BREAK_DETECT
                LED_BREAK_ON;
#endif                                    
            }
            else
                current_channel_index++;
        }
    }
}


//the dmx line is high when idle, going to zero on the start of packet
//the time that remains in zero is the signal break (87us to 4800us) are valid times
//the time in one is the signal mark (8us aprox.) this time its not checked
//after that the serial bytes start to comming on 250Kbits, can have idle times between them
typedef enum {
    PCKT_RX_IDLE = 0,
    PCKT_RX_LOOK_FOR_BREAK,
    PCKT_RX_LOOK_FOR_MARK,
    PCKT_RX_LOOK_FOR_START
} pckt_rx_e;

volatile pckt_rx_e dmx_signal_state = PCKT_RX_IDLE;
void DMX_Int_Break_Handler (void)
{
    unsigned short aux;
    //si no esta con el USART detecta el flanco	PONER TIMEOUT ACA?????
    if ((dmx_receive_flag == 0) || (dmx_timeout_timer == 0))
        //if (dmx_receive_flag == 0)
    {
        switch (dmx_signal_state)
        {
        case PCKT_RX_IDLE:
            if (!(GPIO_BREAK))
            {
                //Activo timer en Falling.
                TIM_BREAK->CNT = 0;
                TIM_BREAK->CR1 |= 0x0001;
                dmx_signal_state++;
            }
            break;

        case PCKT_RX_LOOK_FOR_BREAK:
            if (GPIO_BREAK)
            {
                //Desactivo timer en Rising.
                aux = TIM_BREAK->CNT;

                //reviso BREAK
                //if (((tim_counter_65ms) || (aux > 88)) && (tim_counter_65ms <= 20))
                //if ((aux > 87) && (aux < 210))	//Consola STARLET 6
                //if ((aux > 87) && (aux < 2000))		//Consola marca CODE tiene break 1.88ms
                if ((aux > 87) && (aux < 4600))		//Consola marca CODE modelo A24 tiene break 4.48ms fecha 11-04-17
                {
                    dmx_signal_state++;
                    current_channel_index = 0;
                    dmx_timeout_timer = DMX_TIMEOUT;		//activo el timeout para esperar un MARK valido
                }
                else	//falso disparo
                    dmx_signal_state = PCKT_RX_IDLE;
            }
            else	//falso disparo
                dmx_signal_state = PCKT_RX_IDLE;

            TIM_BREAK->CR1 &= 0xFFFE;
            break;

        case PCKT_RX_LOOK_FOR_MARK:
            if ((!(GPIO_BREAK)) && (dmx_timeout_timer))	//termino Mark after break
            {
                //serial was already enable, let it made a callback
                dmx_receive_flag = 1;
                EXTIOff();    //dejo 20ms del paquete sin INT
#ifdef USE_LED_FOR_BREAK_DETECT
                LED_BREAK_OFF;
#endif
            }
            else	//falso disparo
            {
                //termine por timeout
                dmx_receive_flag = 0;
            }
            dmx_signal_state = PCKT_RX_IDLE;
            break;

        default:
            dmx_signal_state = PCKT_RX_IDLE;
            break;
        }
    }
}


void DMX_Int_Millis_Handler (void)
{
    //after a start of pck rx, leave 20ms the int in off
    if (dmx_timeout_timer)
        dmx_timeout_timer--;
    else if (dmx_exti_enable_flag)
        EXTIOn();
}

//--- end of file ---//





