#include "arcade.h"
#include "tetris.h"
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

int board[NUM_ROWS][NUM_COLUMNS];

/*
tetromino:
1 = I
2 = J
3 = L
4 = O
5 = S
6 = T
7 = Z
*/
int tetromino;

// rotation index {0, 1, 2, 3}
int rotation_index;

// x, y coordinates of tetromino
int x, y;

// width, height of tetromino
int width, height;


void newTetrisGame()
{
  turnOffLEDs();

  // Reset variables
  for (int i = 0; i < NUM_ROWS; i++){
    for (int j = 0; j < NUM_COLUMNS; j++){
      board[i][j] = 0;
    }
  }

  game_over = false;
  led_speed = 750;
  score = 0;

  newTetromino();
  updateLCD();
}

void newTetromino()
{
  // If second row from top is occupied, spawn an I piece
  // Otherwise, spawn a random piece
  tetromino = random(1,8);
  
  for (int column = 3; column <= 5; column++){
    if (board[14][column] < 0) tetromino = 1;
  }
  
  // I
  if (tetromino == 1){
    board[15][2] = 1;
    board[15][3] = 1;
    board[15][4] = 1;
    board[15][5] = 1;
    
    x = 2; y = 15;
    width = 4; height = 1;
  }

  // J
  else if (tetromino == 2){
    board[15][3] = 2;
    board[14][3] = 2;
    board[14][4] = 2;
    board[14][5] = 2;

    x = 3; y = 15;
    width = 3; height = 2;
  }

  // L
  else if (tetromino == 3){
    board[15][5] = 3;
    board[14][3] = 3;
    board[14][4] = 3;
    board[14][5] = 3;

    x = 3; y = 15;
    width = 3; height = 2;
  }

  // O
  else if (tetromino == 4){
    board[15][3] = 4;
    board[15][4] = 4;
    board[14][3] = 4;
    board[14][4] = 4;

    x = 3; y = 15;
    width = 2; height = 2;
  }

  // S
  else if (tetromino == 5){
    board[15][4] = 5;
    board[15][5] = 5;
    board[14][3] = 5;
    board[14][4] = 5;

    x = 3; y = 15;
    width = 3; height = 2;
  }

  // T
  else if (tetromino == 6){
    board[15][4] = 6;
    board[14][3] = 6;
    board[14][4] = 6;
    board[14][5] = 6;

    x = 3; y = 15;
    width = 3; height = 2;
  }
  
  // Z
  else if (tetromino == 7){
    board[15][3] = 7;
    board[15][4] = 7;
    board[14][4] = 7;
    board[14][5] = 7;

    x = 3; y = 15;
    width = 3; height = 2;   
  }

  timestamp = millis();
  rotation_index = 0;
  updateTetrisLEDs();
}

bool canMoveDown()
{  
  for (int i = y-height+1; i <= y; i++){
    for (int j = x; j < x+width; j++){

      if (board[i][j] > 0){
        if (i == 0) return false; // Hit the floor
        else if (board[i-1][j] < 0) return false; // Hit another piece
      }

    }
  }
  return true;
}

void moveDown()
{  
  for (int i = y-height+1; i <= y; i++){
    for (int j = x; j < x+width; j++){

      int k = board[i][j];

      if (k > 0){
        board[i-1][j] = k;
        board[i][j] = 0;
      }

    }
  }
  y--;
  updateTetrisLEDs();
}

void dropDown()
{
  while (canMoveDown()){
    score += 2;
    moveDown();
  }
}

bool canMoveLeft()
{
  for (int i = y; i > y-height; i--){
    for (int j = x; j < x+width; j++){

      if (board[i][j] > 0){
        if (j == 0) return false; // Hit the left wall
        else if (board[i][j-1] < 0) return false; // Hit another piece
      }
      
    }
  }
  return true;
}

void moveLeft()
{
  for (int i = y; i > y-height; i--){
    for (int j = x; j < x+width; j++){

      int k = board[i][j];

      if (k > 0){
        board[i][j-1] = k;
        board[i][j] = 0;
      }

    }
  }
  x--;
  updateTetrisLEDs();
}

