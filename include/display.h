#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "tamagotchi.h"
#include "game.h"
#include "memorygame.h"
#include "tictactoe.h"
#include "eyes.h"

class DisplayManager {
private:
  Adafruit_SSD1306* display;
  Tamagotchi* pet;
  EyesManager eyesManager;
  int currentMood;
  
public:
  DisplayManager();
  void initialize(Adafruit_SSD1306* disp, Tamagotchi* p);
  
  void showMainScreen();
  void showSleepScreen();
  void showInsufficientCoinsScreen();
  void showMenuScreen(int menuOption, bool soundEnabled);
  void showShopMenuScreen(int shopMenuOption);
  void showGameMenuScreen(int gameMenuOption);
  void showGameScreen(DodgeGame* game);
  void showGameOver(DodgeGame* game, int coinsEarned);
  void showMemoryGameScreen(MemoryGame* memGame);
  void showMemoryGameOver(MemoryGame* memGame, int coinsEarned);
  void showTicTacToeScreen(TicTacToeGame* ticTacToe);
  void showTicTacToeGameOver(TicTacToeGame* ticTacToe, int coinsEarned);
  
  // Control de ojos para el juego de memoria
  void showEyesBlink();    // Mostrar parpadeo
  void showEyesLookUp();   // Mostrar mirando arriba
  void showEyesNormal();   // Volver a normal
  
private:
  void drawEyesAnimated();
  void drawStatusBar();
  void drawMenu(int selectedOption);
  void drawGameMenu(int selectedOption);
  void drawCoins();
  void drawStatIndicators();
  void drawLanes(DodgeGame* game);
  void drawPlayer(DodgeGame* game);
  void drawObstacles(DodgeGame* game);
  void drawHungerIcon();
  void drawSleepIcon();
};

#endif
