#include "stm32f4xx_hal.h"
#include "headers/display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//void Print_Alphabet(void)
//{
//    for (uint8_t idx=0; idx<95; idx++) {
//        for (uint8_t row=0; row<=7; row++) {
//            // tutti e 4 i moduli mostrano lo stesso carattere
//            uint8_t pat[4] = {
//                LETTERS[idx].value[row],
//                LETTERS[idx].value[row],
//                LETTERS[idx].value[row],
//                LETTERS[idx].value[row]
//            };
//            MAX72_SendRow(row+1, pat); // rows: 1-8
//        }
//        HAL_Delay(500);
//    }
//}

void MAX72_PrintString(const char *str, uint8_t settings)
{
    /**
     * @brief Display a string on the MAX7219 modules.
     * 
     * This function displays a string on the MAX7219 modules according to the specified settings.
     * 
     * @param str: the string to display (max 8 characters)
     * @param settings: display settings (NO_SETTINGS, MINIDIGITS, FLOAT)
     * 
     * @note Settings can be:
     *  NO_SETTINGS: display as is (up to 4 chars)
     *  MINIDIGITS: display as a number with up to 7 digits, right-aligned (e.g. "  123.45")
     *  FLOAT: display as a float with decimal point (e.g. "12.34", "-0.5", "0.123")
     */
    uint8_t len = strlen(str);

    if (settings == MINIDIGITS) {
    	char padded[8] = {0};
    	        // Initial padding
    	        uint8_t start = (len < 8) ? (8 - len) : 0;
    	        // Initial spaces
    	        for (uint8_t i = 0; i < start; i++)
    	            padded[i] = ' ';
    	        // Copy the string
    	        for (uint8_t i = 0; i < 8 && i < len; i++)
    	            padded[start + i] = str[i];

        char reversed[8];
        for (uint8_t i = 0; i < 8; i++)
            reversed[i] = padded[7 - i];

        for (uint8_t row = 0; row < 8; row++) {
            uint8_t pat[4] = {0};

            for (uint8_t byte_idx = 0; byte_idx < 4; byte_idx++) {
                uint8_t c1 = reversed[byte_idx * 2];
                uint8_t c2 = reversed[byte_idx * 2 + 1];
                uint8_t char1_pattern = 0;
                uint8_t char2_pattern = 0;

                if (c1 >= '0' && c1 <= '9')
                    char1_pattern = DIGITS[c1 - '0' + 2][row] & 0x07;
                else if (c1 == '-')
                    char1_pattern = DIGITS[1][row] & 0x07;
                else if (c1 == '.')
					char1_pattern = DIGITS[0][row] & 0x07;

                if (c2 >= '0' && c2 <= '9')
                    char2_pattern = DIGITS[c2 - '0' + 2][row] & 0x07;
                else if (c2 == '-')
                    char2_pattern = DIGITS[1][row] & 0x07;
                else if (c2 == '.')
                	char2_pattern = DIGITS[0][row] & 0x07;

                pat[byte_idx] = (char1_pattern << 4) | char2_pattern;
            }

            MAX72_SendRow(row + 1, pat);
        }
    }
    else if (settings == FLOAT) {
        char reversed[4] = {0};
        int8_t dot_pos = -1;
        for (uint8_t i = 0; i < len; i++)
            if (str[i] == '.') { dot_pos = i; break; }

        char no_dot_str[5] = {0};
        uint8_t no_dot_len = 0;
        for (uint8_t i = 0; i < len && no_dot_len < 4; i++)
            if (str[i] != '.') no_dot_str[no_dot_len++] = str[i];

        dot_pos = no_dot_len - dot_pos - 1;

        for (uint8_t i = 0; i < 4; i++)
            reversed[i] = (i < no_dot_len) ? no_dot_str[no_dot_len - 1 - i] : ' ';

        for (uint8_t row = 0; row < 8; row++) {
            uint8_t pat[4];
            for (uint8_t i = 0; i < 4; i++)
                pat[i] = LETTERS[reversed[i] - ' '].value[row] << 3;

            if (row == 0) pat[dot_pos] |= 0b010;

            MAX72_SendRow(row + 1, pat);
        }
    }
    else {
        // NO_SETTINGS
        char reversed[4] = {0};
        for (uint8_t i = 0; i < 4 && i < len; i++)
            reversed[i] = str[len - 1 - i];
        for (uint8_t i = len; i < 4; i++)
            reversed[i] = ' ';

        for (uint8_t row = 0; row < 8; row++) {
            uint8_t pat[4];
            for (uint8_t i = 0; i < 4; i++)
                pat[i] = LETTERS[reversed[i] - ' '].value[row] << 3;
            MAX72_SendRow(row + 1, pat);
        }
    }
}


