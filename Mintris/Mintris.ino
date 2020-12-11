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
int grid[8][8];
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
    if (grid[playerX - 1][playerY] != 2) {
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
    if (grid[playerX + 1][playerY] != 2) {
      playerX++;
    } else {
      gameOver();
    }
  }
}

void renderPlayer() {
  AberLED.set(playerX, playerY, YELLOW);
}

void initPlayer() {
  playerX = random(0, 8);
  playerY = 0;
  playerScore = 0;
  // I am not initializing the fallingInterval here but globaly so
  //that the progressivelyFaster funtion can alter it without being overwritten
  fallingEvent = millis() + fallingInterval;
}



/*
   ONE SECOND TIMER FOR END STATE
*/
unsigned long endStateTime = 0L;
unsigned long endStateInterval = 1000L;
// flip this variable when 1 second elapses
bool hasOneSecondElapsed = false;
/*************************************************************
   INIT MODEL
 *************************************************************/
void initModel() {
  initPlayer();
}

/*************************************************************
   GAME OVER
 *************************************************************/
void gameOver() {
  endStateTime = millis() + endStateInterval; // start off the 1s timer
  gotoState(S_END); // go to the end state
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
  fallingInterval = 350L;
  initModel();
  gotoState(S_START);
}

/*
   In handleInput, we deal separately with how each state
   handles user input using a switch statement.
*/

void handleInput() {
  switch (state) {
    case S_START:
      if (AberLED.getButtonDown(FIRE)) {
        initModel();
        gotoState(S_GAME);
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
    case S_GAME:
      if (millis() >= fallingEvent) {
        fallingEvent = millis() + fallingInterval;
        playerY++;
      }
      if (playerY == 7) {
        initPlayer();
      }
      break;
    case S_END:
      if (millis() >= endStateTime) {
        endStateTime = millis() + endStateInterval;
        hasOneSecondElapsed = true; // can move to S_START
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