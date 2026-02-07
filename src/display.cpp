#include "display.h"

void DisplayManager::showShopMenuScreen(int shopMenuOption, bool memGameUnlocked) {
  display->clearDisplay();
  // Mostrar monedas arriba
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->print("Monedas: ");
  display->print(pet->getCoins());
  // Línea divisoria
  display->drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Artículos de la tienda
  const char* labels[5] = {"Manzana (10c/+25H)", "Pan (15c/+50H)", "Queso (20c/+75H)", "Tarta (25c/+100H)", "Juego Memoria (50c)"};
  int totalItems = memGameUnlocked ? 4 : 5;
  int y = 18;
  int itemHeight = 10;
  for (int i = 0; i < totalItems; i++) {
    if (i == shopMenuOption) {
      display->fillRect(0, y + (i * itemHeight), 128, itemHeight, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }
    display->setCursor(5, y + (i * itemHeight) + 1);
    display->setTextSize(1);
    display->print(labels[i]);
  }
  display->display();
}
#include "display.h"

DisplayManager::DisplayManager() {
  display = nullptr;
  pet = nullptr;
  currentMood = 0;
}

void DisplayManager::initialize(Adafruit_SSD1306* disp, Tamagotchi* p) {
  display = disp;
  pet = p;
  eyesManager.initialize(disp);
  currentMood = 0;
}

void DisplayManager::showMainScreen() {
  // Sincronizar mood con el estado del pet
  int newMood = pet->getMood();
  if (newMood != currentMood) {
    currentMood = newMood;
    eyesManager.setMood(currentMood);
  }
  
  // Solo dibujar los ojos - sin overlays
  drawEyesAnimated();
  display->display();
}

void DisplayManager::showSleepScreen() {
  display->clearDisplay();
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  
  // Calcular posición centrada para "ZZZ..."
  const char* text = "ZZZ...";
  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  int x = (128 - w) / 2;
  int y = (64 - h) / 2;
  
  display->setCursor(x, y);
  display->print(text);
  display->display();
}

void DisplayManager::showInsufficientCoinsScreen() {
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  // Calcular posición centrada para "Monedas insuficientes"
  const char* text = "Monedas";
  const char* text2 = "insuficientes";
  int16_t x1, y1;
  uint16_t w, h;
  
  // Línea 1: "Monedas"
  display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (128 - w) / 2;
  int y = 24;
  display->setCursor(x, y);
  display->print(text);
  
  // Línea 2: "insuficientes"
  display->getTextBounds(text2, 0, 0, &x1, &y1, &w, &h);
  x = (128 - w) / 2;
  y = 36;
  display->setCursor(x, y);
  display->print(text2);
  
  display->display();
}

void DisplayManager::drawMenu(int selectedOption) {
  // Dibujar barra de menú inferior
  int y = 54;
  int w = 128 / 3;
  
  // Opciones: TIENDA | JUGAR | DORMIR
  const char* labels[] = {"TIENDA", "JUGAR", "DORMIR"};
  
  for(int i=0; i<3; i++) {
    int x = i * w;
    if(i == selectedOption) {
      display->fillRect(x, y, w, 10, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->drawRect(x, y, w, 10, SSD1306_WHITE);
      display->setTextColor(SSD1306_WHITE);
    }
    
    display->setCursor(x + 2, y + 1);
    display->setTextSize(1);
    display->print(labels[i]);
  }
}

void DisplayManager::showMenuScreen(int selectedOption) {
  display->clearDisplay();
  
  // Mostrar estadísticas arriba
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->print("H:");
  display->print(pet->getHunger());
  display->print("%");
  
  display->setCursor(42, 0);
  display->print("B:");
  display->print(pet->getBoredom());
  display->print("%");
  
  display->setCursor(84, 0);
  display->print("S:");
  display->print(pet->getSleepiness());
  display->print("%");
  
  // Línea divisoria
  display->drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  // Dibujar el menú con las 3 opciones
  int y = 20;
  int itemHeight = 15;
  
  // Opción TIENDA fija
  const char* labels[] = {"TIENDA", "JUGAR", "DORMIR"};
  
  for(int i = 0; i < 3; i++) {
    if(i == selectedOption) {
      display->fillRect(0, y + (i * itemHeight), 128, itemHeight, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }
    
    display->setCursor(10, y + (i * itemHeight) + 3);
    display->setTextSize(1);
    display->print(labels[i]);
  }
  
  display->display();
}

void DisplayManager::showGameScreen(DodgeGame* game) {
  display->clearDisplay();
  
  drawLanes(game);
  drawPlayer(game);
  drawObstacles(game);
  
  // Nivel en la izquierda
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->print("Lvl: ");
  display->print(game->getLevel());
  
  // Récord en la derecha
  display->setCursor(80, 0);
  display->print("RCRD:");
  display->print(game->getRecord());
  
  display->display();
}

void DisplayManager::showGameOver(DodgeGame* game, int coinsEarned) {
  display->clearDisplay();
  
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(20, 10);
  display->println("GAME OVER");
  
  display->setTextSize(1);
  display->setCursor(10, 45);
  display->print("Coins: +");
  display->println(coinsEarned);
  
  display->display();
}

void DisplayManager::drawEyesAnimated() {
  // Los ojos se dibujan a través de eyesManager
  eyesManager.drawEyesAnimated();
}

void DisplayManager::drawHungerIcon() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(115, 10);
  display->print("*");
  display->setCursor(120, 10);
  display->print("*");
}

void DisplayManager::drawSleepIcon() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(115, 15);
  display->print("Z");
  display->setCursor(120, 15);
  display->print("Z");
}

void DisplayManager::drawCoins() {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(100, 32);
  display->print("$");
  display->setCursor(100, 42);
  display->print(pet->getCoins());
}

void DisplayManager::drawLanes(DodgeGame* game) {
  for (int i = 1; i < 3; i++) {
    display->drawLine(0, i * LANE_HEIGHT, GAME_WIDTH, i * LANE_HEIGHT, SSD1306_WHITE);
  }
}

void DisplayManager::drawPlayer(DodgeGame* game) {
  int lane = game->getPlayerLane();
  int playerY = lane * LANE_HEIGHT + LANE_HEIGHT / 2;
  // Dibujar jugador como rectángulo con líneas
  int x = 10;  // Cambiar a la izquierda de la pantalla
  display->drawLine(x, playerY - 3, x + 8, playerY - 3, SSD1306_WHITE);
  display->drawLine(x + 8, playerY - 3, x + 8, playerY + 3, SSD1306_WHITE);
  display->drawLine(x + 8, playerY + 3, x, playerY + 3, SSD1306_WHITE);
  display->drawLine(x, playerY + 3, x, playerY - 3, SSD1306_WHITE);
}

void DisplayManager::drawObstacles(DodgeGame* game) {
  // Dibujar todos los obstáculos del juego
  const Obstacle* obs = game->getObstacles();
  
  // Iterar sobre todos los obstáculos posibles (no solo el count)
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obs[i].active) {
      int obstacleY = obs[i].lane * LANE_HEIGHT + LANE_HEIGHT / 2;
      
      // Dibujar obstáculo como cuadrado relleno
      display->fillRect(obs[i].x, obstacleY - 4, 8, 8, SSD1306_WHITE);
    }
  }
}

