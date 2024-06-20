//----------------------------------------
// Tests for gtk lib
//----------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_lcd_cgrom.h"
#include "tests_lcd_configuration.h"
#include "tests_lcd_application.h"

// Module Types Constants and Macros -------------------------------------------
#define LCD_LAST_ROW    (LCD_ROWS - 1)
#define LCD_LAST_COLUMN    (LCD_COLUMNS - 1)


typedef struct {
    int R;
    int G;
    int B;
} rgb_st;

typedef struct {
    int cursor;
    int blink;
    int blink_was_on;
    int blink_last_index_line;
    int blink_last_index_cursor;
    unsigned char ddram [LCD_ROWS][LCD_COLUMNS];
    
} lcd_display_config_st;

//-- Module Functions Declarations ---------------
void end_program (GtkWidget *, gpointer);
static void init_surface (void);
static void fill_surface (void);
static void unfill_surface (void);

static void draw_patch (GdkPixbuf * p, int x1, int x2, int y1, int y2, rgb_st *);

static void put_pixel (GdkPixbuf *pixbuf,
                       int x,
                       int y,
                       guchar red,
                       guchar green,
                       guchar blue,
                       guchar alpha);

static void draw_lcd_patch (GdkPixbuf * p, int x, int y, rgb_st *);
static void draw_mini_patch (GdkPixbuf * p, int x, int y, rgb_st *);
static void fill_with_lcd_patches (GdkPixbuf * p);
void get_lcd_patch_position (int row, int column, int * x, int * y);
static void draw_lcd_patch_from_cgrom (GdkPixbuf * p, int line_pos, int cursor_pos, int symbol_index);
    
gboolean Print_Callback (gpointer user_data);    //send data tests pourpose

// LCD low level Functions, reimplemeted for hardware abstraction --------------
void Lcd_TransmitStr (char * line);
void Lcd_SetDDRAM (uint8_t ddram);
void Lcd_Command (unsigned char);
void Lcd_sendcommand (uint8_t cmd);

gboolean Lcd_Timeout_Callback (gpointer user_data);

void dwn_button_function (void);
void up_button_function (void);
void set_button_function (void);


// Functions for test declaration ----------------------------------------------

// End of Functions for test declaration ---------------------------------------


// Globals --------------------------------------------------------
GdkPixbuf * pix;
GtkWidget *imag1;
// static cairo_surface_t *surface = NULL;
lcd_display_config_st lcd_display;


//-- Module Functions Definitions ---------------


int main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "lcd_4_20_layout.glade", NULL);

    //widgets from builder
    GtkWidget *win = (GtkWidget *) gtk_builder_get_object (builder, "window1");
    GtkWidget *btn1 = (GtkWidget *) gtk_builder_get_object (builder, "button1");
    GtkWidget *btn2 = (GtkWidget *) gtk_builder_get_object (builder, "button2");
    GtkWidget *btn3 = (GtkWidget *) gtk_builder_get_object (builder, "button3");
    GtkWidget *btn4 = (GtkWidget *) gtk_builder_get_object (builder, "button4");    
    imag1 = (GtkWidget *) gtk_builder_get_object (builder, "image1");    

    g_signal_connect(btn1, "clicked", G_CALLBACK(dwn_button_function), NULL);    
    g_signal_connect(btn2, "clicked", G_CALLBACK(up_button_function), NULL);
    g_signal_connect(btn3, "clicked", G_CALLBACK(set_button_function), NULL);    
    g_signal_connect(btn4, "clicked", G_CALLBACK(end_program), NULL);    

    gtk_widget_show_all (win);
    init_surface();

    // gint mydata = 5;
    // gpointer data = (gpointer *) &mydata;
    // gdk_threads_add_timeout (1000, Print_Callback, data);

    gdk_threads_add_timeout (1000, Lcd_Timeout_Callback, NULL);    
    gdk_threads_add_timeout (1, Test_Timeouts, NULL);
    gdk_threads_add_idle (Test_Main_Loop, NULL);    
    gtk_main();
    
    return 0;
}


