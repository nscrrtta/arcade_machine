#include "arcade.h"
#include "stackers.h"
#include "tetris.h"
#include "snake.h"

#include <FastLED.h>
#include <LiquidCrystal.h>
#include "PinChangeInterrupt.h"


CRGB leds[NUM_LEDS];

LiquidCrystal lcd(
  LCD_PIN_RS,
  LCD_PIN_ENABLE,
  LCD_PIN_DB4,
  LCD_PIN_DB5,
  LCD_PIN_DB6,
  LCD_PIN_DB7
);

// game_mode: 0 = Stackers, 1 = Tetris, 2 = Snake
int game_mode = 0;

static unsigned long last_interrupt_time = 0;
int debounce = 100; // milliseconds

volatile int yellow_button_pressed;
volatile int red_button_pressed;
volatile int blue_button_pressed;
volatile int green_button_pressed;
volatile int white_button_pressed;
volatile int black_button_pressed;

bool game_over;
int led_speed;
int score;


void setup() 
{    
  pinMode(WS2185_DATA_PIN,  OUTPUT);
  LEDS.addLeds<WS2812, WS2185_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  pinMode(BUTTON_PIN_YELLOW, INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_YELLOW), ISR_yellow_button, RISING);

  pinMode(BUTTON_PIN_RED,    INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_RED),    ISR_red_button,    RISING);

  pinMode(BUTTON_PIN_BLUE,   INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_BLUE),   ISR_blue_button,   RISING);

  pinMode(BUTTON_PIN_GREEN,  INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_GREEN),  ISR_green_button,  RISING);

  pinMode(BUTTON_PIN_WHITE,  INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_WHITE),  ISR_white_button,  RISING);

  pinMode(BUTTON_PIN_BLACK,  INPUT);
  attachPCINT(digitalPinToPCINT(BUTTON_PIN_BLACK),  ISR_black_button,  RISING);
  
  lcd.begin(16,2);

  if      (game_mode == 0) newStackersGame();
  else if (game_mode == 1) newTetrisGame();
  else if (game_mode == 2) newSnakeGame();
}

void loop() 
{
  bool new_game = false;

  // Change game mode >
  if (white_button_pressed){
    white_button_pressed = false;
    new_game = true;
    
    game_mode++;
    if (game_mode > 2) game_mode = 0;
  }
  // Change game mode <
  else if (black_button_pressed){
    black_button_pressed = false;
    new_game = true;
    
    game_mode--;
    if (game_mode < 0) game_mode = 2;
  }

  // Stackers
  if (game_mode == 0){
    if (new_game) newStackersGame();
    playStackers();
  }
  
  // Tetris
  else if (game_mode == 1){
    if (new_game) newTetrisGame();
    playTetris();
  }
  
  // Snake
  else if (game_mode == 2){
    if (new_game) newSnakeGame();
    playSnake();  
  }
}

/*
Buttons are positioned in the following way:
 ________________________________________________________________
|                                                                |
|                          ______________             Yellow     |
|                         |              |                       |
| Black       White       |  LCD Screen  |        Blue       Red |
|                         |______________|                       |
|                                                      Green     |
|________________________________________________________________|

Black/White buttons switch between games (Stackers/Tetris/Snake)
Yellow/Red/Green/Blue buttons are used to play the games

Each button triggers an interrupt with the PinChangeInterrupt library
*/

void ISR_yellow_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    yellow_button_pressed = true;
    last_interrupt_time = interrupt_time;
  } 
}

void ISR_red_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    red_button_pressed = true;
    last_interrupt_time = interrupt_time;
  }  
}

void ISR_blue_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    blue_button_pressed = true;
    last_interrupt_time = interrupt_time;
  }  
}

void ISR_green_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    green_button_pressed = true;
    last_interrupt_time = interrupt_time;
  } 
}

void ISR_white_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    white_button_pressed = true;
    last_interrupt_time = interrupt_time;
  } 
}

void ISR_black_button()
{
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > debounce){
    black_button_pressed = true;
    last_interrupt_time = interrupt_time;
  } 
}

void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0,0);

  if (game_over) lcd.print("Game Over!");
  else if (game_mode == 0) lcd.print("Stackers");
  else if (game_mode == 1) lcd.print("Tetris");
  else if (game_mode == 2) lcd.print("Snake");

  lcd.setCursor(0,1);
  lcd.print("Score:");

  int num_digits = 1;

  if (score > 0){
    num_digits = 0;
    int temp = score;

    while (temp != 0) {
      temp /= 10;
      num_digits++;
    }
  }

  lcd.setCursor(16-num_digits, 1);
  lcd.print(score);
}

void turnOffLEDs()
{
  // Turn off all LEDs
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
}
