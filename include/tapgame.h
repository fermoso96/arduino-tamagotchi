#ifndef TAPGAME_H
#define TAPGAME_H

#include <Arduino.h>

class TapGame {
private:
  int score;
  int level;
  unsigned long targetTime;
  unsigned long spawnTime;
  bool targetActive;
  int difficulty; // milisegundos de espera
  int targetX;
  int targetY;
  unsigned long gameStartTime;
  
public:
  TapGame();
  void initialize();
  void reset();
  void update();
  
  void registerTap();
  bool isGameActive() const { return targetActive; }
  
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getTargetX() const { return targetX; }
  int getTargetY() const { return targetY; }
  int getDifficulty() const { return difficulty; }
  
private:
  void spawnTarget();
  void increaseLevel();
};

#endif