// This one is for internal tests only -----------------------------------------
gint new_loop = 0;
gint loops = 0;
gboolean Print_Callback (gpointer user_data)
{
    gint * udata = user_data;
    if (!new_loop)
    {
        g_print("data getted: %d\n", *udata);
        loops = *udata;
        new_loop = 1;
    }

    if (loops)
    {
        g_print("new loop: %d\n", loops);
        loops--;
    }
    else
    {
        g_print("ending loop\n");
        return FALSE;
    }
    
    return TRUE;
}
// End of internal tests -------------------------------------------------------




void end_program (GtkWidget *wid, gpointer ptr)
{
    gtk_main_quit();
}


static void init_surface (void)
{
    gint width;
    gint height;
#ifdef LCD_CONF_2_16
    gdk_pixbuf_get_file_info ("lcd_2_16.jpeg", &width, &height);
    g_print("File is: %d width %d height\n", width, height);
                          
    GError *gerror = NULL;
    pix = gdk_pixbuf_new_from_file ("lcd_2_16.jpeg", &gerror);

    if (pix == NULL)
    {
        g_print("Error loading pix: %s\n", gerror->message);
        g_error_free(gerror);
    }
    else
    {
        g_print("pix loaded\n");
        unfill_surface();
        fill_with_lcd_patches(pix);
    }
#endif
#ifdef LCD_CONF_4_20
    gdk_pixbuf_get_file_info ("lcd_4_20.jpeg", &width, &height);
    g_print("File is: %d width %d height\n", width, height);
                          
    GError *gerror = NULL;
    pix = gdk_pixbuf_new_from_file ("lcd_4_20.jpeg", &gerror);

    if (pix == NULL)
    {
        g_print("Error loading pix: %s\n", gerror->message);
        g_error_free(gerror);
    }
    else
    {
        g_print("pix loaded\n");
        unfill_surface();
        fill_with_lcd_patches(pix);
    }
#endif
}


static void draw_patch (GdkPixbuf * p, int x1, int x2, int y1, int y2, rgb_st * rgb)
{
    for (int j = y1; j < y2; j++)
    {
        for (int i = x1; i < x2; i++)
            put_pixel(p, i, j, rgb->R, rgb->G, rgb->B, 0);
    }
}


#define MINI_PATCH_WIDTH    3
#define MINI_PATCH_HEIGHT    3
static void draw_mini_patch (GdkPixbuf * p, int x, int y, rgb_st * rgb)
{
    for (int j = y; j < (MINI_PATCH_HEIGHT + y); j++)
    {
        for (int i = x; i < (MINI_PATCH_WIDTH + x); i++)
            put_pixel(p, i, j, rgb->R, rgb->G, rgb->B, 0);
    }
}


static void draw_lcd_patch (GdkPixbuf * p, int x, int y, rgb_st * rgb)
{
    int col = 0;
    int row = 0;

    //5 x 8 mini_patch
    for (int j = 0; j < 8; j++)
    {
        row = y + j * (3 + 1);
        for (int i = 0; i < 5; i++)
        {
            col = x + i * (3 + 1);
            draw_mini_patch(p, col, row, rgb);
        }
    }
}


#define PATCH_WIDTH    19
#define PATCH_HEIGHT    30
static void fill_with_patches (GdkPixbuf * p)
{
    rgb_st rgb;
    rgb.R = 255;
    rgb.G = 0;
    rgb.B = 0;
    
    //line 1
    int col = 0;
    for (int i = 0; i < LCD_COLUMNS; i++)
    {
        col = 73 + i * (PATCH_WIDTH + 1);
        draw_patch(p, col, col + PATCH_WIDTH, 75, 75 + PATCH_HEIGHT, &rgb);
    }

    //line 2
    col = 0;
    for (int i = 0; i < LCD_COLUMNS; i++)
    {
        col = 73 + i * (PATCH_WIDTH + 1);
        draw_patch(p, col, col + PATCH_WIDTH, 75 + PATCH_HEIGHT + 2, 75 + 2*PATCH_HEIGHT + 2, &rgb);
    }
}