void DisplayManager::drawGameMenu(int selectedOption) {
  // Dibujar menú de selección de juegos (2 opciones)
  int y = 20;
  int itemHeight = 20;
  
  const char* labels[] = {"ESQUIVAR", "MEMORIA"};
  
  for(int i = 0; i < 2; i++) {
    if(i == selectedOption) {
      display->fillRect(0, y + (i * itemHeight), 128, itemHeight, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }
    
    display->setCursor(20, y + (i * itemHeight) + 5);
    display->setTextSize(1);
    display->print(labels[i]);
  }
}

void DisplayManager::showGameMenuScreen(int selectedOption) {
  display->clearDisplay();
  // Título
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(25, 5);
  display->print("ELIGE UN JUEGO");
  // Línea divisoria
  display->drawLine(0, 15, 127, 15, SSD1306_WHITE);
  // Opciones
  int y = 25;
  int itemHeight = 15;
  int idx = 0;
  // Siempre mostrar ESQUIVAR
  if (selectedOption == idx) {
    display->fillRect(0, y + (idx * itemHeight), 128, itemHeight, SSD1306_WHITE);
    display->setTextColor(SSD1306_BLACK);
  } else {
    display->setTextColor(SSD1306_WHITE);
  }
  display->setCursor(20, y + (idx * itemHeight) + 3);
  display->setTextSize(1);
  display->print("ESQUIVAR");
  idx++;
  // Solo mostrar MEMORIA si está desbloqueado
  if (pet->getMemoryGameUnlocked()) {
    if (selectedOption == idx) {
      display->fillRect(0, y + (idx * itemHeight), 128, itemHeight, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }
    display->setCursor(20, y + (idx * itemHeight) + 3);
    display->setTextSize(1);
    display->print("MEMORIA");
  }
  display->display();
}

void DisplayManager::showMemoryGameScreen(MemoryGame* memGame) {
  display->clearDisplay();
  
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->print("Nivel: ");
  display->print(memGame->getLevel());
  display->setCursor(70, 0);
  display->print("Score: ");
  display->print(memGame->getScore());
  
  // Línea divisoria
  display->drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  // Mostrar secuencia si no está esperando input
  if (!memGame->isWaitingForInput()) {
    display->setCursor(20, 25);
    display->setTextSize(1);
    display->print("Memoriza...");
  } else {
    // Mostrar botones para el jugador
    display->setCursor(15, 25);
    display->setTextSize(1);
    display->print("Repite:");
    
    // Mostrar progreso
    display->setCursor(10, 40);
    display->print(memGame->getCurrentIndex());
    display->print("/");
    display->print(memGame->getSequenceLength());
    
    // Indicadores de botones
    int y = 52;
    display->drawRect(5, y, 30, 10, SSD1306_WHITE);
    display->setCursor(10, y+2);
    display->print("IZQ");
    
    display->drawRect(45, y, 35, 10, SSD1306_WHITE);
    display->setCursor(50, y+2);
    display->print("CENT");
    
    display->drawRect(90, y, 30, 10, SSD1306_WHITE);
    display->setCursor(95, y+2);
    display->print("DER");
    
    // Resaltar último input
    int lastInput = memGame->getLastInput();
    if (lastInput >= 0) {
      int xPos = (lastInput == 0) ? 5 : ((lastInput == 1) ? 45 : 90);
      int width = (lastInput == 1) ? 35 : 30;
      display->fillRect(xPos, y, width, 10, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
      display->setCursor((lastInput == 0) ? 10 : ((lastInput == 1) ? 50 : 95), y+2);
      display->print((lastInput == 0) ? "IZQ" : ((lastInput == 1) ? "CENT" : "DER"));
    }
  }
  
  display->display();
}

void DisplayManager::showMemoryGameOver(MemoryGame* memGame, int coinsEarned) {
  display->clearDisplay();
  
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(20, 10);
  display->println("GAME OVER");
  
  display->setTextSize(1);
  display->setCursor(10, 35);
  display->print("Nivel: ");
  display->println(memGame->getLevel());
  
  display->setCursor(10, 50);
  display->print("Coins: +");
  display->println(coinsEarned);
  
  display->display();
}
