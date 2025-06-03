#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_Sensor.h>

#define SHAKE_THRESHOLD 15.0

int gameState = 0;  // 0 = Ready, 1 = Play, 2 = Win, 3 = Lose
int playerScore = 0;
int mistakeCount = 0;

unsigned long lastActionTime = 0;
const unsigned long actionInterval = 3000;  // 3 seconds to respond

int currentPhase = -1;  // 0 = Attack, 1 = Defend, 2 = Dodge
bool actionDone = false;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
  CircuitPlayground.clearPixels();
  Serial.println("Dragon Battle Game Ready!");
}

void loop() {
  if (gameState == 0) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.playTone(500, 200);
    delay(500);
    CircuitPlayground.playTone(700, 200);
    delay(500);
    gameState = 1;
    nextPhase();
  }

  if (gameState == 1) {
    checkPlayerAction();

    if (millis() - lastActionTime > actionInterval && !actionDone) {
      mistakeCount++;
      CircuitPlayground.playTone(200, 300);  // fail tone
      CircuitPlayground.clearPixels();
      delay(300);
      Serial.println("Too slow!");
      checkGameOver();
      nextPhase();
    }
  }

  if (gameState == 2) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(4, 0, 255, 0);
    CircuitPlayground.setPixelColor(5, 0, 255, 0);
    CircuitPlayground.playTone(800, 500);
    Serial.println("You WIN!");
    delay(2000);
    resetGame();
  }

  if (gameState == 3) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(0, 255, 0, 0);
    CircuitPlayground.setPixelColor(9, 255, 0, 0);
    CircuitPlayground.playTone(150, 700);
    Serial.println("You LOSE!");
    delay(2000);
    resetGame();
  }
}

void nextPhase() {
  currentPhase = random(0, 3);
  actionDone = false;
  lastActionTime = millis();

  switch (currentPhase) {
    case 0: // Attack
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 255, 0, 0); // red
      }
      Serial.println("Dragon Phase: ATTACK! (Press B)");
      break;
    case 1: // Defend
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0, 0, 255); // blue
      }
      Serial.println("Dragon Phase: DEFEND! (Press A)");
      break;
    case 2: // Dodge
      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 255, 255, 0); // yellow
      }
      Serial.println("Dragon Phase: DODGE! (Shake)");
      break;
  }
}

void checkPlayerAction() {
  if (actionDone) return;

  // Attack
  if (currentPhase == 0 && CircuitPlayground.rightButton()) {
    playerSuccess();
  }

  // Defend
  if (currentPhase == 1 && CircuitPlayground.leftButton()) {
    playerSuccess();
  }

  // Dodge
  if (currentPhase == 2 && isShaken()) {
    playerSuccess();
  }
}

bool isShaken() {
  float x = 0, y = 0, z = 0;
  for (int i = 0; i < 10; i++) {
    x += CircuitPlayground.motionX();
    y += CircuitPlayground.motionY();
    z += CircuitPlayground.motionZ();
    delay(2);
  }
  x /= 10; y /= 10; z /= 10;

  float total = sqrt(x * x + y * y + z * z);
  return total > SHAKE_THRESHOLD;
}

void playerSuccess() {
  actionDone = true;
  CircuitPlayground.playTone(600, 300);  // success tone
  playerScore++;
  Serial.println("Success!");
  checkGameOver();
  nextPhase();
}

void checkGameOver() {
  if (playerScore >= 5) {
    gameState = 2;
  } else if (mistakeCount >= 2) {
    gameState = 3;
  }
}

void resetGame() {
  gameState = 0;
  playerScore = 0;
  mistakeCount = 0;
  currentPhase = -1;
  delay(1000);
}
