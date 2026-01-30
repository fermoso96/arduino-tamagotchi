#include "tapgame.h"

TapGame::TapGame() {
  score = 0;
  level = 1;
  targetActive = false;
  difficulty = 2000;
  targetX = 64;
  targetY = 32;
  gameStartTime = 0;
}

void TapGame::initialize() {
  reset();
}

void TapGame::reset() {
  score = 0;
  level = 1;
  targetActive = false;
  difficulty = 2000;
  gameStartTime = millis();
  spawnTarget();
}

void TapGame::update() {
  unsigned long currentTime = millis();
  
  if (!targetActive) {
    if (currentTime - spawnTime >= difficulty) {
      spawnTarget();
    }
  }
}

void TapGame::spawnTarget() {
  targetActive = true;
  spawnTime = millis();
  targetX = 30 + random(68);
  targetY = 20 + random(30);
}

void TapGame::registerTap() {
  if (targetActive) {
    score += 10 * level;
    targetActive = false;
    
    if (score > 0 && score % 100 == 0) {
      increaseLevel();
    }
  }
}

void TapGame::increaseLevel() {
  level++;
  difficulty = max(500, difficulty - 200);
}