void MAX72_PrintInt(int num, uint8_t minidigits)
{
    /**
     * @brief Print an integer on the MAX72 display.
     * 
     * @param num The integer to print.
     * @param minidigits If true, use minidigits mode.
     */
    char sign = num < 0 ? '-' : '\0';
    unsigned u = (num < 0) ? (unsigned) (-num) : (unsigned) num;

    char str[9]; // 8 digits + null terminator
    if (minidigits) {
        u %= sign=='-'?10000000u:100000000u;
    } else {
        u %= sign=='-'?1000u:10000u;
    }

    if (sign == '-'){
    	snprintf(str, sizeof(str), "-%u", u);
    } else {
    	snprintf(str, sizeof(str), "%u", u);
    }


    MAX72_PrintString(str, minidigits ? MINIDIGITS : NO_SETTINGS);
//    MAX72_Print_String(str);
}

void MAX72_PrintFloat(float num, uint8_t decimals, uint8_t minidigits) {
    /**
     * @brief Print a float on the MAX72 display.
     * 
     * @param num The float to print.
     * @param decimals The number of decimal places to display.
     * @param minidigits If true, use minidigits mode.
     */
    uint8_t max_chars = 8;
    char str[max_chars + 1]; // +1 for null terminator

    // Handle sign
    char sign = (num < 0) ? '-' : '\0';
    float abs_num = (num < 0) ? -num : num;

    // Separate integer and fractional parts
    unsigned int_part = (unsigned) abs_num;
    float frac_part = abs_num - int_part;


    // Calculate how many characters are needed for the integer part
    uint8_t int_digits = (int_part == 0) ? 0 : 1;
    unsigned temp = int_part;
    while (temp >= 10) {
        temp /= 10;
        int_digits++;
    }

    // Calculate available space for decimals
    uint8_t available_for_decimals = max_chars - (sign != '\0' ? 1 : 0) - int_digits - 1; // -1 for the decimal point

    // Limit decimals to the minimum between requested and available
    uint8_t actual_decimals = (decimals < available_for_decimals) ? decimals : available_for_decimals;

    // If there is no space for even one decimal, show only the integer part
    if (available_for_decimals == 0) {
        if (int_part == 0) {
            snprintf(str, sizeof(str), "0");
        } else {
            if (sign != '\0') {
                snprintf(str, sizeof(str), "%c%u", sign, int_part);
            } else {
                snprintf(str, sizeof(str), "%u", int_part);
            }
        }
    } else {
        // Calculate the multiplier for the decimals
        unsigned multiplier = 1;
        for (uint8_t i = 0; i < actual_decimals; i++) {
            multiplier *= 10;
        }

        // Round the fractional part
        unsigned frac_digits = (unsigned) (frac_part * multiplier + 0.5f);

        // Handle overflow in rounding
        if (frac_digits >= multiplier) {
            int_part++;
            frac_digits = 0;
        }

        // Build the string
        if (int_part == 0) {
            // Omit the integer part if it is 0
            if (sign != '\0') {
                snprintf(str, sizeof(str), "%c.%0*u", sign, actual_decimals, frac_digits);
            } else {
                snprintf(str, sizeof(str), ".%0*u", actual_decimals, frac_digits);
            }
        } else {
            // Include the integer part
            if (sign != '\0') {
                snprintf(str, sizeof(str), "%c%u.%0*u", sign, int_part, actual_decimals, frac_digits);
            } else {
                snprintf(str, sizeof(str), "%u.%0*u", int_part, actual_decimals, frac_digits);
            }
        }
    }

    MAX72_PrintString(str, minidigits ? MINIDIGITS : FLOAT);
//    MAX72_Print_String(str);
}
