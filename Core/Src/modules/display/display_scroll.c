#include "stm32f4xx_hal.h"
#include "headers/display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

scroll_state_t scroll_state = {0};

// Inizializza lo scrolling con un nuovo testo
void MAX72_Scroll_Start_IT(const char *text) {
    // Pulisci il frame
    memset(frame, 0, sizeof(frame));
    MAX72_SendFrame();

    // Inizializza lo stato - CAMBIATO: inizia dal primo carattere per scorrimento inverso
    scroll_state.text = text;
    scroll_state.current_char_idx = 0; // Inizia dal primo carattere
    scroll_state.current_char = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
            scroll_state.text[scroll_state.current_char_idx] > '~') ?
            ' ' : scroll_state.text[scroll_state.current_char_idx];
    scroll_state.current_col = 0;
    scroll_state.spacing_counter = 0;
    scroll_state.padding_counter = 0;
    scroll_state.state = 0; // Inizia con il primo carattere
    scroll_state.enabled = 1;
    scroll_state.updated = 0; // Indica che lo scrolling è stato aggiornato

    uint8_t len = strlen(text);
    uint8_t temp = len * GLYPH_WIDTH + (len - 1) * CHAR_SPACING;
    uint8_t max_chars = ((DEV_NUM * 8 < temp) ? DEV_NUM*8 : temp) - 8; // Padding iniziale di 8 colonne
    for (uint8_t i = 0; i < max_chars; i++) {
		MAX72_Scroll_Process(); // Processa il primo carattere per inizializzare il frame
	}
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
    if (!scroll_state.enabled || !scroll_state.text) return;

    if (scroll_state.updated) {
		// Se il testo è stato aggiornato, ricalcola il frame

    	// TODO RICALCOLO DEL FRAME AGGIORNATO, fino al carattere corrente

		scroll_state.updated = 0; // Reset flag
	}

    switch (scroll_state.state) {
        case 0: // Processamento carattere
        {
            uint8_t ch = scroll_state.current_char;
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
                if (scroll_state.current_char_idx >= strlen(scroll_state.text)) {
                    // Finiti tutti i caratteri, passa al padding finale
                    scroll_state.state = 2;
                    scroll_state.padding_counter = 0;
                } else {
                    // Passa al carattere successivo
                	scroll_state.current_char = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
                	                         scroll_state.text[scroll_state.current_char_idx] > '~') ?
                	                         ' ' : scroll_state.text[scroll_state.current_char_idx];
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
                scroll_state.current_char = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
						scroll_state.text[scroll_state.current_char_idx] > '~') ?
						' ' : scroll_state.text[scroll_state.current_char_idx];
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
