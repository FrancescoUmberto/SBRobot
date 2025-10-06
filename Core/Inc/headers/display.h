#ifndef MAX7219_H_
#define MAX7219_H_

#define MAX_DATA_LENGTH 8 		// Maximum length of data to be displayed
#define DEV_NUM         4
// Scrolling
#define CHAR_SPACING    1     	// Number of blank columns between characters
#define GLYPH_WIDTH     5     	// Width of fonts in LETTERS

enum String_Settings {
    NO_SETTINGS = 0, 			// No special settings, just print the string as is
    FLOAT = 1, 					// When printing FLOAT with decimal point, without minidigits
    MINIDIGITS = 2 				// When printing INT and FLOAT
};

enum Display_Print_Settings {
	PRINT_INT = 0,
	PRINT_FLOAT = 1,
	PRINT_STRING = 2,
	PRINT_SCROLL = 3,
};

typedef struct {
    char key;
    uint8_t value[8];
} dict_t;

extern dict_t LETTERS[95];
extern uint8_t DIGITS[12][8];

typedef enum {
	DISPLAY_TYPE_INT,
	DISPLAY_TYPE_UINT8,
	DISPLAY_TYPE_UINT16,
	DISPLAY_TYPE_UINT32,
	DISPLAY_TYPE_UINT64,
	DISPLAY_TYPE_INT8,
	DISPLAY_TYPE_INT16,
	DISPLAY_TYPE_INT32,
	DISPLAY_TYPE_INT64,
	DISPLAY_TYPE_FLOAT,
	DISPLAY_TYPE_STRING,
} display_data_type_t;

// Generic pointer to variable that contains the data to be displayed
typedef struct {
	void *data; 				// Pointer to the data to be displayed
	uint8_t settings; 			// Display_Print_Settings
	uint8_t string_settings; 	// String_Settings
	display_data_type_t type; 	// Type of data to be displayed
	uint8_t float_decimals; 	// Number of decimal places for float data
} display_data_t;

typedef struct {
	display_data_t data[MAX_DATA_LENGTH]; 	// Array of data to be displayed
	uint8_t data_count; 					// Number of data items in the array (0-MAX_DATA_LENGTH)
	uint8_t current_index; 					// Index of the currently displayed data
	uint8_t data_duration; 					// Duration to display each data item in seconds (1-25)
	uint8_t change_enabled; 				// Flag to enable/disable automatic change of displayed data
	uint8_t update_enabled; 				// Flag to enable/disable automatic update of displayed data
	uint32_t last_change_time; 				// Timestamp of the last data change (in milliseconds)
} display_t;

extern display_t display; // Global display structure

void MAX72_Init(display_t *display);
void MAX72_SendRow(uint8_t row, uint8_t pattern[DEV_NUM]);
void MAX72_Clear(void);

void MAX72_AddData(display_t *display, display_data_t *data);
void MAX72_RemoveData(display_t *display, display_data_t *data);
void MAX72_ChangeData(display_t *display, uint8_t force_change);
void MAX72_UpdateData(display_t *display);
void MAX72_StopChangingData(display_t *display, uint8_t stop_update);
void MAX72_ResumeChangingData(display_t *display, uint8_t force_update);
void MAX72_StopUpdatingData(display_t *display);
void MAX72_ResumeUpdatingData(display_t *display);

//void Print_Alphabet(void);

void MAX72_PrintString(const char *str, uint8_t settings);
void MAX72_PrintInt(int num, uint8_t minidigits);
void MAX72_PrintFloat(float num, uint8_t decimals, uint8_t minidigits);

// Structure to hold the state of scrolling
typedef struct {
    const char *text;           // Text to display
    int current_char_idx;       // Current character index
    uint8_t current_char;       // Current character to display
    uint8_t current_col;       	// Current column of the character
    uint8_t spacing_counter;   	// Counter for spaces between characters
    uint8_t padding_counter;   	// Counter for final padding
    uint8_t state;             	// Current state: 0=character, 1=spacing, 2=padding
    uint8_t enabled;			// Flag to enable/disable scrolling
    uint8_t updated;			// Flag to indicate if the text has been updated
} scroll_state_t;

extern scroll_state_t scroll_state;

// Blocking (no interrupt)
void MAX72_Scroll(const char *text, uint16_t delay_ms);

// With interrupt
void MAX72_Scroll_Start_IT(const char *text);
void MAX72_Scroll_Process(void);
void MAX72_Scroll_Resume(void);
void MAX72_Scroll_Stop(void);
#endif /* MAX7219_H_ */
