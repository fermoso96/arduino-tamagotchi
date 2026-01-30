#ifndef TAMAGOTCHI_H
#define TAMAGOTCHI_H

#include <Arduino.h>
#include <Preferences.h>

class Tamagotchi {
  void setHunger(int h) { hunger = constrain(h, 0, 100); }
  void setSleepiness(int s) { sleepiness = constrain(s, 0, 100); }
  void setBoredom(int b) { boredom = constrain(b, 0, 100); }
private:
  // Estadísticas (0-100%): hambre, aburrimiento y sueño
  // Hambre: 0=hambriento, 100=lleno
  // Aburrimiento: 0=muy aburrido, 100=bien
  // Sueño: 0=mucho sueño, 100=bien
  int hunger;      // 0-100%
  int boredom;     // 0-100%
  int sleepiness;  // 0-100%
  int coins;
  bool memoryGameUnlocked; // Nuevo: indica si el juego de memoria está desbloqueado
  
  unsigned long lastMinuteUpdate;
  unsigned long sleepStartTime;
  unsigned long sleepDuration; // 15 minutos en ms
  
  Preferences prefs;
  
public:
  bool isSleeping;
  bool showAngryFace;  // Para feedback visual
  unsigned long angryFaceTimer;
  bool showHappyFace;  // Para feedback visual de acciones exitosas
  unsigned long happyFaceTimer;
  bool showInsufficientCoins;  // Mensaje de monedas insuficientes
  unsigned long insufficientCoinsTimer;
  
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
  bool getMemoryGameUnlocked() const { return memoryGameUnlocked; }
  
  // Getters
  int getHunger() const { return hunger; }
  int getBoredom() const { return boredom; }
  int getSleepiness() const { return sleepiness; }
  int getCoins() const { return coins; }
  bool getIsSleeping() const { return isSleeping; }
  
  // Determinar mood para ojos
  int getMood() const;
  
private:
  void updatePerMinute();
  void saveStats();
  void loadStats();
};

#endif