static void fill_with_lcd_patches (GdkPixbuf * p)
{
    rgb_st rgb;
    rgb.R = 123;
    rgb.G = 160;
    rgb.B = 47;

    //line 1
    int col = 0;
    int row_pos = 0;
    for (int j = 0; j < LCD_ROWS; j++)
    {
        col = 0;
        for (int i = 0; i < LCD_COLUMNS; i++)
        {
            if (j)
                row_pos = PATCH_ROW_START + PATCH_HEIGHT * j + 4 * j;
            else
                row_pos = PATCH_ROW_START;
        
            col = PATCH_COLUMN_START + i * (PATCH_WIDTH + 2);
            draw_lcd_patch(p, col, row_pos, &rgb);
        }
    }
}


static void unfill_surface (void)
{
    rgb_st rgb;
    rgb.R = 135;
    rgb.G = 175;
    rgb.B = 53;
    
    int x1 = PATCH_COLUMN_START;
    int x2 = PATCH_COLUMN_START + LCD_COLUMNS * (PATCH_WIDTH + 2);
    int y1 = PATCH_ROW_START;
    int y2 = PATCH_ROW_START +
        LCD_ROWS * PATCH_HEIGHT +
        LCD_ROWS * 4;
    
    draw_patch(pix, x1, x2, y1, y2, &rgb);
    
    gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);
}

static void fill_surface (void)
{
    // fill_with_patches(pix);
    fill_with_lcd_patches(pix);    
    gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);    
}

static void put_pixel (GdkPixbuf *pixbuf,
                       int x,
                       int y,
                       guchar red,
                       guchar green,
                       guchar blue,
                       guchar alpha)
{
    int width, height, rowstride, n_channels;
    guchar *pixels, *p;

    n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    // g_print("n_channels: %d\n", n_channels);

    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
    // g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
    // g_assert (n_channels == 4);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    g_assert (x >= 0 && x < width);
    g_assert (y >= 0 && y < height);

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    p = pixels + y * rowstride + x * n_channels;
    p[0] = red;
    p[1] = green;
    p[2] = blue;
    // p[3] = alpha;
}



static void draw_lcd_patch_from_cgrom (GdkPixbuf * p, int line_pos, int cursor_pos, int symbol_index)
{
    rgb_st rgb_blank;
    rgb_blank.R = 123;
    rgb_blank.G = 160;
    rgb_blank.B = 47;

    rgb_st rgb_fill;
    // rgb_fill.R = 255;
    // rgb_fill.G = 0;
    // rgb_fill.B = 0;
    rgb_fill.R = 10;
    rgb_fill.G = 10;
    rgb_fill.B = 10;

    // sanity checks
    if (symbol_index > LAST_SYMBOL_IN_CGROM)
        return;

    if ((line_pos > LCD_LAST_ROW) || (cursor_pos > LCD_LAST_COLUMN))
        return;

    int x = 0;
    int y = 0;
    get_lcd_patch_position (line_pos, cursor_pos, &x, &y);    

    int col = 0;
    int row = 0;

    //5 x 8 mini_patch
    for (int j = 0; j < 8; j++)
    {
        row = y + j * (3 + 1);
        uint8_t * vline = cgrom[symbol_index];
        uint8_t line = *(vline + j);
        // uint8_t line = char_a[j];        
        uint8_t mask = 0;
        for (int i = 0; i < 5; i++)
        {
            mask = 0x10;
            mask >>= i;
            
            col = x + i * (3 + 1);

            if (line & mask)
                draw_mini_patch(p, col, row, &rgb_fill);
            else
                draw_mini_patch(p, col, row, &rgb_blank);
        }
    }
}


void get_lcd_patch_position (int row, int column, int * x, int * y)
{
    if (row > LCD_LAST_ROW)
        return;

    if (column > LCD_LAST_COLUMN)
        return;
    
    *x = PATCH_COLUMN_START + column * (PATCH_WIDTH + 2);

    if (row)
        *y = PATCH_ROW_START + (PATCH_HEIGHT * row) + 4 * row;
    else
        *y = PATCH_ROW_START;
}






