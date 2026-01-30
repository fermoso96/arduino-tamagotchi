#ifndef MEMORYGAME_H
#define MEMORYGAME_H

#include <Arduino.h>

#define MAX_SEQUENCE 20

class MemoryGame {
private:
  int sequence[MAX_SEQUENCE];
  int sequenceLength;
  int currentIndex;
  int score;
  int level;
  
  unsigned long lastInputTime;
  unsigned long inputTimeout;
  bool waitingForInput;
  int lastInput;
  
public:
  MemoryGame();
  void initialize();
  void reset();
  void update();
  
  void registerInput(int buttonIndex);
  void playSequence();
  
  bool isWaitingForInput() const { return waitingForInput; }
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getCurrentIndex() const { return currentIndex; }
  int getSequenceLength() const { return sequenceLength; }
  const int* getSequence() const { return sequence; }
  int getLastInput() const { return lastInput; }
  
private:
  void addToSequence();
  void increaseLevel();
  void checkGameOver();
};

#endif
