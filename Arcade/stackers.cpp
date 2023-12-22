#include "arcade.h"
#include "stackers.h"

#include <FastLED.h>
#include <LiquidCrystal.h>


// GLOBAL VARIABLES
extern LiquidCrystal lcd;
extern CRGB leds[NUM_LEDS];

extern int yellow_button_pressed;
extern int red_button_pressed;
extern int blue_button_pressed;
extern int green_button_pressed;

extern bool game_over;
extern int led_speed;
extern int score;

// led_state: 2D integer array representing the state of each LED
// 1 = ON, 0 = OFF, -1 = Flash RED then turn OFF
int led_state[NUM_ROWS][NUM_COLUMNS];

int left_bound;  // Left-most  square of the current row. Always current_row * NUM_COLUMNS
int right_bound; // Right-most square of the current row. Always left_bound  + NUM_COLUMNS - 1

int led_direction; // Direction the LEDs are moving. 1 = moving right, -1 = moving left
int led_width;     // Number of LEDs moving. Game starts with led_width = 3, and ends if led_width <= 0

int left_led;   // Position of left-most  square of moving LEDs {0 ... 143} inclusive
int center_led; // Position of the center square of moving LEDs {1 ... 143} inclusive
int right_led;  // Position of right-most square of moving LEDs {2 ... 143} inclusive

/*
NOTE: 
if led_width == 1, left_led = center_led = right_led
if led_width == 2, left_led = center_led, right_led is different
if led_width == 3, left_led, center_led, and right_led are all different
*/

int current_row; // The current row of moving LEDs. {0 .. 15} inclusive


void newStackersGame()
{
  turnOffLEDs();

  // Reset variables
  for (int i = 0; i < NUM_ROWS; i++){
    for (int j = 0; j < NUM_COLUMNS; j++){
      led_state[i][j] = 0;
    }
  }
  
  led_direction = 1;
  led_speed = 125;
  led_width = 3;

  game_over = false;
  current_row = -1;
  score = 0;
  
  updateLevel();
}

void playStackers()
{
  if (yellow_button_pressed || red_button_pressed || blue_button_pressed || green_button_pressed){
    yellow_button_pressed = false;
    red_button_pressed    = false;
    blue_button_pressed   = false;
    green_button_pressed  = false;
    
    stopLeds();
  }
  else{
    moveLeds();
    delay(led_speed);
  }
}

void moveLeds()
{
  leds[left_led]   = CRGB(0,0,0);
  leds[center_led] = CRGB(0,0,0);
  leds[right_led]  = CRGB(0,0,0);
  
  if (left_led == left_bound) led_direction = 1;
  else if (right_led == right_bound) led_direction = -1;

  left_led   += led_direction;
  center_led += led_direction;
  right_led  += led_direction;
  
  leds[left_led]   = CRGB(0,0,255);
  leds[center_led] = CRGB(0,0,255);
  leds[right_led]  = CRGB(0,0,255);

  FastLED.show();
}

void stopLeds()
{ 
  leds[left_led]   = CRGB(0,0,255);
  leds[center_led] = CRGB(0,0,255); 
  leds[right_led]  = CRGB(0,0,255);

  FastLED.show();
  
  led_state[current_row][left_led  %NUM_COLUMNS] = 1;
  led_state[current_row][center_led%NUM_COLUMNS] = 1;
  led_state[current_row][right_led %NUM_COLUMNS] = 1;
  
  if (current_row > 0) checkAlignment();
  else updateLevel();
}

void checkAlignment()
{ 
  int squaresMissed = 0;
  
  for (int i = left_led; i <= right_led; i++){ 
    if (!led_state[current_row-1][i%9]){ 
      led_state[current_row][i%9] = -1;
      squaresMissed++;
    }
  }

  if (squaresMissed > 0){
    loseSquaresAnimation();
    led_width -= squaresMissed;
  }
  
  if (led_width <= 0){
    game_over = true;
    updateLCD();
    
    loseGameAnimation();
    newStackersGame();
  }
  else if (current_row == NUM_ROWS-1){
    score += (4-led_width)*(current_row+1);
    updateLCD();
  
    winGameAnimation();
    newStackersGame();
  }
  else updateLevel(); 
}

void updateLevel()
{
  led_speed -= 5;
  current_row++;
  
  left_bound  = current_row * NUM_COLUMNS;
  right_bound = left_bound  + NUM_COLUMNS - 1; 

  center_led = random(left_bound+1, right_bound);  
  left_led   = center_led;
  right_led  = center_led;

  if (led_width >= 2) right_led++;
  if (led_width == 3) left_led--;

  score += (4-led_width)*current_row;
  updateLCD();
}

void loseSquaresAnimation()
{
  // Blink red 3 times
  for (int i = 0; i < 3; i++){
    
    // Turn red
    for (int i = left_led; i <= right_led; i++){
      if (led_state[current_row][i%9] == -1){
        leds[i] = CRGB(255,0,0);
      }
    }

    FastLED.show();
    delay(100);

    // Turn off
    for (int i = left_led; i <= right_led; i++){
      if (led_state[current_row][i%9] == -1){
        leds[i] = CRGB(0,0,0);
      }
    }
    
    FastLED.show();
    delay(100);
  }

  for (int i = left_led; i <= right_led; i++){
    if (led_state[current_row][i%9] == -1){
      led_state[current_row][i%9] = 0;
    }
  }
}

void loseGameAnimation()
{
  // Loop through each row from top down
  for (int i = current_row; i >= 0; i--){
    
    // Turn LEDs that are on red
    for (int j = 0; j < NUM_COLUMNS; j++){
      if (led_state[i][j]) leds[i*NUM_COLUMNS + j] = CRGB(255,0,0);
    }
    
    FastLED.show();
    delay(150);

    // Turn off
    for (int j = 0; j < NUM_COLUMNS; j++){
      leds[j+i*NUM_COLUMNS] = CRGB(0,0,0);
    }
    
    FastLED.show();
  }
}

void winGameAnimation()
{  
  // Loop through each row from bottom up
  for (int i = 0; i < NUM_ROWS; i++){
    
    // Turn LEDs that are on green
    for (int j = 0; j < NUM_COLUMNS; j++){
      if (led_state[i][j]) leds[i*NUM_COLUMNS + j] = CRGB(0,255,0); 
    }
    
    FastLED.show();
    delay(100);
  }
  
  delay(700);  
}
