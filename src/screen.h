//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F303
// ##
// #### SCREEN.H ##############################
//---------------------------------------------
#ifndef _SCREEN_H_
#define _SCREEN_H_


// Config Defines --------------------------------------------------------------


// Common Defines --------------------------------------------------------------
#define SCREEN_Text4_Line1(X)    SCREEN_Font_Line_Text((4),(1),(X))
#define SCREEN_Text4_Line2(X)    SCREEN_Font_Line_Text((4),(2),(X))


#define SCREEN_Text2_Line1(X)    SCREEN_Font_Line_Text((2),(1),(X))
#define SCREEN_Text2_Line2(X)    SCREEN_Font_Line_Text((2),(2),(X))
#define SCREEN_Text2_Line3(X)    SCREEN_Font_Line_Text((2),(3),(X))
#define SCREEN_Text2_Line4(X)    SCREEN_Font_Line_Text((2),(4),(X))

#define SCREEN_Text2_BlankLine1()    SCREEN_Font_BlankLine((2),(1))
#define SCREEN_Text2_BlankLine2()    SCREEN_Font_BlankLine((2),(2))
#define SCREEN_Text2_BlankLine3()    SCREEN_Font_BlankLine((2),(3))
#define SCREEN_Text2_BlankLine4()    SCREEN_Font_BlankLine((2),(4))


// Module Exported Functions ---------------------------------------------------
void SCREEN_Init (void);
void SCREEN_Clear (void);
void SCREEN_ShowText2 (char *, char *, char *, char *);

void SCREEN_Font_Line_Text (unsigned char f, unsigned char l, char * text);
void SCREEN_Font_BlankLine (unsigned char f, unsigned char l);

// void SCREEN_Text2_BlankLine1 (void);
// void SCREEN_Text2_BlankLine2 (void);
// void SCREEN_Text2_BlankLine3 (void);
// void SCREEN_Text2_BlankLine4 (void);

#endif    /* _SCREEN_H_ */