bool canMoveRight()
{  
  for (int i = y; i > y-height; i--){
    for (int j = x+width-1; j >= x; j--){

      if (board[i][j] > 0){
        if (j == NUM_COLUMNS-1) return false; // Hit the right wall
        else if (board[i][j+1] < 0) return false; // Hit another piece
      }

    }
  }
  return true;
}

void moveRight()
{  
  for (int i = y; i > y-height; i--){
    for (int j = x+width-1; j >= x; j--){

      int k = board[i][j];

      if (k > 0){
        board[i][j+1] = k;
        board[i][j] = 0;
      }

    }
  }
  x++;
  updateTetrisLEDs();
}

bool canRotate()
{
  // Rotating an O piece does nothing
  if (tetromino == 4) return false;

  int shape[width][height];
  int a = y-height+1;

  // Create shape of rotated tetromino
  for (int i = a; i <= y; i++){
    for (int j = x; j < x+width; j++){
      
      if (board[i][j] > 0) shape[j-x][i-a] = 1;
      else shape[j-x][i-a] = 0;
    }
  }

  // Swap width and height
  int h = width;
  int w = height;

  // Update rotation index
  int r = rotation_index+1;
  if (r == 4) r = 0;

  // Adjust x,y coordinates
  int tx = x;
  int ty = y;

  // I
  if (tetromino == 1){
    if      (r == 0){ tx-=1; ty-=1; }
    else if (r == 1){ tx+=2; ty+=1; }
    else if (r == 2){ tx-=2; ty-=2; }
    else if (r == 3){ tx+=1; ty+=2; }
  }

  // J, L, S, T, Z
  else{
    if      (r == 0){ tx+=0; ty+=0; }
    else if (r == 1){ tx+=1; ty+=0; }
    else if (r == 2){ tx-=1; ty-=1; }
    else if (r == 3){ tx+=0; ty+=1; }
  }

  // Wall kicks

  if (ty > NUM_ROWS-1) ty = NUM_ROWS-1; // Ceiling
  else if (ty < h-1) ty = h-1; // Floor

  if (tx < 0) tx = 0; // Left wall
  else if (tx > NUM_COLUMNS-w) tx = NUM_COLUMNS-w; // Right wall

  // Check if rotated tetromino would intersect another piece
  for (int i = 0; i < h; i++){
    for (int j = 0; j < w; j++){
      
      if (shape[i][j] > 0 && board[ty-i][tx+j] < 0){
        return false; // Tetromino would intersect another piece
      }
    }
  }

  return true;
}

void rotate()
{  
  int shape[width][height];
  int a = y-height+1;

  // Create shape of rotated tetromino
  for (int i = a; i <= y; i++){
    for (int j = x; j < x+width; j++){
      
      if (board[i][j] > 0){
        shape[j-x][i-a] = tetromino;
        board[i][j] = 0;
      }
      else shape[j-x][i-a] = 0;
    }
  }

  // Swap width and height
  int temp = width;
  width = height;
  height = temp;

  // Update rotation index
  rotation_index++;
  if (rotation_index == 4) rotation_index = 0;

  // Adjust x,y coordinates

  // I
  if (tetromino == 1){
    if      (rotation_index == 0){ x-=1; y-=1; }
    else if (rotation_index == 1){ x+=2; y+=1; }
    else if (rotation_index == 2){ x-=2; y-=2; }
    else if (rotation_index == 3){ x+=1; y+=2; }
  }

  // J, L, S, T, Z
  else{
    if      (rotation_index == 0){ x+=0; y+=0; }
    else if (rotation_index == 1){ x+=1; y+=0; }
    else if (rotation_index == 2){ x-=1; y-=1; }
    else if (rotation_index == 3){ x+=0; y+=1; }
  }

  // Wall kicks

  if (y > NUM_ROWS-1) y = NUM_ROWS-1; // Ceiling
  else if (y < height-1) y = height-1; // Floor

  if (x < 0) x = 0; // Left wall
  else if (x > NUM_COLUMNS-width) x = NUM_COLUMNS-width; // Right wall

  // Place rotated tetromino on board
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      if (shape[i][j] > 0) board[y-i][x+j] = tetromino;
    }
  }

  updateTetrisLEDs();
}

