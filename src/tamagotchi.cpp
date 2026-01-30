#include "tamagotchi.h"

Tamagotchi::Tamagotchi() {
  hunger = 100;       // Empieza lleno
  boredom = 100;      // Empieza bien
  sleepiness = 100;   // Empieza bien
  coins = 0;
  memoryGameUnlocked = false;
  isSleeping = false;
  showAngryFace = false;
  angryFaceTimer = 0;
  showHappyFace = false;
  happyFaceTimer = 0;
  showInsufficientCoins = false;
  insufficientCoinsTimer = 0;
  
  lastMinuteUpdate = 0;
  sleepStartTime = 0;
  sleepDuration = 10 * 60 * 1000; // 10 minutos
}

void Tamagotchi::initialize() {
  prefs.begin("tamagotchi", false);
  loadStats();
  // Leer si el juego de memoria está desbloqueado
  memoryGameUnlocked = prefs.getBool("memgame", false);
  
  lastMinuteUpdate = millis();
  
  if (isSleeping) {
    // Si estaba durmiendo al reiniciar, despertar
    isSleeping = false;
    saveStats();
  }
}
// Lógica de compra de comida
bool Tamagotchi::buyFood(int type) {
  int cost = 0;
  int restore = 0;
  switch(type) {
    case 0: cost = 10; restore = 25; break; // Manzana
    case 1: cost = 15; restore = 50; break; // Pan
    case 2: cost = 20; restore = 75; break; // Queso
    case 3: cost = 25; restore = 100; break; // Tarta
    default: return false;
  }
  if (coins < cost) {
    showInsufficientCoins = true;
    insufficientCoinsTimer = millis();
    return false;
  }
  coins -= cost;
  hunger = min(100, hunger + restore);
  showHappyFace = true;
  happyFaceTimer = millis();
  saveStats();
  return true;
}

// Lógica de compra del juego de memoria
bool Tamagotchi::buyMemoryGame() {
  if (memoryGameUnlocked) return false;
  if (coins < 50) {
    showInsufficientCoins = true;
    insufficientCoinsTimer = millis();
    return false;
  }
  coins -= 50;
  memoryGameUnlocked = true;
  prefs.putBool("memgame", true);
  showHappyFace = true;
  happyFaceTimer = millis();
  saveStats();
  return true;
}

void Tamagotchi::update() {
  unsigned long currentTime = millis();
  
  // Gestionar cara enfadada temporal (3 segundos)
  if (showAngryFace && (currentTime - angryFaceTimer >= 3000)) {
    showAngryFace = false;
  }
  
  // Gestionar cara feliz temporal (3 segundos)
  if (showHappyFace && (currentTime - happyFaceTimer >= 3000)) {
    showHappyFace = false;
  }
  
  // Gestionar mensaje de monedas insuficientes (3 segundos)
  if (showInsufficientCoins && (currentTime - insufficientCoinsTimer >= 3000)) {
    showInsufficientCoins = false;
  }
  
  // Si está durmiendo
  if (isSleeping) {
    if (currentTime - sleepStartTime >= sleepDuration) {
      wakeUp();
    }
    return; // No actualizar otros parámetros mientras duerme
  }
  
  // Actualizar cada minuto
  updatePerMinute();
}

void Tamagotchi::updatePerMinute() {
  unsigned long currentTime = millis();
  if (currentTime - lastMinuteUpdate >= 60000) { // 60 segundos
    // Hambre pierde 4% por minuto
    hunger = max(0, hunger - 4);
    
    // Aburrimiento pierde 2% por minuto
    boredom = max(0, boredom - 2);
    
    // Sueño pierde 1% por minuto
    sleepiness = max(0, sleepiness - 1);
    
    // Si el sueño llegó a 0%, dormir automáticamente
    if (sleepiness <= 0 && !isSleeping) {
      sleep();
    }
    
    lastMinuteUpdate = currentTime;
    saveStats();
  }
}

bool Tamagotchi::feed() {
  if (isSleeping) return false;
  
  // Validación 1: debe tener 10 monedas (primero)
  if (coins < 10) {
    showInsufficientCoins = true;
    insufficientCoinsTimer = millis();
    return false;
  }
  
  // Validación 2: si tiene >80% hambre (muy lleno)
  if (hunger > 80) {
    showAngryFace = true;
    angryFaceTimer = millis();
    return false;
  }
  
  // Comer cuesta 10 monedas
  coins -= 10;
  
  // Hambre sube 20%
  hunger = min(100, hunger + 20);
  
  // Sueño baja 5%
  sleepiness = max(0, sleepiness - 5);
  
  // Mostrar cara feliz
  showHappyFace = true;
  happyFaceTimer = millis();
  
  saveStats();
  return true;
}

bool Tamagotchi::play() {
  if (isSleeping) return false;
  
  // Permitir jugar siempre (no validar aburrimiento)
  // El juego siempre debe estar disponible
  
  // Hambre baja 15%
  hunger = max(0, hunger - 15);
  
  // Sueño baja 15%
  sleepiness = max(0, sleepiness - 15);
  
  // Mostrar cara feliz
  showHappyFace = true;
  happyFaceTimer = millis();
  
  saveStats();
  return true;
}

bool Tamagotchi::sleep() {
  if (isSleeping) return false;
  
  // Validación: si tiene >25% sueño (no tiene sueño)
  if (sleepiness > 25) {
    showAngryFace = true;
    angryFaceTimer = millis();
    return false;
  }
  
  isSleeping = true;
  sleepStartTime = millis();
  saveStats();
  return true;
}

void Tamagotchi::wakeUp() {
  isSleeping = false;
  
  // Al despertar: sueño al 100%, aburrimiento +20%, hambre -15%
  sleepiness = 100;
  boredom = min(100, boredom + 20);
  hunger = max(0, hunger - 15);
  
  lastMinuteUpdate = millis();
  saveStats();
}

void Tamagotchi::addCoins(int amount) {
  coins += amount;
  saveStats();
}

void Tamagotchi::addBoredom(int amount) {
  boredom = min(100, boredom + amount);
  saveStats();
}

int Tamagotchi::getMood() const {
  // Prioridad 1: mostrar cara feliz si está activa
  if (showHappyFace) return 3; // HAPPY (RoboEyes)

  // Prioridad 2: sueño muy bajo → SLEEPY (50% cerrado)
  if (sleepiness < 20) return 4; // SLEEPY (RoboEyes)
  
  // Prioridad 3: mostrar cara enfadada si está activa
  if (showAngryFace) return 2; // ANGRY
  
  // Hambre < 20% → ANGRY (2)
  if (hunger < 20) return 2;
  
  // Aburrimiento < 20% → SAD (5)
  if (boredom < 20) return 5;
  
  // Normal/feliz
  return 0; // DEFAULT
}

void Tamagotchi::saveStats() {
  prefs.putInt("hunger", hunger);
  prefs.putInt("boredom", boredom);
  prefs.putInt("sleep", sleepiness);
  prefs.putInt("coins", coins);
  prefs.putBool("sleeping", isSleeping);
}

void Tamagotchi::loadStats() {
  hunger = prefs.getInt("hunger", 100);
  boredom = prefs.getInt("boredom", 100);
  sleepiness = prefs.getInt("sleep", 100);
  coins = prefs.getInt("coins", 0);
  isSleeping = prefs.getBool("sleeping", false);
}
