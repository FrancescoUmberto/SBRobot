#include "stm32f4xx_hal.h"
#include "headers/display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Buffer: 8 rows x DEV_NUM devices
static uint8_t frame[8][DEV_NUM];

static void MAX72_SendFrame(void) {
    /** 
     * @brief Send the entire frame buffer to the MAX7219 devices.
    */
    for (uint8_t row = 1; row <= 8; row++) {
        uint8_t pattern[DEV_NUM];
        for (uint8_t d = 0; d < DEV_NUM; d++) {
            pattern[d] = frame[row-1][DEV_NUM - 1 - d];
        }
        MAX72_SendRow(row, pattern);
    }
}

void MAX72_Scroll(const char *text, uint16_t delay_ms) {
    /**
     * @brief Scroll text from left to right across cascaded MAX72 matrices in loop
     *
     * @param text the string to scroll (only characters from ' ' to '~' are supported)
     * @param delay_ms delay in milliseconds between each column shift
     */
    
    size_t len = strlen(text);

    while (1) {
        for (int idx = len - 1; idx >= 0; idx--) {
            uint8_t ch = (text[idx] < ' ' || text[idx] > '~') ? ' ' : text[idx];
            uint8_t *glyph = LETTERS[ch - ' '].value;

            // Scroll each column of the character from left to right (0 to 4)
            for (uint8_t col = 0; col < GLYPH_WIDTH; col++) {
                for (uint8_t row = 0; row < 8; row++) {
                    // Extract the bit from the current column (from the left of the character)
                    uint8_t new_bit = (glyph[row] >> (GLYPH_WIDTH - 1 - col)) & 0x01;
                    uint8_t carry = new_bit;

                    for (int d = 0; d < DEV_NUM; d++) {
                        uint8_t next_carry = (frame[row][d] >> 7) & 0x01;
                        frame[row][d] = (frame[row][d] << 1) | carry;
                        carry = next_carry;
                    }
                }
                MAX72_SendFrame();
                HAL_Delay(delay_ms);
            }

            // Add space between characters
            for (uint8_t sp = 0; sp < CHAR_SPACING; sp++) {
                for (uint8_t row = 0; row < 8; row++) {
                    uint8_t carry = 0;  // Empty bit for space
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

        // Padding at the end before the loop - add one space
        for (uint8_t col = 0; col < GLYPH_WIDTH; col++) {
            for (uint8_t row = 0; row < 8; row++) {
                // Empty space (space character)
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

        // Additional space after padding the space character
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

void MAX72_Scroll_Start_IT(const char *text) {
    /**
     * @brief Initialize scrolling text with interrupts (non-blocking)
     *
     * @param text the string to scroll (only characters from ' ' to '~' are supported)
     * Note: Call MAX72_Scroll_Process() in the main loop to update the display
     */

    // Clear the frame
    memset(frame, 0, sizeof(frame));
    MAX72_SendFrame();

    // Initialize state
    scroll_state.text = text;
    scroll_state.current_char_idx = 0; // Start from the first character
    scroll_state.current_char = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
            scroll_state.text[scroll_state.current_char_idx] > '~') ?
            ' ' : scroll_state.text[scroll_state.current_char_idx];
    scroll_state.current_col = 0;
    scroll_state.spacing_counter = 0;
    scroll_state.padding_counter = 0;
    scroll_state.state = 0; // Start with the first character
    scroll_state.enabled = 1;
    scroll_state.updated = 0; // Indicates that scrolling has been updated

    uint8_t len = strlen(text);
    uint8_t temp = len * GLYPH_WIDTH + (len - 1) * CHAR_SPACING;
    uint8_t max_chars = ((DEV_NUM * 8 < temp) ? DEV_NUM*8 : temp) - 8; // Initial padding of 8 columns
    for (uint8_t i = 0; i < max_chars; i++) {
		MAX72_Scroll_Process(); // Process the first character to initialize the frame
	}
}

void MAX72_Scroll_Stop(void) {
    /**
     * @brief Stop scrolling text
     * 
     */
    scroll_state.enabled = 0;
}

void MAX72_Scroll_Resume(void) {
    /**
     * @brief Resume scrolling text
     * 
     */
    scroll_state.enabled = 1;
}

void MAX72_Scroll_Process(void) {
    /**
     * @brief Process one step of scrolling text (to be called in main loop)
     * 
     */
    if (!scroll_state.enabled || !scroll_state.text) return;

    if (scroll_state.updated) {
		// If the text has been updated, recalculate the frame
		scroll_state.updated = 0; // Reset flag
	}

    switch (scroll_state.state) {
        case 0: // Process character
        {
            uint8_t ch = scroll_state.current_char;
            uint8_t *glyph = LETTERS[ch - ' '].value;

            for (uint8_t row = 0; row < 8; row++) {
                uint8_t new_bit = (glyph[row] >> scroll_state.current_col) & 0x01;
                uint8_t carry = new_bit;

                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.current_col++;

            if (scroll_state.current_col >= GLYPH_WIDTH) {
                scroll_state.current_col = 0;
                scroll_state.spacing_counter = 0;
                scroll_state.state = 1;
            }
            break;
        }

        case 1: // Spacing between characters
        {
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t carry = 0;
                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.spacing_counter++;

            if (scroll_state.spacing_counter >= CHAR_SPACING) {
                scroll_state.spacing_counter = 0;

                scroll_state.current_char_idx++;
                if (scroll_state.current_char_idx >= strlen(scroll_state.text)) {
                    scroll_state.state = 2;
                    scroll_state.padding_counter = 0;
                } else {
                	scroll_state.current_char = (scroll_state.text[scroll_state.current_char_idx] < ' ' ||
                	                         scroll_state.text[scroll_state.current_char_idx] > '~') ?
                	                         ' ' : scroll_state.text[scroll_state.current_char_idx];
                    scroll_state.state = 0;
                }
            }
            break;
        }

        case 2:
        {
            for (uint8_t row = 0; row < 8; row++) {
                uint8_t carry = 0;
                for (int d = DEV_NUM - 1; d >= 0; d--) {
                    uint8_t next_carry = frame[row][d] & 0x01;
                    frame[row][d] = (frame[row][d] >> 1) | (carry << 7);
                    carry = next_carry;
                }
            }

            scroll_state.padding_counter++;

            if (scroll_state.padding_counter >= (GLYPH_WIDTH + CHAR_SPACING)) {
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

    MAX72_SendFrame();
}
