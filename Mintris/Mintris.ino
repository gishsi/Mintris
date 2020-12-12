#include <AberLED.h>
#include "letters.h"
// this is an invalid state: if we are in this state, things have
// gone badly wrong! The setup function should change this immediately.
#define S_INVALID   -1
#define S_START     0
#define S_GAME      1
#define S_END       2

/*************************************************************
   Other parts of the model than the state
 *************************************************************/

/*************************************************************
   GRID MODEL
   1 - a red block
 *************************************************************/
int grid[8][8]; // 8x8 grid for the game

void renderGrid() {
  for (int x = 0; x < 8; x ++ ) {
    for (int y = 0; y < 8; y++ ) {
      switch (grid[x][y]) {
        case 0:
          break;
        case 1:
          AberLED.set(x, y, RED);
          break;
        default:
          break;
      }
    }
  }
}
// we must go in a reversed order because otherwise we would overwrite rows (copy the one row over and over again)
void scrollGridDown(int rowY) {
  for (int x = 0; x < 8; x++) {
    for (int y = rowY - 1; y >= 0; y--) {
      grid[x][y + 1] = grid[x][y];
    }
  }
}
// check if the given row is full
bool isRowFull(int rowY) {
  for (int x = 0; x < 8; x++) {
    if (grid[x][rowY] != 1) {
      return false;
    }
  }
  return true;
}

/*************************************************************
  MOVE TOWER UP
*************************************************************/

unsigned const long moveTowerUpInterval = 20000L;
unsigned long moveTowerUpEvent;
// check if it won't go outside the 8x8 grid if it moves up
// move everything  up
// overwrite the bottom row
void scrollGridUp() {
  // must only move blocks
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      if (grid[x][y] == 1) {
        grid[x][y - 1] = grid[x][y];
      }
    }
  }
}
bool canItMoveUp() {
  // the grid cannot be moved up if there is atleast one red block in the top row
  for (int x = 0; x < 8; x++) {
    if (grid[x][0] == 1) {
      return false;
    }
  }
  return true;
}

// initialize the grid
void initGrid() {
  for (int y = 0; y < 8; y ++) {
    for (int x = 0; x < 8; x++) {
      grid[x][y] = 0;
    }
  }
}

/*************************************************************
   PLAYER MODEL
 *************************************************************/
int playerX;
int playerY;
int playerScore;

// timer variables responsible for the player falling
unsigned long fallingEvent;
unsigned long fallingInterval; // assigned in setup, not in the initPlayer() ( would overwrite pogressivelyFaster())


void movePlayerLeft() {
  if (playerX > 0) {
    if (grid[playerX - 1][playerY] != 1) {
      playerX--;
    } else {
      gameOver();
    }
  }
}
void movePlayerRight() {
  /*
     i'm doing double if because the else only applies to the second condition
  */
  if (playerX < 7) {
    if (grid[playerX + 1][playerY] != 1) {
      playerX++;
    } else {
      gameOver();
    }
  }
}
/*
   movePlayerDown() - additional
*/
void movePlayerDown() {
  if (grid[playerX][7] != 1) { // i need to check for the bottom row
    // if it is empty then I need to place a red block there
    playerY = 7;
    //grid[playerX][playerY] = 1;
  } else  {
    // must be 8 so that the if condition triggers correctly
    // if it is not I put a block in the previous y position
    for (int y = 0; y < 8; y++) {
      if (grid[playerX][y] == 1) {
        playerY = y - 1;
        grid[playerX][playerY] = 1;
      }
    }
  }
}

/*
   Player falls faster - additional
*/
unsigned const long fasterInterval = 5000L; // every 5 seconds the player starts to fall faster
unsigned long fasterEvent;
// the velocity of the player shoudn't be too high
void pogressivelyFaster() {
  if (fallingInterval >= 100L) {
    fallingInterval -= 10L;
  }
}


void renderPlayer() {
  AberLED.set(playerX, playerY, YELLOW);
}

// can the player be created in the first row? (or is it occupied by a red block?)
bool canItBeCreated(int randomX) {
  if (grid[randomX][0] == 1) {
    return false;
  }
  return true;
}

void initPlayer() {
  playerX = random(0, 8);
  playerY = 0;
  // I am not initializing the fallingInterval here but globaly so
  //that the progressivelyFaster funtion can alter it without being overwritten
  fallingEvent = millis() + fallingInterval;
}

/*************************************************************
   GAME OVER
 *************************************************************/
#define MINUTE 60000
#define SECOND 1000
void gameOver() {
  scoreDisplay();
  gotoState(S_END); // go to the end state
}
//time survived & rows destroyed
void scoreDisplay () { // time survived & rows destroyed
  Serial.print("Your score: ");
  Serial.println(playerScore);
  Serial.print("Time survived: ");
  // using the functions from framework to see how long the player survived forgetStateTime()
  int seconds =  (getStateTime() % MINUTE) / SECOND;
  int minutes = getStateTime() / MINUTE;
  Serial.print(minutes); //minutes
  Serial.print("min ");
  Serial.print(seconds);
  Serial.println(" sec");
}

/*************************************************************
   INIT MODEL
 *************************************************************/
void initTimers() {
  fasterEvent = millis() + fasterInterval;
  moveTowerUpEvent = millis() + moveTowerUpInterval;
}
void initModel() {
  initGrid();
  initPlayer();
  initTimers();
}
/*************************************************************
   DRAWING TO THE GRID 
 *************************************************************/
