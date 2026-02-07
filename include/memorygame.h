#ifndef MEMORYGAME_H
#define MEMORYGAME_H

#include <Arduino.h>

#define MAX_SEQUENCE 20

// Estados del juego
enum MemoryGameState {
  MGS_IDLE,
  MGS_SHOWING_SEQUENCE,
  MGS_WAITING_INPUT,
  MGS_GAME_OVER
};

// Símbolos tipo morse
enum MorseSymbol {
  MORSE_DOT = 0,    // Pulsación corta (punto)
  MORSE_DASH = 1    // Pulsación larga (raya)
};

class MemoryGame {
private:
  int sequence[MAX_SEQUENCE];      // 0=punto, 1=raya
  int playerInput[MAX_SEQUENCE];   // Input del jugador
  int sequenceLength;
  int currentInputIndex;           // Índice actual del input del jugador
  int level;                       // Nivel actual (empieza en 0)
  int highScore;                   // Record de niveles alcanzados
  MemoryGameState state;
  
  unsigned long buttonPressStartTime;  // Para medir duración de pulsación
  bool buttonWasPressed;
  
public:
  MemoryGame();
  void initialize();
  void reset();
  void update();
  
  // Control del juego
  void startShowingSequence();
  void startWaitingInput();
  void registerButtonPress();      // Cuando se pulsa el botón
  void registerButtonRelease(unsigned long pressDuration);  // Cuando se suelta
  void checkInput();
  void nextLevel();
  void gameOver();
  
  // Getters
  MemoryGameState getState() const { return state; }
  int getLevel() const { return level; }
  int getHighScore() const { return highScore; }
  int getSequenceLength() const { return sequenceLength; }
  int getCurrentInputIndex() const { return currentInputIndex; }
  const int* getSequence() const { return sequence; }
  const int* getPlayerInput() const { return playerInput; }
  
private:
  void generateSequence();
};

#endif
