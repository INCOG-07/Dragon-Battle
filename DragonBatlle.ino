#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_Sensor.h>

// Define the motion threshold for a shake to count as a "dodge" action
#define SHAKE_THRESHOLD 15.0

// Game state: 0 = Ready, 1 = Play, 2 = Win, 3 = Lose
int gameState = 0;

// Player stats
int playerScore = 0;
int mistakeCount = 0;

// Timer for player reaction window
unsigned long lastActionTime = 0;
const unsigned long actionInterval = 3000;  // 3 seconds allowed per phase

// Current phase: -1 = none, 0 = Attack, 1 = Defend, 2 = Dodge
int currentPhase = -1;

// Tracks whether the correct player input was received in the current phase
bool actionDone = false;

// Tracks the previous state of the physical slide switch
bool lastSwitchState = true;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);  // set accelerometer sensitivity
  CircuitPlayground.clearPixels();  // clear all LEDs
  Serial.println("Dragon Battle Game Ready!");
}

void loop() {
  bool currentSwitchState = CircuitPlayground.slideSwitch();

  // If switch is OFF (left), clear LEDs and end the game
  if (!currentSwitchState) {
    CircuitPlayground.clearPixels();
    lastSwitchState = false;
    return;
  }

  // If switch was OFF and turned ON (right), reset the game
  if (!lastSwitchState && currentSwitchState) {
    resetGame();
    Serial.println("Switch flipped ON = Game reset.");
  }

  lastSwitchState = currentSwitchState;  // update switch state tracker

  // Game is in READY state
  if (gameState == 0) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.playTone(500, 200);  // startup tones
    delay(500);
    CircuitPlayground.playTone(700, 200);
    delay(500);
    gameState = 1;       // switch to PLAY state
    nextPhase();         // begin first phase
  }

  // Game is in PLAY state
  if (gameState == 1) {
    checkPlayerAction();  // check if player performed correct input

    // Player took too long to respond
    if (millis() - lastActionTime > actionInterval && !actionDone) {
      mistakeCount++;
      CircuitPlayground.playTone(200, 300);  // failure tone
      CircuitPlayground.clearPixels();
      delay(300);
      Serial.println("Too slow!");
      checkGameOver();  // check if game is over
      nextPhase();      // move to next phase
    }
  }

  // Game is in WIN state
  if (gameState == 2) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(4, 0, 255, 0);  // green LEDs in middle
    CircuitPlayground.setPixelColor(5, 0, 255, 0);
    CircuitPlayground.playTone(800, 500);  // victory tone
    Serial.println("You WIN!");
    delay(2000);
    resetGame();  // reset game for replay
  }

  // Game is in LOSE state
  if (gameState == 3) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(0, 255, 0, 0);  // red LEDs on ends
    CircuitPlayground.setPixelColor(9, 255, 0, 0);
    CircuitPlayground.playTone(150, 700);  // loss tone
    Serial.println("You LOSE!");
    delay(2000);
    resetGame();
  }
}

// Move to a randomized dragon phase (attack, defend, dodge)
void nextPhase() {
  currentPhase = random(0, 3);      // random number from 0 to 2
  actionDone = false;              // reset action status
  lastActionTime = millis();       // record phase start time

  switch (currentPhase) {
    case 0: // Attack Phase (Red LEDs)
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 255, 0, 0);
      }
      Serial.println("Dragon Phase: ATTACK! (Press B)");
      break;

    case 1: // Defend Phase (Blue LEDs)
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0, 0, 255);
      }
      Serial.println("Dragon Phase: DEFEND! (Press A)");
      break;

    case 2: // Dodge Phase (Yellow LEDs)
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 255, 255, 0);
      }
      Serial.println("Dragon Phase: DODGE! (Shake)");
      break;
  }
}

// Check if player input matches current dragon phase
void checkPlayerAction() {
  if (actionDone) return;

  // Correct input for Attack phase
  if (currentPhase == 0 && CircuitPlayground.rightButton()) {
    playerSuccess();
  }

  // Correct input for Defend phase
  if (currentPhase == 1 && CircuitPlayground.leftButton()) {
    playerSuccess();
  }

  // Correct input for Dodge phase
  if (currentPhase == 2 && isShaken()) {
    playerSuccess();
  }
}

// Determine if the player shook the CPX hard enough
bool isShaken() {
  float x = 0, y = 0, z = 0;

  // Average 10 readings for motion smoothing
  for (int i = 0; i < 10; i++) {
    x += CircuitPlayground.motionX();
    y += CircuitPlayground.motionY();
    z += CircuitPlayground.motionZ();
    delay(2);
  }

  x /= 10; y /= 10; z /= 10;
  float total = sqrt(x * x + y * y + z * z);

  return total > SHAKE_THRESHOLD;  // return true if shake is strong enough
}

// Handle correct player input
void playerSuccess() {
  actionDone = true;
  CircuitPlayground.playTone(600, 300);  // success tone
  playerScore++;
  Serial.println("Success!");
  checkGameOver();  // see if player has won
  nextPhase();      // go to next phase
}

// Check if player won or lost
void checkGameOver() {
  if (playerScore >= 5) {
    gameState = 2;  // WIN
  } else if (mistakeCount >= 2) {
    gameState = 3;  // LOSE
  }
}

// Reset all game variables
void resetGame() {
  gameState = 0;
  playerScore = 0;
  mistakeCount = 0;
  currentPhase = -1;
  delay(1000);
}
