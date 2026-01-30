#include "memorygame.h"

MemoryGame::MemoryGame() {
  sequenceLength = 0;
  currentIndex = 0;
  score = 0;
  level = 1;
  waitingForInput = false;
  lastInput = -1;
  inputTimeout = 3000;
  lastInputTime = 0;
}

void MemoryGame::initialize() {
  reset();
}

void MemoryGame::reset() {
  sequenceLength = 0;
  currentIndex = 0;
  score = 0;
  level = 1;
  lastInput = -1;
  waitingForInput = false;
  
  addToSequence();
  playSequence();
}

void MemoryGame::update() {
  unsigned long currentTime = millis();
  
  if (waitingForInput) {
    if (currentTime - lastInputTime >= inputTimeout) {
      checkGameOver();
    }
  }
}

void MemoryGame::addToSequence() {
  if (sequenceLength < MAX_SEQUENCE) {
    sequence[sequenceLength] = random(0, 3); // 3 botones disponibles: 0=IZQ, 1=CENTRO, 2=DER
    sequenceLength++;
  }
}

void MemoryGame::registerInput(int buttonIndex) {
  if (!waitingForInput) return;
  
  lastInput = buttonIndex;
  
  // Verificar si es correcto
  if (buttonIndex == sequence[currentIndex]) {
    score += 10;
    currentIndex++;
    
    if (currentIndex >= sequenceLength) {
      // Secuencia completada
      currentIndex = 0;
      addToSequence();
      increaseLevel();
      lastInputTime = millis();
      delay(1000); // Esperar antes de siguiente ronda
      playSequence();
    }
  } else {
    checkGameOver();
  }
}

void MemoryGame::playSequence() {
  // En un juego real, aquí sonaría cada botón en la secuencia
  currentIndex = 0;
  waitingForInput = true;
  lastInputTime = millis();
}

void MemoryGame::increaseLevel() {
  level++;
  inputTimeout = (unsigned long)max((int)inputTimeout - 200, 1000);
}

void MemoryGame::checkGameOver() {
  // Juego terminado
  waitingForInput = false;
}