// Testing Function loop -------------------------------------------------------
// static GMutex mutex;


// extern volatile unsigned short show_select_timer;
// volatile unsigned short mode_effect_timer = 0;
// unsigned char mode_state;

// volatile unsigned char dmx_buff_data[3] = { 0 };
// volatile unsigned char Packet_Detected_Flag = 0;
// volatile unsigned short DMX_channel_selected = 1;
// parameters_typedef mem_conf;
// unsigned char pwm_channels [2] = { 0 };

// sw_actions_t switch_actions = selection_none;
// int setup_done = 0;
// gboolean Test_Main_Loop (gpointer user_data)
// {
//     resp_t resp = resp_continue;

//     // mem_conf.channels_operation_mode = 1;
//     resp = ManualMode (pwm_channels, switch_actions);

//     if (resp == resp_need_to_save)
//     {
//         printf("memory needs a save!\n");
//     }

//     //wraper to clean sw
//     g_mutex_lock (&mutex);

//     if (switch_actions != selection_none)
//         switch_actions = selection_none;
    
//     g_mutex_unlock (&mutex);
//     // usleep(500);
        
//     return TRUE;
// }


// gboolean Test_Timeouts (gpointer user_data)
// {
//     //timeout lcd_utils internal
//     if (show_select_timer)
//         show_select_timer--;

//     //timeout for dmx_mode or manual_mode
//     if (mode_effect_timer)
//         mode_effect_timer--;
    
//     return TRUE;
// }

// //buttons functions
// void dwn_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     switch_actions = selection_dwn;
//     g_mutex_unlock (&mutex);
// }

// void up_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     switch_actions = selection_up;
//     g_mutex_unlock (&mutex);
// }

// void set_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     switch_actions = selection_enter;
//     g_mutex_unlock (&mutex);
// }



// LCD low level Functions, reimplemeted for hardware abstraction --------------
// Nedded by lcd_utils module provided by lcd module
void LCD_Init_Setup (void)
{
}


int lcd_line = 0;
int lcd_cursor = 0;
void Lcd_TransmitStr (char * line)
{
    while (*line != '\0')
    {
        //if we have the char in the list send it
        if ((*line >= 32) && (*line < (LAST_SYMBOL_IN_CGROM + 32)))
        {
            draw_lcd_patch_from_cgrom (pix, lcd_line, lcd_cursor, (*line - 32));
            if ((lcd_line < LCD_ROWS) && (lcd_cursor < LCD_COLUMNS))
                lcd_display.ddram[lcd_line][lcd_cursor] = *line;
        }

        //update the cursor
        lcd_cursor++;
        line++;
    }
    gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);    
}


void Lcd_SetDDRAM (uint8_t ddram)
{
    if (ddram < LCD_COLUMNS)
    {
        lcd_line = 0;
        lcd_cursor = ddram;
    }

    if ((ddram < (0x40 + LCD_COLUMNS)) && (ddram >= 0x40))
    {
        lcd_line = 1;
        lcd_cursor = ddram - 0x40;
    }

#ifdef LCD_CONF_4_20
    if ((ddram < (0x14 + LCD_COLUMNS)) && (ddram >= 0x14))
    {
        lcd_line = 2;
        lcd_cursor = ddram - 0x14;
    }

    if ((ddram < (0x54 + LCD_COLUMNS)) && (ddram >= 0x54))
    {
        lcd_line = 3;
        lcd_cursor = ddram - 0x54;
    }    
#endif
}


void Lcd_sendcommand (uint8_t cmd)
{
    if (cmd & 0x80)
    {
        cmd &= ~(0x80);
        Lcd_SetDDRAM(cmd);
    }

    //TODO: implement setcgram
}


void Lcd_senddata (unsigned char data)
{
    if ((data >= 32) && (data < (LAST_SYMBOL_IN_CGROM + 32)))
    {
        draw_lcd_patch_from_cgrom (pix, lcd_line, lcd_cursor, (data - 32));
        if ((lcd_line < LCD_ROWS) && (lcd_cursor < LCD_COLUMNS))
            lcd_display.ddram[lcd_line][lcd_cursor] = data;
        
    }

    //update the cursor
    lcd_cursor++;

    gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);    
    
}

