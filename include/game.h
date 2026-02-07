#ifndef GAME_H
#define GAME_H

#include <Arduino.h>

#define GAME_WIDTH 128
#define GAME_HEIGHT 64
#define LANE_HEIGHT 21
#define NUM_LANES 3
#define MAX_OBSTACLES 5

struct Obstacle {
  float x;
  int lane;
  bool active;
};

class DodgeGame {
private:
  int playerLane;
  int score;
  int level;
  int record; // Récord de nivel más alto alcanzado
  float obstacleSpeed;
  unsigned long lastObstacleTime;
  unsigned long lastUpdateTime;
  int boxesDodgedThisLevel;
  
  Obstacle obstacles[MAX_OBSTACLES];
  int obstacleCount;
  int maxActiveObstacles; // Número máximo de cajas activas según nivel
  
public:
  DodgeGame();
  void initialize();
  void reset();
  void update();
  void loadRecord();
  void saveRecord();
  
  void toggleLane();  // Alternar entre carril central (1) e inferior (2)
  
  bool checkCollision();
  
  // Getters
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getRecord() const { return record; }
  int getPlayerLane() const { return playerLane; }
  float getObstacleSpeed() const { return obstacleSpeed; }
  
  const Obstacle* getObstacles() const { return obstacles; }
  int getObstacleCount() const { return obstacleCount; }
  
private:
  void spawnObstacle();
  void updateObstacles();
  void increaseLevel();
  int getBoxesRequiredForLevel(int level) const;
};

#endif
