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
  // remember to go to the initial state!
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
        gotoState(S_GAME);
      }
      break;
    case S_GAME:
      if (AberLED.getButtonDown(FIRE)) {
        gotoState(S_END);
      }
      break;
    case S_END:
      if (AberLED.getButtonDown(FIRE)) {
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
      AberLED.set(4, 4, YELLOW);
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