void Lcd_SetCGRAM (unsigned char data)
{
}



gboolean Lcd_Timeout_Callback (gpointer user_data)
{
    gint line_pos = lcd_line;
    gint cursor_pos = lcd_cursor;
    
    if ((lcd_display.cursor) && (lcd_display.blink))
    {
        if (lcd_display.blink_was_on)
        {
            lcd_display.blink_was_on = 0;

            //check for old blinking index
            if ((lcd_display.blink_last_index_cursor != cursor_pos) ||
                (lcd_display.blink_last_index_line != line_pos))
            {
                int cp = lcd_display.blink_last_index_cursor;
                int lp = lcd_display.blink_last_index_line;
                draw_lcd_patch_from_cgrom (pix,
                                           lp,
                                           cp,
                                           (lcd_display.ddram[lp][cp] - 32));
            }

            if ((line_pos <= LCD_LAST_ROW) &&
                (cursor_pos <= LCD_LAST_COLUMN))
            {
                draw_lcd_patch_from_cgrom (pix,
                                           line_pos,
                                           cursor_pos,
                                           (lcd_display.ddram[line_pos][cursor_pos] - 32));
            }
        }
        else
        {
            lcd_display.blink_was_on = 1;

            //check for old blinking index
            if ((lcd_display.blink_last_index_cursor != cursor_pos) ||
                (lcd_display.blink_last_index_line != line_pos))
            {
                int cp = lcd_display.blink_last_index_cursor;
                int lp = lcd_display.blink_last_index_line;
                draw_lcd_patch_from_cgrom (pix,
                                           lp,
                                           cp,
                                           (lcd_display.ddram[lp][cp] - 32));
            }
            
            if ((line_pos <= LCD_LAST_ROW) &&
                (cursor_pos <= LCD_LAST_COLUMN))
            {
                rgb_st rgb_fill;
                rgb_fill.R = 10;
                rgb_fill.G = 10;
                rgb_fill.B = 10;
                
                int x = 0;
                int y = 0;
                get_lcd_patch_position (line_pos, cursor_pos, &x, &y);
                draw_lcd_patch (pix, x, y, &rgb_fill);

                lcd_display.blink_last_index_cursor = cursor_pos;
                lcd_display.blink_last_index_line = line_pos;
            }
        }
        
        gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);
    }

    return TRUE;
}


#define CLEAR         0
#define RET_HOME      1
#define DISPLAY_ON    2
#define DISPLAY_OFF   3
#define CURSOR_ON     4
#define CURSOR_OFF    5
#define BLINK_ON      6
#define BLINK_OFF     7
void Lcd_Command (unsigned char data)
{
    if ((data == CLEAR) || (data == DISPLAY_OFF))
    {
        unfill_surface();
        fill_with_lcd_patches(pix);
        gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);        
        Lcd_SetDDRAM(0x00);
        for (int j = 0; j < LCD_ROWS; j++)
        {
            for (int i = 0; i < LCD_COLUMNS; i++)
                lcd_display.ddram[j][i] = 0;

        }
    }

    if (data == RET_HOME)
        Lcd_SetDDRAM(0x00);

    if (data == DISPLAY_OFF)
    {
        //TODO: poner traba para no mostrar nada hasta DISPLAY_ON
        unfill_surface();
        fill_with_lcd_patches(pix);
        gtk_image_set_from_pixbuf (GTK_IMAGE(imag1), pix);        
        lcd_display.cursor = 0;
        lcd_display.blink = 0;
    }
    
    if (data == CURSOR_ON)
        lcd_display.cursor = 1;

    if (data == CURSOR_OFF)
        lcd_display.cursor = 0;

    if (data == BLINK_ON)
        lcd_display.blink = 1;

    if (data == BLINK_OFF)
        lcd_display.blink = 0;
    
}
// End of LCD low level Functions ----------------------------------------------





//--- end of file ---//
