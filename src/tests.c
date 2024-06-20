//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------

#include <stdio.h>
#include <string.h>


// Types Constants and Macros --------------------------------------------------


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned char usart2_have_data = 0;
unsigned char rx2buff [256] = { 0 };
unsigned char * prx2;



// Module Functions to Test ----------------------------------------------------
void FuncsGSMG_Entering (void);
unsigned char Usart2HaveData (void);
void Usart2HaveDataReset (void);
unsigned char Usart2ReadBuffer (unsigned char * bout, unsigned short max_len);


// Module Auxiliary Functions --------------------------------------------------
void Test_Functions (void);

// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    Test_Functions ();

    return 0;
}


void Test_Functions (void)
{
    printf("Testing FuncsGSMG_Entering logic...\n");

    printf("Test overbuffer\n");
    usart2_have_data = 1;
    char s_test_string[] = {"01234567890123456789012345678901234567890123456789"};
    prx2 = rx2buff;
    for (int i = 0; i < sizeof(s_test_string); i++)
    {
        *prx2 = s_test_string[i];
        prx2++;
    }
    printf("rx2buff pos %p prx2 pos %p str size %d\n",
           rx2buff,
           prx2,
           sizeof(s_test_string));

    FuncsGSMG_Entering();
    printf("Test overbuffer ended\n");

    printf("Test entering string\n");
    char s_enter[] = {"gsm_gw_mode"};

    usart2_have_data = 1;
    for (int i = 0; i < sizeof(s_enter); i++)
    {
        *prx2 = s_enter[i];
        prx2++;
    }
    printf("rx2buff pos %p prx2 pos %p str size %d\n",
           rx2buff,
           prx2,
           sizeof(s_enter));

    FuncsGSMG_Entering();
    printf("Test entering string ended\n");
    
}


unsigned char Usart2ReadBuffer (unsigned char * bout, unsigned short max_len)
{
    unsigned int len;

    len = prx2 - rx2buff;
    printf("len %d\n", len);

    if (len < max_len)
    {
        len += 1;    //space for '\0' from int
        memcpy(bout, (unsigned char *) rx2buff, len);
    }
    else
    {
        len = max_len;
        memcpy(bout, (unsigned char *) rx2buff, len);
    }

    //ajusto punteros de rx luego de la copia
    prx2 = rx2buff;
    return (unsigned char) len;
}


void FuncsGSMG_Entering (void)
{
    char buff [40] = { 0 };

    // check if we need to enter in this mode
    if (Usart2HaveData())
    {
        Usart2HaveDataReset();
        Usart2ReadBuffer((unsigned char *)buff, sizeof(buff));
        printf("str has len: %d\n", strlen(buff));
        if (!strncmp(buff, "gsm_gw_mode", sizeof ("gsm_gw_mode") -1))
        {
            printf("In FuncsGSMGateway () \n");
        }
    }
}


unsigned char Usart2HaveData (void)
{
    return usart2_have_data;
}


void Usart2HaveDataReset (void)
{
    usart2_have_data = 0;
}



//--- end of file ---//


