#ifndef MAX7219_H_
#define MAX7219_H_

typedef struct dict_t_struct {
    char key;
    uint8_t value[8];
} dict_t;


void MAX72_Init(void);
void MAX72_SendRow(uint8_t row, uint8_t pattern[4]);
void MAX72_Clear(void);

void Print_Alphabet(void);

void MAX72_PrintChar(char c);
void MAX72_Print_String(const char *str);
void MAX72_Print_Number(int num);

// Senza Interrupt (Bloccante)
void MAX72_Scroll(const char *text, uint16_t delay_ms);

// Con interrupt
void MAX72_Start_Scrolling(const char *text);
void MAX72_Scroll_Process(void);
void MAX72_Scroll_Resume(void);
void MAX72_Scroll_Stop(void);
void MAX72_Scroll_Timer_ISR(void);
#endif /* MAX7219_H_ */
