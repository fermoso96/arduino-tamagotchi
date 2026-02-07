#include "game.h"
#include <Preferences.h>

DodgeGame::DodgeGame() {
  playerLane = 1;
  score = 0;
  level = 1;
  record = 0;
  obstacleSpeed = 2;
  lastObstacleTime = 0;
  lastUpdateTime = 0;
  obstacleCount = 0;
  boxesDodgedThisLevel = 0;
  maxActiveObstacles = 1;
  
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = false;
  }
}

void DodgeGame::initialize() {
  loadRecord();
  reset();
}

void DodgeGame::loadRecord() {
  Preferences prefs;
  prefs.begin("tamagotchi", true); // Solo lectura
  record = prefs.getInt("gameRecord", 0);
  prefs.end();
}

void DodgeGame::saveRecord() {
  if (level > record) {
    record = level;
    Preferences prefs;
    prefs.begin("tamagotchi", false);
    prefs.putInt("gameRecord", record);
    prefs.end();
  }
}

void DodgeGame::reset() {
  playerLane = 1;
  score = 0;
  level = 1;
  obstacleSpeed = 2;
  lastObstacleTime = millis();
  lastUpdateTime = millis();
  obstacleCount = 0;
  boxesDodgedThisLevel = 0;
  maxActiveObstacles = 1; // Empezar con 1 caja
  
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = false;
  }
}

void DodgeGame::update() {
  unsigned long currentTime = millis();
  
  // Spawnear obstáculos
  if (currentTime - lastObstacleTime >= max(1000 - level * 50, 200)) { // Aumenta frecuencia
    spawnObstacle();
    lastObstacleTime = currentTime;
  }
  
  // Actualizar obstáculos
  updateObstacles();
  
  // Verificar si pasamos de nivel basado en cajas esquivadas
  if (boxesDodgedThisLevel >= getBoxesRequiredForLevel(level)) {
    increaseLevel();
  }
}

void DodgeGame::moveLeft() {
  if (playerLane > 0) {
    playerLane--;
  }
}

void DodgeGame::moveRight() {
  if (playerLane < NUM_LANES - 1) {
    playerLane++;
  }
}

void DodgeGame::spawnObstacle() {
  // Solo spawnear si no hemos alcanzado el máximo de cajas activas para este nivel
  if (obstacleCount >= maxActiveObstacles) return;
  
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) {
      obstacles[i].x = GAME_WIDTH;
      obstacles[i].lane = random(0, NUM_LANES);
      obstacles[i].active = true;
      obstacleCount++;
      break;
    }
  }
}

void DodgeGame::updateObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      obstacles[i].x -= obstacleSpeed;
      
      // Puntos por esquivar (cuando pasa X=10) y contar cajas
      if (obstacles[i].x <= 10 && obstacles[i].x > 10 - obstacleSpeed) {
        score += 10 * level;
        boxesDodgedThisLevel++;
      }
      
      // Eliminar si salió de pantalla
      if (obstacles[i].x < -10) {
        obstacles[i].active = false;
        obstacleCount--;
      }
    }
  }
}

bool DodgeGame::checkCollision() {
  // Posición del jugador en pantalla
  const int PLAYER_X = 10;
  const int PLAYER_WIDTH = 8;
  
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      // Verificar si el obstáculo está en el carril del jugador
      if (obstacles[i].lane == playerLane) {
        // Comprobar colisión por rango X
        // El obstáculo ocupa desde obs[i].x hasta obs[i].x + 8
        // El jugador ocupa desde PLAYER_X (10) hasta PLAYER_X + PLAYER_WIDTH (18)
        // Hay colisión si se superponen
        if (obstacles[i].x < (PLAYER_X + PLAYER_WIDTH) && 
            (obstacles[i].x + 8) > PLAYER_X) {
          return true;
        }
      }
    }
  }
  return false;
}

void DodgeGame::increaseLevel() {
  level++;
  boxesDodgedThisLevel = 0;  // Resetear contador de cajas
  // Aumentar velocidad de forma muy gradual: +0.3 cada nivel
  obstacleSpeed = min(5.5, obstacleSpeed + 0.3);
  // Aumentar número de cajas activas (máximo MAX_OBSTACLES)
  maxActiveObstacles = min(MAX_OBSTACLES, level);
}

int DodgeGame::getBoxesRequiredForLevel(int level) const {
  // Fórmula: (level² + level + 8) / 2
  // Nivel 1→2: 5 cajas
  // Nivel 2→3: 7 cajas
  // Nivel 3→4: 10 cajas
  // Nivel 4→5: 14 cajas
  return (level * level + level + 8) / 2;
}
