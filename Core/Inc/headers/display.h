#ifndef MAX7219_H_
#define MAX7219_H_

#define MAX_DATA_LENGTH 8 // Maximum length of data to be displayed
#define DEV_NUM         4
// Scrolling
#define CHAR_SPACING    1     // number of blank columns between characters
#define GLYPH_WIDTH     5     // width of fonts in LETTERS

enum String_Settings {
    NO_SETTINGS = 0, // No special settings, just print the string as is
    FLOAT = 1, // When printing FLOAT with decimal point, without minidigits
    MINIDIGITS = 2 // When printing INT and FLOAT
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
	void *data; // Pointer to the data to be displayed
	uint8_t settings; // Display_Print_Settings
	uint8_t string_settings; // String_Settings
	display_data_type_t type; // Type of data to be displayed
	uint8_t float_decimals; // Number of decimal places for float data
} display_data_t;

typedef struct {
	display_data_t data[MAX_DATA_LENGTH]; // Array of data to be displayed
	uint8_t data_count; // Number of data items in the array (0-MAX_DATA_LENGTH)
	uint8_t current_index; // Index of the currently displayed data
	uint8_t data_duration; // Duration to display each data item in seconds (1-25)
	uint8_t change_enabled; // Flag to enable/disable automatic change of displayed data
	uint8_t update_enabled; // Flag to enable/disable automatic update of displayed data
	uint32_t last_change_time; // Timestamp of the last data change (in milliseconds)
} display_t;

extern display_t display; // Global display structure

void MAX72_init(display_t *display);
void MAX72_SendRow(uint8_t row, uint8_t pattern[DEV_NUM]);
void MAX72_Clear(void);

void MAX72_Add_Data(display_t *display, display_data_t *data);
void MAX72_Update_Data(display_t *display);
void MAX72_Change_Data(display_t *display, uint8_t force_change);
void MAX72_Stop_Changing_Data(display_t *display, uint8_t stop_update);
void MAX72_Resume_Changing_Data(display_t *display);
void MAX72_Stop_Updating_Data(display_t *display);
void MAX72_Resume_Updating_Data(display_t *display);

//void Print_Alphabet(void);

void MAX72_Print_String(const char *str, uint8_t settings);
void MAX72_Print_Int(int num, uint8_t minidigits);
void MAX72_Print_Float(float num, uint8_t decimals, uint8_t minidigits);

// Struttura per gestire lo stato dello scrolling
typedef struct {
    const char *text;           // Testo da visualizzare
    int current_char_idx;      // Indice carattere corrente
    uint8_t current_char;         // Carattere corrente da visualizzare
    uint8_t current_col;       // Colonna corrente del carattere
    uint8_t spacing_counter;   // Contatore per gli spazi tra caratteri
    uint8_t padding_counter;   // Contatore per il padding finale
    uint8_t state;             // Stato corrente: 0=carattere, 1=spacing, 2=padding
    uint8_t enabled;
    uint8_t updated;// Flag per abilitare/disabilitare lo scrolling
} scroll_state_t;

extern scroll_state_t scroll_state;

// Senza Interrupt (Bloccante)
void MAX72_Scroll(const char *text, uint16_t delay_ms);

// Con interrupt
void MAX72_Scroll_Start_IT(const char *text);
void MAX72_Scroll_Process(void);
void MAX72_Scroll_Resume(void);
void MAX72_Scroll_Stop(void);
#endif /* MAX7219_H_ */
