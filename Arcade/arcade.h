#define LCD_PIN_RS          1
#define LCD_PIN_DB7         9
#define LCD_PIN_DB6        10
#define LCD_PIN_DB5        11
#define LCD_PIN_DB4        12
#define LCD_PIN_ENABLE     13

#define BUTTON_PIN_YELLOW   2
#define BUTTON_PIN_RED      3
#define BUTTON_PIN_BLUE     4
#define BUTTON_PIN_GREEN    5
#define BUTTON_PIN_WHITE    6
#define BUTTON_PIN_BLACK    7

#define WS2185_DATA_PIN     8

#define NUM_COLUMNS  9
#define NUM_ROWS     16
#define NUM_LEDS     NUM_COLUMNS*NUM_ROWS

void updateLCD();
void turnOffLEDs();
