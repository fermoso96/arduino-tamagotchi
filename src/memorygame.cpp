#include "memorygame.h"

MemoryGame::MemoryGame() {
  sequenceLength = 0;
  currentInputIndex = 0;
  level = 0;  // Empieza en nivel 0
  highScore = 0;
  state = MGS_IDLE;
  buttonWasPressed = false;
  buttonPressStartTime = 0;
}

void MemoryGame::initialize() {
  highScore = 0;  // Se mantiene entre partidas
  reset();
}

void MemoryGame::reset() {
  level = 0;
  sequenceLength = 3;  // Empieza con 3 símbolos
  currentInputIndex = 0;
  state = MGS_IDLE;
  buttonWasPressed = false;
  buttonPressStartTime = 0;
  
  // Limpiar arrays
  for (int i = 0; i < MAX_SEQUENCE; i++) {
    sequence[i] = 0;
    playerInput[i] = 0;
  }
  
  generateSequence();
}

void MemoryGame::update() {
  // Por ahora, el update es manejado externamente desde main.cpp
}

void MemoryGame::generateSequence() {
  // Generar secuencia aleatoria de puntos y rayas
  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(0, 2);  // 0 = punto, 1 = raya
  }
  log_i("Generated sequence of length %d for level %d", sequenceLength, level);
}

void MemoryGame::startShowingSequence() {
  state = MGS_SHOWING_SEQUENCE;
  currentInputIndex = 0;
  
  // Limpiar input del jugador
  for (int i = 0; i < MAX_SEQUENCE; i++) {
    playerInput[i] = 0;
  }
  
  log_i("Showing sequence to player");
}

void MemoryGame::startWaitingInput() {
  state = MGS_WAITING_INPUT;
  currentInputIndex = 0;
  buttonWasPressed = false;
  log_i("Waiting for player input");
}

void MemoryGame::registerButtonPress() {
  if (state != MGS_WAITING_INPUT) return;
  if (buttonWasPressed) return;  // Ya está presionado
  
  buttonWasPressed = true;
  buttonPressStartTime = millis();
  log_i("Button pressed at %lu", buttonPressStartTime);
}

void MemoryGame::registerButtonRelease(unsigned long pressDuration) {
  if (state != MGS_WAITING_INPUT) return;
  if (!buttonWasPressed) return;
  
  buttonWasPressed = false;
  
  // Determinar si fue punto o raya
  // Punto: < 400ms, Raya: >= 400ms
  int symbol = (pressDuration < 400) ? MORSE_DOT : MORSE_DASH;
  playerInput[currentInputIndex] = symbol;
  
  log_i("Button released after %lu ms - Symbol: %s", 
        pressDuration, (symbol == MORSE_DOT) ? "DOT" : "DASH");
  
  // Verificar inmediatamente si el símbolo es correcto
  if (playerInput[currentInputIndex] != sequence[currentInputIndex]) {
    log_i("Wrong input! Expected %d, got %d", 
          sequence[currentInputIndex], playerInput[currentInputIndex]);
    gameOver();
    return;
  }
  
  currentInputIndex++;
  
  // Verificar si completó la secuencia
  if (currentInputIndex >= sequenceLength) {
    log_i("Sequence completed correctly!");
    nextLevel();
  }
}

void MemoryGame::checkInput() {
  // Esta función ya no es necesaria porque verificamos en registerButtonRelease
}

void MemoryGame::nextLevel() {
  level++;
  sequenceLength++;  // Aumenta 1 símbolo por nivel
  
  if (sequenceLength > MAX_SEQUENCE) {
    sequenceLength = MAX_SEQUENCE;
  }
  
  // Actualizar high score
  if (level > highScore) {
    highScore = level;
  }
  
  currentInputIndex = 0;
  generateSequence();
  
  log_i("Advanced to level %d, sequence length: %d", level, sequenceLength);
}

void MemoryGame::gameOver() {
  state = MGS_GAME_OVER;
  
  // Actualizar high score si es necesario
  if (level > highScore) {
    highScore = level;
  }
  
  log_i("Game Over! Final level: %d, High score: %d", level, highScore);
}
