#include <AberLED.h>

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
void gameOver() {
  gotoState(S_END); // go to the end state
}
/*************************************************************
   INIT MODEL
 *************************************************************/
void initModel() {
  initGrid();
  initPlayer();
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
      if (AberLED.getButtonDown(FIRE)) {
        gameOver();
      }
      break;
    case S_END:
      if (AberLED.getButtonDown(FIRE) && hasOneSecondElapsed) {
        hasOneSecondElapsed = false; // reset to false
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
      break;
    case S_END:
      if(getStateTime() >= 1000) {
        hasOneSecondElapsed = true;
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
      AberLED.set(4, 4, GREEN);
      break;
    case S_GAME:
      renderPlayer();
      renderGrid();
      break;
    case S_END:
      AberLED.set(4, 4, RED);
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