unsigned long stringEvent;
unsigned const long stringInterval = 400L;
int colour;

// START STATE
int press[7] = {SPACE, P, R, E, S, S, FIVE};
int pressIndex;

//END STATE
int end[4] = {SPACE, E, N, D};
int endIndex;
// FUNCTIONS
void renderString(int letter[8][8], int colour) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      if (letter[i][j] == 1) {
        AberLED.set(j, i, colour);
      }
    }
  }
}
/*************************************************************
   This is the state machine code given in the worksheet, which
   you should use in your assignment too.
 *************************************************************/

// the state variable - starts out invalid
int state = S_INVALID;
// the time the current state was entered
unsigned long stateStartTime;

// always change state by calling this function, never
// set the "state" variable directly. All this does is change
// the "state" variable and record when it happened,
// so that you can respond to "state" in different ways.

void gotoState(int s) {
  // this actually changes the state
  state = s;
  // and this sets the time the new state was entered
  stateStartTime = millis();
}

// get the time the system has been in the current state

unsigned long getStateTime() {
  // return the difference between the current time and
  // the time the state was entered
  return millis() - stateStartTime;
}

/*************************************************************
   End of state machine code
 *************************************************************/

// setup puts the state machine into the first state and
// starts the timer

void setup() {
  AberLED.begin();
  Serial.begin(9600);
  randomSeed(analogRead(0));
  pressIndex = 0;
  initModel();
  gotoState(S_START);
}

/*
   In handleInput, we deal separately with how each state
   handles user input using a switch statement.
*/

// flip this variable when 1 second elapses (for the END state)
bool hasOneSecondElapsed = false;

void handleInput() {
  switch (state) {
    case S_START:
      endIndex = 0; // so that it the END string will start over every time player loses
      if (AberLED.getButtonDown(FIRE)) {
        fallingInterval = 250L;
        playerScore = 0;
        gotoState(S_GAME);
        initModel();
        // when I called this function before going to the game state, in the first instance the model was created
        // the Game State render wouldn't be fast enough to render the user at posistion y = 0;
        // instead it incremented first to y = 1
      }
      break;
    case S_GAME:
      if (AberLED.getButtonDown(4)) {
        movePlayerLeft();
      }
      if (AberLED.getButtonDown(3)) {
        movePlayerRight();
      }
      if (AberLED.getButtonDown(FIRE)) { // delete this later
        gameOver();
      }
      if (AberLED.getButtonDown(DOWN)) {
        movePlayerDown();
      }
      break;
    case S_END:
      if (AberLED.getButtonDown(FIRE) && hasOneSecondElapsed) {
        hasOneSecondElapsed = false; // reset to false
        pressIndex = 0;
        gotoState(S_START);
      }
      break;
    default:
      Serial.println("Invalid state!");
  }
}

/*
   update deals with the model changing over time, rather than in
   response to user input. The state is part of the model, so here
   we deal with changing state over time.
*/
void updateModel() {
  switch (state) {
    case S_START:
       if (millis() > stringEvent) {
        stringEvent = millis() + stringInterval;
        colour = random(1, 4); // random color: red, green and yellow
        pressIndex++;
        if (pressIndex == 7) {
          pressIndex = 0;
        }
      }
      break;
    case S_GAME:
      if (millis() >= fallingEvent) {
        fallingEvent = millis() + fallingInterval;
        playerY++;
      }
      // check if a given X position is not a RED block, end game if it is
      if (!canItBeCreated(playerX)) {
        gameOver();
      }
      // turning the block red and initializing the player again
      if (playerY == 7 || grid[playerX][playerY + 1] == 1) {
        grid[playerX][playerY] = 1;
        initPlayer();
      }
      // if isRowFull returns true then we scroll the grid
      if (isRowFull(7)) {
        scrollGridDown(7);
        playerScore++;
      }
      if (millis() >= fasterEvent) {
        fasterEvent = millis() + fasterInterval;
        pogressivelyFaster();
      }

      if (millis() >= moveTowerUpEvent) {
        moveTowerUpEvent = millis() + moveTowerUpInterval;
        if (canItMoveUp()) {
          scrollGridUp();
        } else {
          gameOver();
        }
      }
      break;
    case S_END:
      if (getStateTime() >= SECOND) {
        hasOneSecondElapsed = true;
      }
      if (millis() > stringEvent) {
        stringEvent = millis() + stringInterval;
        endIndex++;
        if (endIndex == 4) {
          endIndex = 0;
        }
      }
      break;
    default:
      Serial.println("Invalid state!");
  }
}

/*
   The render function is the only place where we draw anything.
   So here, we render different things in each state.
*/

void render() {
  switch (state) {
    case S_START:
      renderString(press[pressIndex], colour);
      break;
    case S_GAME:
      renderPlayer();
      renderGrid();
      break;
    case S_END:
      renderString(end[endIndex], RED);
      break;
    default:
      Serial.println("Invalid state!");
  }
}

/*
   This is the loop function, which just calls the three parts
   of the main loop - handle input, update model, render the model.
   I've added clear/swap around the render call so you don't need to
   worry about putting them into render() yourself.
*/
void loop() {
  handleInput();
  updateModel();
  AberLED.clear();
  render();
  AberLED.swap();
}
