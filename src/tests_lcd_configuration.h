//----------------------------------------
// Tests for gtk lib
//----------------------------------------

// Select the lcd used
// The LCD used 2x16 or 4x20
#define LCD_CONF_2_16
// #define LCD_CONF_4_20

#ifdef LCD_CONF_4_20
#define LCD_ROWS    4
#define LCD_COLUMNS    20
// Start rows and columns from these pixels on the image
#define PATCH_COLUMN_START    54
#define PATCH_ROW_START    72
#endif

#ifdef LCD_CONF_2_16
#define LCD_ROWS    2
#define LCD_COLUMNS    16
// Start rows and columns from these pixels on the image
#define PATCH_COLUMN_START    70
#define PATCH_ROW_START    74
#endif


//--- end of file ---//
