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
  playHungrySound = false;
  playBoredSound = false;
  playSleepySound = false;
  
  lastMinuteUpdate = 0;
  sleepStartTime = 0;
  lastSleepTick = 0;
  wasHungry = false;
  wasBored = false;
  wasSleepy = false;
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
  sleepiness = max(0, sleepiness - 10); // Reducir sueño 10% al comer
  
  // Verificar si sueño llegó a 20 o menos
  if (sleepiness <= 20 && !wasSleepy) {
    playSleepySound = true;
    wasSleepy = true;
  } else if (sleepiness > 20) {
    wasSleepy = false;
  }
  
  showHappyFace = true;
  happyFaceTimer = millis();
  saveStats();
  return true;
}

// Lógica de compra del juego de memoria
bool Tamagotchi::buyMemoryGame() {
  if (memoryGameUnlocked) return false;
  if (coins < 100) {
    showInsufficientCoins = true;
    insufficientCoinsTimer = millis();
    return false;
  }
  coins -= 100;
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
    // Cada 5 segundos, aumentar sueño en 1%
    if (currentTime - lastSleepTick >= 5000) {
      sleepiness = min(100, sleepiness + 1);
      lastSleepTick = currentTime;
      saveStats();
      
      // Si llega a 100%, despertar automáticamente
      if (sleepiness >= 100) {
        wakeUp();
      }
    }
    return; // No actualizar otros parámetros mientras duerme
  }
  
  // Actualizar cada minuto
  updatePerMinute();
}

void Tamagotchi::updatePerMinute() {
  unsigned long currentTime = millis();
  if (currentTime - lastMinuteUpdate >= 60000) { // 60 segundos
    // Hambre pierde 1% por minuto
    hunger = max(0, hunger - 1);
    
    // Detectar si hambre llegó a 20 o menos
    if (hunger <= 20 && !wasHungry) {
      playHungrySound = true;
      wasHungry = true;
    } else if (hunger > 20) {
      wasHungry = false;
    }
    
    // Aburrimiento pierde 1% por minuto
    boredom = max(0, boredom - 1);
    
    // Detectar si aburrimiento llegó a 20 o menos
    if (boredom <= 20 && !wasBored) {
      playBoredSound = true;
      wasBored = true;
    } else if (boredom > 20) {
      wasBored = false;
    }
    
    // Sueño pierde 1% cada 2 minutos (contador interno)
    static int sleepCounter = 0;
    sleepCounter++;
    if (sleepCounter >= 2) {
      sleepiness = max(0, sleepiness - 1);
      sleepCounter = 0;
    }
    
    // Detectar si sueño llegó a 20 o menos
    if (sleepiness <= 20 && !wasSleepy) {
      playSleepySound = true;
      wasSleepy = true;
    } else if (sleepiness > 20) {
      wasSleepy = false;
    }
    
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
  
  // Verificar si sueño llegó a 20 o menos
  if (sleepiness <= 20 && !wasSleepy) {
    playSleepySound = true;
    wasSleepy = true;
  } else if (sleepiness > 20) {
    wasSleepy = false;
  }
  
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
  
  // Verificar si hambre llegó a 20 o menos
  if (hunger <= 20 && !wasHungry) {
    playHungrySound = true;
    wasHungry = true;
  } else if (hunger > 20) {
    wasHungry = false;
  }
  
  // Sueño baja 15%
  sleepiness = max(0, sleepiness - 15);
  
  // Verificar si sueño llegó a 20 o menos
  if (sleepiness <= 20 && !wasSleepy) {
    playSleepySound = true;
    wasSleepy = true;
  } else if (sleepiness > 20) {
    wasSleepy = false;
  }
  
  // NO mostrar cara feliz al iniciar (se mostrará al terminar el juego)
  
  saveStats();
  return true;
}

bool Tamagotchi::sleep() {
  if (isSleeping) return false;
  
  // Validación: si tiene >20% sueño (no tiene sueño)
  if (sleepiness > 20) {
    showAngryFace = true;
    angryFaceTimer = millis();
    return false;
  }
  
  isSleeping = true;
  sleepStartTime = millis();
  lastSleepTick = millis();
  saveStats();
  return true;
}

void Tamagotchi::wakeUp() {
  // Si despierta con 20% o menos de sueño, mostrar animación enfadado
  if (sleepiness <= 20) {
    showAngryFace = true;
    angryFaceTimer = millis();
  }
  
  isSleeping = false;
  
  // Al despertar: mantener sueño actual, aburrimiento +20%, hambre -15%
  boredom = min(100, boredom + 20);
  hunger = max(0, hunger - 15);
  
  // Verificar si hambre llegó a 20 o menos
  if (hunger <= 20 && !wasHungry) {
    playHungrySound = true;
    wasHungry = true;
  } else if (hunger > 20) {
    wasHungry = false;
  }
  
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

  // Prioridad 2: mostrar cara enfadada si está activa
  if (showAngryFace) return 2; // ANGRY
  
  // Prioridad 3: sueño muy bajo → SLEEPY (50% cerrado)
  if (sleepiness < 20) return 4; // SLEEPY (RoboEyes)
  
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
