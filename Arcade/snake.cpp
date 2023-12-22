#include "arcade.h"
#include "snake.h"
#include "pt.h"

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

static unsigned long timestamp;
static struct pt pt1;

int snake_length;

/* Head of snake is first element in array */
int x_snake[NUM_LEDS]; // Array of x coordinates of snake
int y_snake[NUM_LEDS]; // Array of y coordinates of snake

// Direction snake is moving
int dx, dy;

// x,y coordinates of apple
int x_apple, y_apple;

bool button_ready;


void newSnakeGame()
{
  turnOffLEDs();

  // Reset variables
  score = 0;
  led_speed = 350;
  snake_length = 1;
  game_over = false;
  button_ready = true;
  timestamp = millis();

  for (int i = 0; i < NUM_LEDS; i++){
    x_snake[i] = 0; y_snake[i] = 0;
  }

  // Set start position and direction of snake
  x_snake[0] = 4; y_snake[0] = 8;
  leds[76] = CRGB(0,255,0);
  dx = 0; dy = -1;

  moveApple();
  updateLCD();
}

void moveSnake()
{
  int x_tail = x_snake[snake_length-1];
  int y_tail = y_snake[snake_length-1];

  // Turn off LED of snake's tail
  leds[y_tail*NUM_COLUMNS + x_tail] = CRGB(0,0,0);
  
  // Calculate next coordinate of the snake's head
  int x_head = x_snake[0] + dx;
  int y_head = y_snake[0] + dy;

  // Wrap around
  if (x_head < 0) x_head = NUM_COLUMNS-1;
  else if (x_head > NUM_COLUMNS-1) x_head = 0;

  if (y_head < 0) y_head = NUM_ROWS-1;
  else if (y_head > NUM_ROWS-1) y_head = 0;

  // Move snake's body
  for (int i = snake_length-1; i > 0; i--){
    int x_body = x_snake[i-1];
    int y_body = y_snake[i-1];
    
    x_snake[i] = x_body;
    y_snake[i] = y_body;

    // Turn on LEDs of snake's body
    leds[y_body*NUM_COLUMNS + x_body] = CRGB(0,255,0);
  }

  // Update snake's head
  x_snake[0] = x_head;
  y_snake[0] = y_head;

  // Turn on LED of snake's new head
  leds[y_head*NUM_COLUMNS + x_head] = CRGB(0,255,0);
  FastLED.show();


  // Snake bit itself -> game over
  if (snakeBitItself()){
    leds[y_head*NUM_COLUMNS + x_head] = CRGB(255,255,0);
    FastLED.show();
    
    game_over = true;
    updateLCD();

    delay(3000);
    newSnakeGame();
  }

  // Snake ate apple
  else if (x_head == x_apple && y_head == y_apple){
    growSnake();

    // All apples eaten -> game over
    if (snake_length == NUM_LEDS){
      game_over = true;
      updateLCD();
  
      delay(3000);
      newSnakeGame();
    }
    else moveApple();
  }
}

void moveApple()
{  
  bool apple_moved = false;
  
  // Randomly move apple to a square the snake doesn't occupy
  while (!apple_moved){
    
    x_apple = random(0, NUM_COLUMNS);
    y_apple = random(0, NUM_ROWS);

    apple_moved = true;

    // Make sure apple didn't move somewhere the snake is
    for (int i = 0; i < snake_length; i++){
      if (x_apple == x_snake[i] && y_apple == y_snake[i]){
        apple_moved = false;
        break;
      }
    }

  } // while loop

  // Turn on LED of new apple
  leds[y_apple*NUM_COLUMNS + x_apple] = CRGB(255,0,0);
  FastLED.show();
}

bool snakeBitItself()
{
  for (int i = 1; i < snake_length; i++){
    if (x_snake[0] == x_snake[i] && y_snake[0] == y_snake[i]){      
      return true; // Snake head collided with part of its body
    }
  }
  return false;
}

void growSnake()
{
  score++;
  updateLCD();

  x_snake[snake_length] = x_snake[snake_length-1];
  y_snake[snake_length] = y_snake[snake_length-1];

  snake_length++;
}

static int protothread1(struct pt *pt)
{  
  PT_BEGIN(pt);
  while (true){
    PT_WAIT_UNTIL(pt, millis() - timestamp > led_speed );
    timestamp = millis(); // take a new timestamp
    
    moveSnake();
    button_ready = true;
  }
  PT_END(pt);
}

void playSnake()
{
  // Thread that moves snake
  protothread1(&pt1);

  // Move up
  if (yellow_button_pressed){
    yellow_button_pressed = false;
    
    if (dy != -1 && button_ready){
      button_ready = false;
      dx = 0; dy = 1;
    }
  }
  
  // Move left
  else if (blue_button_pressed){
    blue_button_pressed = false;
    
    if (dx != 1 && button_ready){
      button_ready = false;
      dx = -1; dy = 0;
    }
  }

  // Move right
  else if (red_button_pressed){
    red_button_pressed = false;
    
    if (dx != -1 && button_ready){
      button_ready = false;
      dx = 1; dy = 0;
    }
  }

  // Move down
  else if (green_button_pressed){
    green_button_pressed = false;
    
    if (dy != 1 && button_ready){
      button_ready = false;
      dx = 0; dy = -1;
    }
  }
}
