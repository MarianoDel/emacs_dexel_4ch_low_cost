//----------------------------------------
// Tests for gtk lib
//----------------------------------------

#include <gtk/gtk.h>

// #define OLED_128_64
#define OLED_128_32

// Application Module Exported Functions ---------------------------------------
gboolean Test_Main_Loop (gpointer user_data);
gboolean Test_Timeouts_Loop_1ms (gpointer user_data);
gboolean Test_Timeouts_Loop_1000ms (gpointer user_data);

void button1_function (void);
void button2_function (void);
void button3_function (void);


//--- end of file ---//
