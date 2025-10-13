#include "stm32f4xx_hal.h"
#include "headers/display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern SPI_HandleTypeDef hspi2;
display_t display;

dict_t LETTERS[95] =
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

uint8_t DIGITS[12][8] = {
		{0b010, 0b000, 0b000, 0b000, 0b000, 0b000, 0b000, 0b000}, // .
		{0b000, 0b000, 0b111, 0b000, 0b000, 0b000, 0b000, 0b000}, // -
        {0b111, 0b101, 0b101, 0b101, 0b111, 0b000, 0b000, 0b000}, // 0
        {0b111, 0b010, 0b010, 0b011, 0b010, 0b000, 0b000, 0b000},
        {0b111, 0b001, 0b111, 0b100, 0b111, 0b000, 0b000, 0b000},
        {0b111, 0b100, 0b111, 0b100, 0b111, 0b000, 0b000, 0b000},
        {0b100, 0b100, 0b111, 0b101, 0b101, 0b000, 0b000, 0b000},
        {0b111, 0b100, 0b111, 0b001, 0b111, 0b000, 0b000, 0b000},
        {0b111, 0b101, 0b111, 0b001, 0b111, 0b000, 0b000, 0b000},
        {0b100, 0b100, 0b100, 0b100, 0b111, 0b000, 0b000, 0b000},
        {0b111, 0b101, 0b111, 0b101, 0b111, 0b000, 0b000, 0b000},
        {0b111, 0b100, 0b111, 0b101, 0b111, 0b000, 0b000, 0b000}, // 9
};

static const uint8_t InitCommands[5][2] = {
    {0x09, 0x00}, // Decode Mode: no decode, matrix mode
    {0x0A, 0x00}, // Intensity: min brightness
    {0x0B, 0x07}, // Scan Limit: all 8 digits
    {0x0C, 0x01}, // Shutdown Register: normal operation
    {0x0F, 0x00}  // Display Test: off
};

void MAX72_Init(display_t *display)
{
    /**
     * @brief Initialize the MAX72xx display
     * 
     * The function initializes the MAX72xx display by sending a series of commands
     * to configure the display settings. The commands are sent to all connected
     * devices in parallel using SPI communication. The display is set to normal
     * operation mode with a minimum brightness level.
     * 
     * @param display Pointer to the display structure to be initialized
     */
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

    display->data_count = 0;        // Initializes the data count to 0
    display->current_index = 0;     // Initializes the current index to 0
    display->data_duration = 5;     // Sets the display duration to 5 seconds
    display->change_enabled = 1;    // Enables automatic data changing
    display->update_enabled = 1;    // Enables automatic data updating
    display->last_change_time = HAL_GetTick(); // Initializes the last change time to the current time
    MAX72_Clear();                  // Clears the display at the beginning
}

