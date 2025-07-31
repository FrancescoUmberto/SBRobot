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

void MAX72_init(display_t *display)
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

    display->data_count = 0; // Inizializza il numero di dati a 0
    display->current_index = 0; // Inizializza l'indice corrente a 0
    display->data_duration = 5; // Imposta la durata di visualizzazione a 5 secondi
    display->change_enabled = 1; // Abilita il cambio automatico dei dati
    display->update_enabled = 1; // Abilita l'aggiornamento automatico dei dati
    display->last_change_time = HAL_GetTick(); // Inizializza il tempo dell'ultimo cambio a ora corrente
    MAX72_Clear(); // Pulisce il display all'inizio
}

void MAX72_SendRow(uint8_t row, uint8_t pattern[DEV_NUM])
{
    uint8_t txBuf[8];
    // pattern[i] = dato per il modulo i
    for (uint8_t dev=0; dev<DEV_NUM; dev++) {
        txBuf[dev*2]   = row;          // registro DIG[row]
        txBuf[dev*2+1] = pattern[dev]; // i‑esimo byte di dati
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, txBuf, sizeof(txBuf), HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void MAX72_Clear(void)
{
	uint8_t emptyPattern[4] = {0, 0, 0, 0};
	    for (uint8_t row=1; row<=8; row++) {
	        MAX72_SendRow(row, emptyPattern);
	    }
}

void MAX72_Add_Data(display_t *display, display_data_t *data) {
	if (display->data_count < MAX_DATA_LENGTH) {
		display->data[display->data_count] = *data; // Aggiungi i dati al display
		display->data_count++; // Incrementa il conteggio dei dati
	}

	if (display->data_count == 1) {
		MAX72_Change_Data(display,1); // Se è il primo dato, aggiorna subito il display
	}
}

void MAX72_Update_Data(display_t *display) {
	if (display->data_count == 0 || !display->update_enabled) {
		return; // Non ci sono dati da visualizzare
	}

	display_data_t *current_data = &display->data[display->current_index];

	switch (current_data->settings) {
		case PRINT_INT:
			switch(current_data->type) {
				case DISPLAY_TYPE_INT:
					MAX72_Print_Int(*(int *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT8:
					MAX72_Print_Int((int)*(uint8_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT16:
					MAX72_Print_Int((int)*(uint16_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT32:
					MAX72_Print_Int((int)*(uint32_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_UINT64:
					MAX72_Print_Int((int)*(uint64_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT8:
					MAX72_Print_Int(*(int8_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT16:
					MAX72_Print_Int(*(int16_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT32:
					MAX72_Print_Int(*(int32_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_INT64:
					MAX72_Print_Int(*(int64_t *)current_data->data, current_data->string_settings);
					break;
				case DISPLAY_TYPE_FLOAT:
					MAX72_Print_Int((int)*(float *)current_data->data, current_data->string_settings);
					break;
			}
			break;
		case PRINT_FLOAT:
			if (current_data->type == DISPLAY_TYPE_FLOAT) {
				MAX72_Print_Float(*(float *)current_data->data, current_data->float_decimals, current_data->string_settings);
			}
			break;
		case PRINT_STRING:
			if (current_data->type == DISPLAY_TYPE_STRING) {
				MAX72_Print_String((const char *)current_data->data, current_data->string_settings);
			}
			break;
		case PRINT_SCROLL:
			if (current_data->type == DISPLAY_TYPE_STRING) {
				scroll_state.text = (const char *)current_data->data;
			}
			break;
	}
}

void MAX72_Change_Data(display_t *display, uint8_t force_change) {
	if (display->data_count == 0 || (!display->change_enabled && !force_change)) {
		return; // Non ci sono dati da visualizzare o il cambio automatico è disabilitato
	}

	uint32_t currentTime = HAL_GetTick();
	if (!force_change && currentTime - display->last_change_time < (uint32_t)display->data_duration * 1000) {
		return;
	}
	display->current_index = (display->current_index + 1) % display->data_count;
	display->last_change_time = currentTime;

	if (display->data[display->current_index].settings == PRINT_SCROLL) {
		if(!scroll_state.enabled || display->data_count > 1){
		MAX72_Scroll_Start_IT((const char *)display->data[display->current_index].data);}
	} else if (scroll_state.enabled) {
		MAX72_Scroll_Stop();
	}

	MAX72_Update_Data(display); // Aggiorna i dati visualizzati)
}

void MAX72_Stop_Changing_Data(display_t *display, uint8_t stop_update) {
	display->change_enabled = 0; // Disabilita il cambio automatico dei dati
	if (stop_update) {
		MAX72_Stop_Updating_Data(display); // Disabilita l'aggiornamento automatico dei dati
	}
}

void MAX72_Resume_Changing_Data(display_t *display) {
	display->change_enabled = 1; // Abilita il cambio automatico dei dati
	MAX72_Resume_Updating_Data(display); // Assicurati che l'aggiornamento sia abilitato
}

void MAX72_Stop_Updating_Data(display_t *display) {
	display->update_enabled = 0; // Disabilita l'aggiornamento automatico dei dati
}

void MAX72_Resume_Updating_Data(display_t *display) {
	display->update_enabled = 1; // Abilita l'aggiornamento automatico dei dati
}
