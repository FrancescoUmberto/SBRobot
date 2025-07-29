#include "stm32f4xx_hal.h"
#include "headers/display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHAR_MAX 95

extern SPI_HandleTypeDef hspi2;

const uint8_t InitCommands[5][2] = {
    {0x09, 0x00}, // Decode Mode: no decode, matrix mode
    {0x0A, 0x00}, // Intensity: min brightness
    {0x0B, 0x07}, // Scan Limit: all 8 digits
    {0x0C, 0x01}, // Shutdown Register: normal operation
    {0x0F, 0x00}  // Display Test: off
};

dict_t LETTERS[CHAR_MAX] =
{
    {' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'!', {0b00100, 0b00000, 0b00000, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000}},
    {'"', {0b00000, 0b00000, 0b00000, 0b00000, 0b01010, 0b01010, 0b01010, 0b00000}},
    {'#', {0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010, 0b00000}},
    {'$', {0b00100, 0b01111, 0b10100, 0b01110, 0b00101, 0b11110, 0b00100, 0b00000}},
    {'%', {0b11000, 0b11001, 0b00010, 0b00100, 0b01000, 0b10011, 0b00011, 0b00000}},
    {'&', {0b10110, 0b01001, 0b10101, 0b00010, 0b00101, 0b01001, 0b00110, 0b00000}},
    {'\'', {0b00000, 0b00000, 0b00000, 0b00000, 0b00010, 0b00100, 0b00110, 0b00000}},
    {'(', {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000, 0b00000}},
    {')', {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010, 0b00000}},
    {'*', {0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000, 0b00000}},
    {'+', {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000, 0b00000}},
    {',', {0b00010, 0b00100, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'.', {0b00110, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'/', {0b00000, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000, 0b00000}},
    {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110, 0b00000}},
    {'1', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00110, 0b00100, 0b00000}},
    {'2', {0b11111, 0b00010, 0b00100, 0b01000, 0b10000, 0b10001, 0b01110, 0b00000}},
    {'3', {0b01110, 0b10001, 0b10000, 0b01000, 0b00100, 0b01000, 0b11111, 0b00000}},
    {'4', {0b01000, 0b01000, 0b11111, 0b01001, 0b01010, 0b01100, 0b01000, 0b00000}},
    {'5', {0b01110, 0b10001, 0b10000, 0b10000, 0b01111, 0b00001, 0b11111, 0b00000}},
    {'6', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100, 0b00000}},
    {'7', {0b00010, 0b00010, 0b00010, 0b00100, 0b01000, 0b10001, 0b11111, 0b00000}},
    {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110, 0b00000}},
    {'9', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110, 0b00000}},
    {':', {0b00110, 0b00110, 0b00000, 0b00110, 0b00110, 0b00000, 0b00000, 0b00000}},
    {';', {0b00010, 0b00100, 0b00110, 0b00000, 0b00110, 0b00110, 0b00000, 0b00000}},
    {'<', {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000, 0b00000}},
    {'=', {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
    {'>', {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010, 0b00000}},
    {'?', {0b00100, 0b00000, 0b00100, 0b01000, 0b10000, 0b10001, 0b01110, 0b00000}},
    {'@', {0b01110, 0b10101, 0b10101, 0b10110, 0b10000, 0b10001, 0b01110, 0b00000}},
    {'A', {0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000}},
    {'B', {0b01111, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b01111, 0b00000}},
    {'C', {0b01110, 0b10001, 0b00001, 0b00001, 0b00001, 0b10001, 0b01110, 0b00000}},
    {'D', {0b01111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01111, 0b00000}},
    {'E', {0b11111, 0b00001, 0b00001, 0b11111, 0b00001, 0b00001, 0b11111, 0b00000}},
    {'F', {0b00001, 0b00001, 0b00001, 0b01111, 0b00001, 0b00001, 0b11111, 0b00000}},
    {'G', {0b01110, 0b10001, 0b10001, 0b11101, 0b00001, 0b10001, 0b01110, 0b00000}},
    {'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b00000}},
    {'I', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111, 0b00000}},
    {'J', {0b00110, 0b01001, 0b01001, 0b01000, 0b01000, 0b01000, 0b11110, 0b00000}},
    {'K', {0b10001, 0b01001, 0b00101, 0b00011, 0b00101, 0b01001, 0b10001, 0b00000}},
    {'L', {0b11111, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00000}},
    {'M', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001, 0b00000}},
    {'N', {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b00000}},
    {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000}},
    {'P', {0b00001, 0b00001, 0b00001, 0b01111, 0b10001, 0b10001, 0b01111, 0b00000}},
    {'Q', {0b10110, 0b01001, 0b10101, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000}},
    {'R', {0b10001, 0b10001, 0b01001, 0b01111, 0b10001, 0b10001, 0b01111, 0b00000}},
    {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110, 0b00000}},
    {'T', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111, 0b00000}},
    {'U', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000}},
    {'V', {0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000}},
    {'W', {0b01010, 0b10101, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001, 0b00000}},
    {'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001, 0b00000}},
    {'Y', {0b00100, 0b00100, 0b00100, 0b00100, 0b01010, 0b10001, 0b10001, 0b00000}},
    {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111, 0b00000}},
    {'[', {0b00111, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00111, 0b00000}},
    {'\\', {0b00000, 0b10000, 0b01000, 0b00100, 0b00010, 0b00001, 0b00000, 0b00000}},
    {']', {0b11100, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11100, 0b00000}},
    {'^', {0b00000, 0b00000, 0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b00000}},
    {'_', {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'`', {0b00000, 0b00000, 0b00000, 0b00000, 0b01000, 0b00100, 0b00010, 0b00000}},
    {'a', {0b11110, 0b10001, 0b11110, 0b10000, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'b', {0b01111, 0b10001, 0b10001, 0b10011, 0b01101, 0b00001, 0b00001, 0b00000}},
    {'c', {0b01110, 0b10001, 0b00001, 0b00001, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'d', {0b11110, 0b10001, 0b10001, 0b11001, 0b10110, 0b10000, 0b10000, 0b00000}},
    {'e', {0b01110, 0b00001, 0b11111, 0b10001, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'f', {0b00010, 0b00010, 0b00010, 0b00111, 0b00010, 0b10010, 0b01100, 0b00000}},
    {'g', {0b01110, 0b10000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000, 0b00000}},
    {'h', {0b10001, 0b10001, 0b10001, 0b10011, 0b01101, 0b00001, 0b00001, 0b00000}},
    {'i', {0b01110, 0b00100, 0b00100, 0b00100, 0b00110, 0b00000, 0b00100, 0b00000}},
    {'j', {0b00110, 0b01001, 0b01000, 0b01000, 0b01100, 0b00000, 0b01000, 0b00000}},
    {'k', {0b01001, 0b00101, 0b00011, 0b00101, 0b01001, 0b00001, 0b00001, 0b00000}},
    {'l', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00110, 0b00000}},
    {'m', {0b10001, 0b10001, 0b10101, 0b10101, 0b01010, 0b00000, 0b00000, 0b00000}},
    {'n', {0b10001, 0b10001, 0b10001, 0b10011, 0b01101, 0b00000, 0b00000, 0b00000}},
    {'o', {0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'p', {0b00001, 0b00001, 0b01111, 0b10001, 0b01111, 0b00000, 0b00000, 0b00000}},
    {'q', {0b01000, 0b01000, 0b01110, 0b01001, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'r', {0b00001, 0b00001, 0b00001, 0b10011, 0b01101, 0b00000, 0b00000, 0b00000}},
    {'s', {0b01111, 0b10000, 0b01110, 0b00001, 0b01110, 0b00000, 0b00000, 0b00000}},
    {'t', {0b01100, 0b10010, 0b00010, 0b00010, 0b00111, 0b00010, 0b00010, 0b00000}},
    {'u', {0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000, 0b00000, 0b00000}},
    {'v', {0b00100, 0b01010, 0b10001, 0b10001, 0b10001, 0b00000, 0b00000, 0b00000}},
    {'w', {0b01010, 0b10101, 0b10101, 0b10001, 0b10001, 0b00000, 0b00000, 0b00000}},
    {'x', {0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000}},
    {'y', {0b01110, 0b10000, 0b11110, 0b10001, 0b10001, 0b00000, 0b00000, 0b00000}},
    {'z', {0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000, 0b00000, 0b00000}},
    {'{', {0b01000, 0b00100, 0b00100, 0b00010, 0b00100, 0b00100, 0b01000, 0b00000}},
    {'|', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000}},
    {'}', {0b00010, 0b00100, 0b00100, 0b01000, 0b00100, 0b00100, 0b00010, 0b00000}},
    {'~', {0b00000, 0b00000, 0b00000, 0b01001, 0b10110, 0b00000, 0b00000, 0b00000}},
};


void MAX72_init(void)
{
    uint8_t txBuf[8];
    for (uint8_t cmd=0; cmd<5; cmd++) {
        // riempi txBuf con 4 volte InitCommands[cmd]
        for (uint8_t dev=0; dev<4; dev++) {
            txBuf[dev*2]   = InitCommands[cmd][0];
            txBuf[dev*2+1] = InitCommands[cmd][1];
        }
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // CS
        HAL_SPI_Transmit(&hspi2, txBuf, sizeof(txBuf), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); // CS
    }
}

void MAX72_SendRow(uint8_t row, uint8_t pattern[4])
{
    uint8_t txBuf[8];
    // pattern[i] = dato per il modulo i
    for (uint8_t dev=0; dev<4; dev++) {
        txBuf[dev*2]   = row;          // registro DIG[row]
        txBuf[dev*2+1] = pattern[dev]; // i‑esimo byte di dati
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, txBuf, sizeof(txBuf), HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void Print_Alphabet(void)
{
    for (uint8_t idx=0; idx<CHAR_MAX; idx++) {
        for (uint8_t row=0; row<=7; row++) {
            // tutti e 4 i moduli mostrano lo stesso carattere
            uint8_t pat[4] = {
                LETTERS[idx].value[row],
                LETTERS[idx].value[row],
                LETTERS[idx].value[row],
                LETTERS[idx].value[row]
            };
            MAX72_SendRow(row+1, pat); // rows: 1-8
        }
        HAL_Delay(500);
    }
}

void MAX72_Clear(void)
{
	uint8_t emptyPattern[4] = {0, 0, 0, 0};
	    for (uint8_t row=1; row<=8; row++) {
	        MAX72_SendRow(row, emptyPattern);
	    }
}

void MAX72_PrintChar(char c)
{
	// ASCII range: 32-126
	// ' ' is 32 and '~' is 126
    if (c < ' ' || c > '~') return; // carattere non valido
    uint8_t idx = c - ' '; // calcola l'indice del carattere
    for (uint8_t row=0; row<=7; row++) {
        // tutti e 4 i moduli mostrano lo stesso carattere
        uint8_t pat[4] = {
            LETTERS[idx].value[row],
            LETTERS[idx].value[row],
            LETTERS[idx].value[row],
            LETTERS[idx].value[row]
        };
        MAX72_SendRow(row+1, pat); // rows: 1-8
    }
}

void MAX72_Print_String(const char *str)
{
    // str reverse
    // if str length is greater than 4, truncate it
    uint8_t len = strlen(str);
    char reversed[4] = {}; // max 4 chars
    for (uint8_t i = 0; i < 4 && i < len; i++) {
        reversed[i] = str[len - 1 - i];
    }

    // if str length is less than 4, pad with spaces
    for (uint8_t i = len; i < 4; i++) {
        reversed[i] = ' ';
    }

    // create a pattern and print with SendRow
    for (uint8_t row=0; row<=7; row++) {
        uint8_t pat[4] = {
            LETTERS[reversed[0] - ' '].value[row],
            LETTERS[reversed[1] - ' '].value[row],
            LETTERS[reversed[2] - ' '].value[row],
            LETTERS[reversed[3] - ' '].value[row]
        };
        MAX72_SendRow(row+1, pat); // rows: 1-8
    }
}

void MAX72_Print_Int(int num)
{
    char str[5];
    snprintf(str, sizeof(str), "%4d", num); // formatta il numero in 4 caratteri
    MAX72_Print_String(str);
}

void MAX72_Print_Float(float num) {
    int int_part = (int)num;
    int frac_part = abs((int)((num - int_part) * 100));  // due cifre decimale

    char str[5];
    snprintf(str, sizeof(str), "%d.%02d", int_part, frac_part);
    MAX72_Print_String(str);
}



// Configuration
#define DEV_NUM         4
#define CHAR_SPACING    1     // number of blank columns between characters
#define GLYPH_WIDTH     5     // width of fonts in LETTERS

// Buffer: 8 rows x DEV_NUM devices
static uint8_t frame[8][DEV_NUM];

// Send full frame to display (reversed order for correct cascade)
static void MAX72_SendFrame(void) {
    for (uint8_t row = 1; row <= 8; row++) {
        uint8_t pattern[DEV_NUM];
        for (uint8_t d = 0; d < DEV_NUM; d++) {
            pattern[d] = frame[row-1][DEV_NUM - 1 - d];
        }
        MAX72_SendRow(row, pattern);
    }
}

// Scroll text from left to right across cascaded MAX72 matrices in loop
void MAX72_Scroll(const char *text, uint16_t delay_ms) {
    size_t len = strlen(text);

    // Infinite loop
    while (1) {
        for (int idx = len - 1; idx >= 0; idx--) {
            uint8_t ch = (text[idx] < ' ' || text[idx] > '~') ? ' ' : text[idx];
            uint8_t *glyph = LETTERS[ch - ' '].value;

            // Scorri ogni colonna del carattere da sinistra a destra (0 a 4)
            for (uint8_t col = 0; col < GLYPH_WIDTH; col++) {
                for (uint8_t row = 0; row < 8; row++) {
                    // Estrai il bit dalla colonna corrente (da sinistra del carattere)
                    uint8_t new_bit = (glyph[row] >> (GLYPH_WIDTH - 1 - col)) & 0x01;
                    uint8_t carry = new_bit;

                    // CORREZIONE: Shift left partendo dal device 0 (il più a sinistra)
                    for (int d = 0; d < DEV_NUM; d++) {
                        uint8_t next_carry = (frame[row][d] >> 7) & 0x01;
                        frame[row][d] = (frame[row][d] << 1) | carry;
                        carry = next_carry;
                    }
                }
                MAX72_SendFrame();
                HAL_Delay(delay_ms);
            }

            // Aggiungi spazio tra caratteri
            for (uint8_t sp = 0; sp < CHAR_SPACING; sp++) {
                for (uint8_t row = 0; row < 8; row++) {
                    uint8_t carry = 0;  // Bit vuoto per lo spazio
                    for (int d = 0; d < DEV_NUM; d++) {
                        uint8_t next_carry = (frame[row][d] >> 7) & 0x01;
                        frame[row][d] = (frame[row][d] << 1) | carry;
                        carry = next_carry;
                    }
                }
                MAX72_SendFrame();
                HAL_Delay(delay_ms);
            }
        }

        // Padding alla fine prima del loop - aggiungi uno spazio
        for (uint8_t col = 0; col < GLYPH_WIDTH; col++) {
            for (uint8_t row = 0; row < 8; row++) {
                // Spazio vuoto (carattere spazio)
                uint8_t carry = 0;
                for (int d = 0; d < DEV_NUM; d++) {
                    uint8_t next_carry = (frame[row][d] >> 7) & 0x01;
                    frame[row][d] = (frame[row][d] << 1) | carry;
                    carry = next_carry;
                }
            }
            MAX72_SendFrame();
            HAL_Delay(delay_ms);
        }

        // Spazio aggiuntivo dopo il padding del carattere spazio
        for (uint8_t sp = 0; sp < CHAR_SPACING; sp++) {
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t carry = 0;
                for (int d = 0; d < DEV_NUM; d++) {
                    uint8_t next_carry = (frame[row][d] >> 7) & 0x01;
                    frame[row][d] = (frame[row][d] << 1) | carry;
                    carry = next_carry;
                }
            }
            MAX72_SendFrame();
            HAL_Delay(delay_ms);
        }
    }
}

// Struttura per gestire lo stato dello scrolling
typedef struct {
    const char *text;           // Testo da visualizzare
    size_t text_len;           // Lunghezza del testo
    int current_char_idx;      // Indice carattere corrente
    uint8_t current_col;       // Colonna corrente del carattere
    uint8_t spacing_counter;   // Contatore per gli spazi tra caratteri
    uint8_t padding_counter;   // Contatore per il padding finale
    uint8_t state;             // Stato corrente: 0=carattere, 1=spacing, 2=padding
    uint8_t enabled;           // Flag per abilitare/disabilitare lo scrolling
} scroll_state_t;

static scroll_state_t scroll_state = {0};

// Flag per indicare quando aggiornare lo scrolling
static volatile uint8_t scroll_update_flag = 0;

// Da chiamare nell'interrupt del timer - SOLO setta il flag
void MAX72_Scroll_Timer_ISR(void) {
    if (scroll_state.enabled) {
        scroll_update_flag = 1;
    }
}

// Inizializza lo scrolling con un nuovo testo
void MAX72_Scroll_Init(const char *text) {
    // Pulisci il frame
    memset(frame, 0, sizeof(frame));
    MAX72_SendFrame();

    // Inizializza lo stato - CAMBIATO: inizia dal primo carattere per scorrimento inverso
    scroll_state.text = text;
    scroll_state.text_len = strlen(text);
    scroll_state.current_char_idx = 0; // Inizia dal primo carattere
    scroll_state.current_col = 0;
    scroll_state.spacing_counter = 0;
    scroll_state.padding_counter = 0;
    scroll_state.state = 0; // Inizia con il primo carattere
    scroll_state.enabled = 1;
    scroll_update_flag = 0;
}

// Ferma lo scrolling
void MAX72_Scroll_Stop(void) {
    scroll_state.enabled = 0;
}

// Riprende lo scrolling
void MAX72_Scroll_Resume(void) {
    scroll_state.enabled = 1;
}

// Funzione da chiamare nel main loop - NON bloccante
void MAX72_Scroll_Process(void) {
    // Controlla se c'è un update da fare
    if (!scroll_update_flag) return;

    // Reset del flag
    scroll_update_flag = 0;

    if (!scroll_state.enabled || !scroll_state.text) return;

    switch (scroll_state.state) {
        case 0: // Processamento carattere
        {
            uint8_t ch = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
                         scroll_state.text[scroll_state.current_char_idx] > '~') ?
                         ' ' : scroll_state.text[scroll_state.current_char_idx];
            uint8_t *glyph = LETTERS[ch - ' '].value;

            // CAMBIATO: Shift verso destra (da destra verso sinistra)
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t new_bit = (glyph[row] >> scroll_state.current_col) & 0x01;
                uint8_t carry = new_bit;

                // CAMBIATO: Shift right invece di left, da device più a destra
                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.current_col++;

            // Finito il carattere corrente?
            if (scroll_state.current_col >= GLYPH_WIDTH) {
                scroll_state.current_col = 0;
                scroll_state.spacing_counter = 0;
                scroll_state.state = 1; // Passa agli spazi tra caratteri
            }
            break;
        }

        case 1: // Spacing tra caratteri
        {
            // CAMBIATO: Shift di uno spazio vuoto verso destra
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t carry = 0;
                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.spacing_counter++;

            // Finito lo spacing?
            if (scroll_state.spacing_counter >= CHAR_SPACING) {
                scroll_state.spacing_counter = 0;

                // CAMBIATO: Passa al carattere successivo (incrementa invece di decrementare)
                scroll_state.current_char_idx++;
                if (scroll_state.current_char_idx >= scroll_state.text_len) {
                    // Finiti tutti i caratteri, passa al padding finale
                    scroll_state.state = 2;
                    scroll_state.padding_counter = 0;
                } else {
                    // Passa al carattere successivo
                    scroll_state.state = 0;
                }
            }
            break;
        }

        case 2: // Padding finale
        {
            // CAMBIATO: Shift di uno spazio vuoto verso destra
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t carry = 0;
                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.padding_counter++;

            // Finito il padding? (GLYPH_WIDTH + CHAR_SPACING)
            if (scroll_state.padding_counter >= (GLYPH_WIDTH + CHAR_SPACING)) {
                // CAMBIATO: Ricomincia dal primo carattere (indice 0)
                scroll_state.current_char_idx = 0;
                scroll_state.current_col = 0;
                scroll_state.spacing_counter = 0;
                scroll_state.padding_counter = 0;
                scroll_state.state = 0;
            }
            break;
        }
    }

    // Aggiorna il display
    MAX72_SendFrame();
}

// Versione semplificata per iniziare lo scrolling
void MAX72_Start_Scrolling(const char *text) {
    MAX72_Scroll_Init(text);
}