void MAX72_SendRow(uint8_t row, uint8_t pattern[DEV_NUM])
{
    /**
     * @brief Send a row of data to the MAX72xx display
     * 
     * This function sends a specific row of data to all connected MAX72xx devices.
     * The row is specified by the `row` parameter, and the data for each device
     * is provided in the `pattern` array. The function constructs a transmission
     * buffer containing the command and data for each device and sends it via SPI.
     * 
     * @param row The row number to send (1-8)
     * @param pattern An array containing the data for each device
     */
    uint8_t txBuf[8];
    for (uint8_t dev=0; dev<DEV_NUM; dev++) {
        txBuf[dev*2]   = row;          // DIG[row] register
        txBuf[dev*2+1] = pattern[dev]; // i-th byte of pattern
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, txBuf, sizeof(txBuf), HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void MAX72_Clear(void)
{
    /**
     * @brief Clear the MAX72xx display
     * 
     * This function clears the entire display by sending empty data to all rows.
     */
	uint8_t emptyPattern[4] = {0, 0, 0, 0};
	    for (uint8_t row=1; row<=8; row++) {
	        MAX72_SendRow(row, emptyPattern);
	    }
}

void MAX72_AddData(display_t *display, display_data_t *data) {
    /**
     * @brief Add data to the display
     * 
     * This function adds a new data entry to the display's data buffer.
     * If the buffer is not full, the new data is appended, and the data
     * count is incremented.
     * 
     * @param display Pointer to the display structure
     * @param data Pointer to the data to be added
     */
	if (display->data_count < MAX_DATA_LENGTH) {
		display->data[display->data_count] = *data; // Adds the data to the display
		display->data_count++;                      // Increments the data count
	}

	if (display->data_count == 1) {
		MAX72_ChangeData(display,1);                // If it's the first data, update the display immediately
	}
}

void MAX72_RemoveData(display_t *display, display_data_t *data) {
    /**
     * @brief Remove data from the display
     * 
     * This function removes a specific data entry from the display's data buffer.
     * If the data is found, it is removed, and the subsequent entries are shifted
     * to fill the gap. The data count is decremented accordingly.
     * 
     * @param display Pointer to the display structure
     * @param data Pointer to the data to be removed
     */
	if (display->data_count == 0) {
		return; // No data to remove
	}

	for (uint8_t i = 0; i < display->data_count; i++) {
		if (display->data[i].data == data->data){
			// Shift subsequent elements back
			for (uint8_t j = i; j < display->data_count - 1; j++) {
				display->data[j] = display->data[j + 1];
			}
			display->data_count--; // Decrement the data count
			if (display->current_index >= display->data_count) {
				display->current_index = 0; // Reset the current index if necessary
			}
			break; // Exit the loop after removing the data
		}
	}
}

void MAX72_UpdateData(display_t *display) {
    /**
     * @brief Update the display with the current data
     * 
     * This function updates the MAX72xx display with the data at the current index.
     * It checks if there are any data entries and if updates are enabled before
     * proceeding. Depending on the type and settings of the current data, it calls
     * the appropriate function to display integers, floats, strings, or scrolling text.
     * 
     * @param display Pointer to the display structure
     */
	if (display->data_count == 0 || !display->update_enabled) {
		return; // No data to display
	}

	display_data_t *current_data = &display->data[display->current_index];

	switch (current_data->settings) {
		case PRINT_INT:
			switch(current_data->type) {
				case DISPLAY_TYPE_INT:
					MAX72_PrintInt(*(int *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT8:
					MAX72_PrintInt((int)*(uint8_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT16:
					MAX72_PrintInt((int)*(uint16_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT32:
					MAX72_PrintInt((int)*(uint32_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT64:
					MAX72_PrintInt((int)*(uint64_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT8:
					MAX72_PrintInt(*(int8_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT16:
					MAX72_PrintInt(*(int16_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT32:
					MAX72_PrintInt(*(int32_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT64:
					MAX72_PrintInt(*(int64_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_FLOAT:
					MAX72_PrintFloat((int)*(float *)current_data->data, current_data->float_decimals, current_data->string_settings);
					break;
			}
			break;
		case PRINT_FLOAT:
			if (current_data->type == DISPLAY_TYPE_FLOAT) {
				MAX72_PrintFloat(*(float *)current_data->data, current_data->float_decimals, current_data->string_settings == 2);
			}
			break;
		case PRINT_STRING:
			if (current_data->type == DISPLAY_TYPE_STRING) {
				MAX72_PrintString((const char *)current_data->data, current_data->string_settings);
			}
			break;
		case PRINT_SCROLL:
			if (current_data->type == DISPLAY_TYPE_STRING) {
				// update scroll_state.updated if text has changed
				if (strcmp(scroll_state.text, (const char *)current_data->data) != 0) {
					scroll_state.updated = 1; // Indicates that the text has been updated

					scroll_state.text = (const char *)current_data->data;
				}
			}
			break;
	}
}

void MAX72_ChangeData(display_t *display, uint8_t force_change) {
    /**
     * @brief Change the displayed data to the next entry
     * 
     * This function changes the currently displayed data to the next entry in the
     * display's data buffer. It checks if there are any data entries and if automatic
     * changes are enabled or forced. The function also respects the duration setting
     * for each data entry before changing. If the new data is set to scroll, it starts
     * the scrolling effect; otherwise, it stops any ongoing scrolling.
     * 
     * @param display Pointer to the display structure
     * @param force_change If set to 1, forces an immediate change of data
     */
	if (display->data_count == 0 || (!display->change_enabled && !force_change)) {
		return; // No data to display or automatic change is disabled
	}

	uint32_t currentTime = HAL_GetTick();
	if (!force_change && currentTime - display->last_change_time < (uint32_t)display->data_duration * 1000) {
		return;
	}
	display->current_index = (display->current_index + 1) % display->data_count;
	display->last_change_time = currentTime;

	if (display->data[display->current_index].settings == PRINT_SCROLL) {
		if(!scroll_state.enabled || display->data_count > 1){
		MAX72_ScrollStart_IT((const char *)display->data[display->current_index].data);}
	} else if (scroll_state.enabled) {
		MAX72_ScrollStop();
	}

	MAX72_UpdateData(display); // Update the displayed data
}

void MAX72_StopChangingData(display_t *display, uint8_t stop_update) {
    /**
     * @brief Stop changing the displayed data
     * 
     * This function stops the automatic changing of the displayed data.
     * If requested, it also stops any ongoing updates to the display.
     * 
     * @param display Pointer to the display structure
     * @param stop_update If set to 1, stops the display updates
     */
	display->change_enabled = 0; // Disable automatic data changing
	if (stop_update) {
		MAX72_StopUpdatingData(display); // Stop automatic data updating
	}
}

void MAX72_ResumeChangingData(display_t *display, uint8_t force_update) {
    /**
     * @brief Resume changing the displayed data
     * 
     * This function resumes the automatic changing of the displayed data.
     * If requested, it forces an immediate update of the display with the
     * current data.
     * 
     * @param display Pointer to the display structure
     * @param force_update If set to 1, forces an immediate update of the display
     */
	display->change_enabled = 1; // Enable automatic data changing
	MAX72_ResumeUpdatingData(display); // Ensure that updating is enabled
	if (force_update) {
		MAX72_UpdateData(display); // Force the data change
	}
}

void MAX72_StopUpdatingData(display_t *display) {
    /**
     * @brief Stop updating the displayed data
     * 
     * This function stops the automatic updating of the displayed data.
     * It sets the update_enabled flag to 0, preventing any further updates
     * until explicitly resumed.
     * 
     * @param display Pointer to the display structure
     */
	display->update_enabled = 0; // Stop automatic data updating  
}

void MAX72_ResumeUpdatingData(display_t *display) {
    /**
     * @brief Resume updating the displayed data
     * 
     * This function resumes the automatic updating of the displayed data.
     * It sets the update_enabled flag to 1, allowing updates to occur
     * as per the current settings.
     * 
     * @param display Pointer to the display structure
     */
	display->update_enabled = 1; // Enable automatic data updating
}