void placePiece()
{
  score += 4;
  
  for (int i = y; i > y-height; i--){
    for (int j = x; j < x+width; j++){

      int k = board[i][j];
      if (k > 0) board[i][j] = -1*k;

    }
  }

  clearRows();
  
  if (gameOver()){
    game_over = true;
    updateLCD();
    
    delay(3000);
    newTetrisGame();
  }
  else{
    newTetromino();
  }
}

void clearRows()
{
  int current_row = 0;
  int num_full_rows = 0;

  // Loop through rows from bottom up
  while (current_row < NUM_ROWS){

    bool row_full = true;

    // Check if current row is full
    for (int column = 0; column < NUM_COLUMNS; column++){

      if (board[current_row][column] == 0){
        row_full = false;
        break;
      }
    }

    // If current row is not full, move onto next row
    if (!row_full){
      current_row++;
    }

    // If row is full, shift all rows above current row down one row
    else{
      num_full_rows++;
      
      for (int row = current_row; row < NUM_ROWS; row++){
        for (int column = 0; column < NUM_COLUMNS; column++){

          if (row == NUM_ROWS-1) board[row][column] = 0; // Clear top row
          else board[row][column] = board[row+1][column]; // Shift down
          
        } // inner for-loop
      } // outer for-loop
      
    } // else
 
  } // while loop

  delay(600);
  updateTetrisLEDs();

  score += 5*num_full_rows*(num_full_rows+1);
  updateLCD();
}

bool gameOver()
{  
  // Check if any piece is touching the ceiling
  for (int i = 0; i < NUM_COLUMNS; i++){
    if (board[NUM_ROWS-1][i] != 0) return true;
  }
  return false;
}

static int protothread1(struct pt *pt)
{  
  PT_BEGIN(pt);
  while (true){
    PT_WAIT_UNTIL(pt, millis() - timestamp > led_speed );
    timestamp = millis(); // take a new timestamp
    
    if (canMoveDown()) moveDown();
    else placePiece();
  }
  PT_END(pt);
}

void playTetris()
{
  // Thread that moves pieces down
  protothread1(&pt1);
  
  // Rotate
  if (yellow_button_pressed){
    yellow_button_pressed = false;
    if (canRotate()) rotate();
  }
  
  // Move left
  else if (blue_button_pressed){
    blue_button_pressed = false;
    if (canMoveLeft()) moveLeft();
  }

  // Move right
  else if (red_button_pressed){
    red_button_pressed = false;
    if (canMoveRight()) moveRight();
  }

  // Drop down
  else if (green_button_pressed){
    green_button_pressed = false;
    if (y < 15) dropDown();
  }
}

void updateTetrisLEDs()
{
  for (int i = 0; i < NUM_ROWS; i++){
    for (int j = 0; j < NUM_COLUMNS; j++){

      int k = board[i][j];
      if (k < 0) k *= -1;

      if      (k == 0) leds[i*NUM_COLUMNS + j] = CRGB(0,  0,  0); // Off
      else if (k == 1) leds[i*NUM_COLUMNS + j] = CRGB(0,255,255); // I
      else if (k == 2) leds[i*NUM_COLUMNS + j] = CRGB(0,  0,255); // J
      else if (k == 3) leds[i*NUM_COLUMNS + j] = CRGB(255,100,0); // L
      else if (k == 4) leds[i*NUM_COLUMNS + j] = CRGB(255,255,0); // O
      else if (k == 5) leds[i*NUM_COLUMNS + j] = CRGB(0, 255, 0); // S
      else if (k == 6) leds[i*NUM_COLUMNS + j] = CRGB(255,0,255); // T
      else if (k == 7) leds[i*NUM_COLUMNS + j] = CRGB(255, 0, 0); // Z
    }
  }
  FastLED.show();
}
