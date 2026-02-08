#ifndef TAMAGOTCHI_H
#define TAMAGOTCHI_H

#include <Arduino.h>
#include <Preferences.h>

class Tamagotchi {
private:
  // Estadísticas (0-100%): hambre, aburrimiento y sueño
  // Hambre: 0=hambriento, 100=lleno
  // Aburrimiento: 0=muy aburrido, 100=bien
  // Sueño: 0=mucho sueño, 100=bien
  int hunger;      // 0-100%
  int boredom;     // 0-100%
  int sleepiness;  // 0-100%
  int coins;
  bool memoryGameUnlocked; // Indica si el juego de memoria está desbloqueado
  bool ticTacToeUnlocked;  // Indica si el juego de tres en raya está desbloqueado
  
  unsigned long lastMinuteUpdate;
  unsigned long sleepStartTime;
  
  Preferences prefs;
  unsigned long lastSleepTick; // Para controlar incremento de sueño cada 5 segundos
  
  // Variables para rastrear cuándo se activan los sonidos
  bool wasHungry;   // Si el hambre ya estaba <= 20
  bool wasBored;    // Si el aburrimiento ya estaba <= 20
  bool wasSleepy;   // Si el sueño ya estaba <= 20
  
public:
  bool isSleeping;
  bool showAngryFace;  // Para feedback visual
  unsigned long angryFaceTimer;
  bool showHappyFace;  // Para feedback visual de acciones exitosas
  unsigned long happyFaceTimer;
  bool showInsufficientCoins;  // Mensaje de monedas insuficientes
  unsigned long insufficientCoinsTimer;
  bool playHungrySound;    // Señal para reproducir sonido de hambre
  bool playBoredSound;     // Señal para reproducir sonido de aburrimiento
  bool playSleepySound;    // Señal para reproducir sonido de sueño
  
  Tamagotchi();
  void initialize();
  void update();
  
  // Acciones (retornan true si se ejecutaron)
  bool feed();
  bool play();
  bool sleep();
  void wakeUp();
  void addCoins(int amount);
  void addBoredom(int amount);
  // Tienda
  bool buyFood(int type); // 0: manzana, 1: pan, 2: queso, 3: tarta
  bool buyMemoryGame();
  bool buyTicTacToeGame();
  bool getMemoryGameUnlocked() const { return memoryGameUnlocked; }
  bool getTicTacToeUnlocked() const { return ticTacToeUnlocked; }
  
  // Getters
  int getHunger() const { return hunger; }
  int getBoredom() const { return boredom; }
  int getSleepiness() const { return sleepiness; }
  int getCoins() const { return coins; }
  bool getIsSleeping() const { return isSleeping; }
  
  // Setters (para modo TEST)
  void setHunger(int h) { hunger = constrain(h, 0, 100); }
  void setSleepiness(int s) { sleepiness = constrain(s, 0, 100); }
  void setBoredom(int b) { boredom = constrain(b, 0, 100); }
  void setCoins(int c) { coins = max(0, c); }
  
  // Determinar mood para ojos
  int getMood() const;
  
private:
  void updatePerMinute();
  void saveStats();
  void loadStats();
};

#endif
