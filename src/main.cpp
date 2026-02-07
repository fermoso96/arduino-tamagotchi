#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "tamagotchi.h"
#include "game.h"
#include "memorygame.h"
#include "display.h"

// Configuración de pines
#define BTN_LEFT 0
#define BTN_ENTER 1
#define BTN_RIGHT 2
#define BUZZER_PIN 5
#define I2C_SDA 8
#define I2C_SCL 9

// Declarar el display
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Variables globales
Tamagotchi pet;
DodgeGame game;
MemoryGame memoryGame;
DisplayManager displayMgr;
unsigned long lastUpdateTime = 0;
unsigned long gameStartTime = 0;
unsigned long menuOpenTime = 0;
bool inGame = false;
bool inMemoryGame = false;
bool showMenu = false;
bool showGameMenu = false; // Submenú de juegos
bool showShopMenu = false; // Submenú de tienda
bool testMode = true; // Modo TEST activado/desactivado
int menuOption = 0; // 0: TIENDA, 1: JUGAR, 2: DORMIR
int gameMenuOption = 0; // 0: ESQUIVAR, 1: MEMORIA
int shopMenuOption = 0; // 0: Manzana, 1: Pan, 2: Queso, 3: Tarta, 4: Juego de memoria
const unsigned long MENU_TIMEOUT = 5000; // Cerrar menú después de 5 segundos sin actividad

// Declaraciones forward
void playHappySound();
void playAngrySound();
void playBeep();
void startGame();
void updateGame();
void endGame();
void startMemoryGame();
void updateMemoryGame();
void endMemoryGame();
void handleButtons();
void playSound(int frequency, int duration);

// Sonido feliz: melodía ascendente
void playHappySound() {
  int melody[] = {2500, 3000, 3500, 4000};
  int noteDuration = 80;
  for (int i = 0; i < 4; ++i) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 10);
  }
  noTone(BUZZER_PIN);
}

// Sonido enfadado: dos notas graves descendentes
void playAngrySound() {
  int melody[] = {1200, 900};
  int noteDuration = 120;
  for (int i = 0; i < 2; ++i) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 10);
  }
  noTone(BUZZER_PIN);
}

void playSound(int frequency, int duration);

void playSound(int frequency, int duration) {
  int testDuration = duration;
  if (testDuration < 100) testDuration = 100;
  tone(BUZZER_PIN, frequency, testDuration);
  delay(testDuration + 20);
  noTone(BUZZER_PIN);
}

// Bip simple para botón
void playBeep() {
  tone(BUZZER_PIN, 3000, 60);
  delay(70);
  noTone(BUZZER_PIN);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  unsigned long serialStart = millis();
  while (!Serial && (millis() - serialStart < 2000)) {
    delay(10);
  }
  delay(200);
  log_i("=== TAMAGOTCHI START ===");
  // Inicializar pines de botones (Solo 3 botones)
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  
  // Inicializar buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Configurar pines I2C personalizados
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializar display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    log_i("SSD1306 allocation failed");
    for (;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Tamagotchi Init...");
  display.display();
  
  // Inicializar tamagotchi
  pet.initialize();
  
  // Inicializar juego
  game.initialize();
  
  // Inicializar juego de memoria
  memoryGame.initialize();
  
  // Inicializar display manager
  displayMgr.initialize(&display, &pet);
  
  log_i("Tamagotchi initialized successfully!");
}

void loop() {
  unsigned long currentTime = millis();
  static unsigned long lastHeartbeat = 0;
  
  // Actualizar estado del tamagotchi cada 500ms
  if (currentTime - lastUpdateTime >= 500) {
    lastUpdateTime = currentTime;
    pet.update();
  }
  
  // === ACTIVAR/DESACTIVAR MODO TEST ===
  // Mantener los 3 botones presionados durante 3 segundos
  static unsigned long allBtnPressStart = 0;
  static bool allBtnProcessed = false;
  
  if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_ENTER) == LOW && digitalRead(BTN_RIGHT) == LOW) {
    if (allBtnPressStart == 0) {
      allBtnPressStart = millis();
    } else if (!allBtnProcessed && (millis() - allBtnPressStart >= 3000)) {
      testMode = !testMode; // Alternar modo TEST
      // Pitido de confirmación: 2 tonos si se activa, 1 tono si se desactiva
      if (testMode) {
        playBeep();
        delay(100);
        playBeep();
      } else {
        playBeep();
      }
      allBtnProcessed = true;
    }
  } else {
    allBtnPressStart = 0;
    allBtnProcessed = false;
  }
  
  // === MODO TEST: Mantener botones 2 segundos para restaurar stats ===
  // Solo funciona en pantalla principal (no en menús, juegos, etc.) y si testMode está activado
  if (testMode && !showMenu && !showShopMenu && !showGameMenu && !inGame && !inMemoryGame && !pet.isSleeping) {
    static unsigned long btnLeftPressStart = 0;
    static unsigned long btnEnterPressStart = 0;
    static unsigned long btnRightPressStart = 0;
    static bool btnLeftProcessed = false;
    static bool btnEnterProcessed = false;
    static bool btnRightProcessed = false;
    
    // Botón IZQUIERDO - Restaurar hambre al 100%
    if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_ENTER) == HIGH && digitalRead(BTN_RIGHT) == HIGH) {
      if (btnLeftPressStart == 0) {
        btnLeftPressStart = millis();
      } else if (!btnLeftProcessed && (millis() - btnLeftPressStart >= 2000)) {
        playBeep();
        pet.setHunger(100);
        btnLeftProcessed = true;
      }
    } else {
      btnLeftPressStart = 0;
      btnLeftProcessed = false;
    }
    
    // Botón CENTRO - Restaurar aburrimiento al 100%
    if (digitalRead(BTN_ENTER) == LOW && digitalRead(BTN_LEFT) == HIGH && digitalRead(BTN_RIGHT) == HIGH) {
      if (btnEnterPressStart == 0) {
        btnEnterPressStart = millis();
      } else if (!btnEnterProcessed && (millis() - btnEnterPressStart >= 2000)) {
        playBeep();
        pet.setBoredom(100);
        btnEnterProcessed = true;
      }
    } else {
      btnEnterPressStart = 0;
      btnEnterProcessed = false;
    }
    
    // Botón DERECHO - Restaurar sueño al 100%
    if (digitalRead(BTN_RIGHT) == LOW && digitalRead(BTN_LEFT) == HIGH && digitalRead(BTN_ENTER) == HIGH) {
      if (btnRightPressStart == 0) {
        btnRightPressStart = millis();
      } else if (!btnRightProcessed && (millis() - btnRightPressStart >= 2000)) {
        playBeep();
        pet.setSleepiness(100);
        btnRightProcessed = true;
      }
    } else {
      btnRightPressStart = 0;
      btnRightProcessed = false;
    }
  }
  
  // Mostrar animación ANGRY o HAPPY si corresponde (superpone todo y bloquea botones)
  static bool angrySoundPlayed = false;
  static bool happySoundPlayed = false;
  if (pet.showAngryFace) {
    displayMgr.showMainScreen(); // Mostrará los ojos en modo ANGRY
    if (!angrySoundPlayed) {
      playAngrySound();
      angrySoundPlayed = true;
    }
    // NO procesar botones durante la animación
  } else if (pet.showHappyFace) {
    displayMgr.showMainScreen(); // Mostrará los ojos en modo HAPPY
    if (!happySoundPlayed) {
      playHappySound();
      happySoundPlayed = true;
    }
    angrySoundPlayed = false;
    // NO procesar botones durante la animación
  } else {
    // Solo procesar botones si NO hay animaciones activas
    // Y tampoco si se está mostrando el mensaje de monedas insuficientes
    // Y tampoco si estamos en la tienda (la tienda tiene su propia lógica de botones)
    if (!pet.showInsufficientCoins && !showShopMenu) {
      handleButtons();
    }
    happySoundPlayed = false;
    angrySoundPlayed = false;
    // Prioridad: si estamos en la tienda, gestionar eso primero
    if (showShopMenu) {
      // Controles de la tienda y renderizado
      int totalShopItems = pet.getMemoryGameUnlocked() ? 4 : 5;
      // Si hay mensaje de monedas insuficientes, mostrarlo y bloquear controles
      if (pet.showInsufficientCoins) {
        // Solo mostrar la pantalla de mensaje, sin aceptar controles
        displayMgr.showInsufficientCoinsScreen();
        if (millis() - menuOpenTime > MENU_TIMEOUT) {
          showShopMenu = false;
        }
      } else {
      // === MODO TEST en tienda: Ajustar monedas ===
      if (testMode) {
        static unsigned long shopLeftPressStart = 0;
        static unsigned long shopRightPressStart = 0;
        static bool shopLeftProcessed = false;
        static bool shopRightProcessed = false;
        
        // Botón IZQUIERDO - Poner monedas a 0
        if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_ENTER) == HIGH && digitalRead(BTN_RIGHT) == HIGH) {
          if (shopLeftPressStart == 0) {
            shopLeftPressStart = millis();
          } else if (!shopLeftProcessed && (millis() - shopLeftPressStart >= 3000)) {
            playBeep();
            pet.setCoins(0);
            shopLeftProcessed = true;
          }
        } else {
          shopLeftPressStart = 0;
          shopLeftProcessed = false;
        }
        
        // Botón DERECHO - Poner monedas a 50
        if (digitalRead(BTN_RIGHT) == LOW && digitalRead(BTN_LEFT) == HIGH && digitalRead(BTN_ENTER) == HIGH) {
          if (shopRightPressStart == 0) {
            shopRightPressStart = millis();
          } else if (!shopRightProcessed && (millis() - shopRightPressStart >= 3000)) {
            playBeep();
            pet.setCoins(50);
            shopRightProcessed = true;
          }
        } else {
          shopRightPressStart = 0;
          shopRightProcessed = false;
        }
      }
      
      // Detección de botones por flanco ascendente (al soltar)
      static bool lastShopLeftState = HIGH;
      static bool lastShopRightState = HIGH;
      static bool lastShopEnterState = HIGH;
      
      bool currentShopLeftState = digitalRead(BTN_LEFT);
      bool currentShopRightState = digitalRead(BTN_RIGHT);
      bool currentShopEnterState = digitalRead(BTN_ENTER);
      
      // Botón izquierda - detectar al soltar
      if (lastShopLeftState == LOW && currentShopLeftState == HIGH) {
        shopMenuOption--;
        if (shopMenuOption < 0) shopMenuOption = totalShopItems - 1;
        playBeep();
        menuOpenTime = millis();
        delay(50);
      }
      
      // Botón derecha - detectar al soltar
      if (lastShopRightState == LOW && currentShopRightState == HIGH) {
        shopMenuOption++;
        if (shopMenuOption >= totalShopItems) shopMenuOption = 0;
        playBeep();
        menuOpenTime = millis();
        delay(50);
      }
      
      // Botón Enter - detectar al soltar
      if (lastShopEnterState == LOW && currentShopEnterState == HIGH) {
        bool bought = false;
        bool insufficient = false;
        if (shopMenuOption == 4 && !pet.getMemoryGameUnlocked()) {
          bought = pet.buyMemoryGame();
          if (!bought) { playSound(150, 50); insufficient = true; }
          else {
            // Si compra exitosa, cerrar tienda y dejar que la animación happy se gestione en el loop
            pet.showInsufficientCoins = false;
            showShopMenu = false;
          }
        } else {
          bought = pet.buyFood(shopMenuOption);
          if (!bought) { playSound(150, 50); insufficient = true; }
          else {
            // Si compra exitosa, cerrar tienda y dejar que la animación happy se gestione en el loop
            pet.showInsufficientCoins = false;
            showShopMenu = false;
          }
        }
        menuOpenTime = millis();
        if (shopMenuOption == 4 && bought) {
          shopMenuOption = 0;
        }
        if (insufficient) {
          pet.showInsufficientCoins = true;
          pet.insufficientCoinsTimer = millis();
          // NO cerrar la tienda, solo mostrar el mensaje superpuesto
          // showShopMenu permanece TRUE
        }
        delay(100);
      }
      
      // Actualizar estados anteriores
      lastShopLeftState = currentShopLeftState;
      lastShopRightState = currentShopRightState;
      lastShopEnterState = currentShopEnterState;
      
        displayMgr.showShopMenuScreen(shopMenuOption, pet.getMemoryGameUnlocked());
        if (millis() - menuOpenTime > MENU_TIMEOUT) {
          showShopMenu = false;
        }
      } // Fin del else de controles de tienda
    } else if (pet.showInsufficientCoins) {
      // Mostrar mensaje de monedas insuficientes (sin estar en tienda)
      displayMgr.showInsufficientCoinsScreen();
    } else if (pet.isSleeping) {
      // Mostrar pantalla de sueño
      displayMgr.showSleepScreen();
    } else if (inGame) {
      updateGame();
    } else if (inMemoryGame) {
      updateMemoryGame();
    } else if (showGameMenu) {
      displayMgr.showGameMenuScreen(gameMenuOption);
      // Cerrar menú si pasó mucho tiempo
      if (millis() - menuOpenTime > MENU_TIMEOUT) {
        showGameMenu = false;
      }
    } else if (showMenu) {
      displayMgr.showMenuScreen(menuOption);
      // Cerrar menú si pasó mucho tiempo
      if (millis() - menuOpenTime > MENU_TIMEOUT) {
        showMenu = false;
      }
  } else {
    displayMgr.showMainScreen();
  }
}

if (currentTime - lastHeartbeat >= 2000) {
    lastHeartbeat = currentTime;
    log_i("Stats - H:%d%% B:%d%% S:%d%% Coins:%d", 
          pet.getHunger(), pet.getBoredom(), pet.getSleepiness(), pet.getCoins());
  }
  
  // Sin delay, máxima fluidez
}

void handleButtons() {
  // Variables estáticas para detección de flanco ascendente (excepto en juegos)
  static bool lastMenuLeftState = HIGH;
  static bool lastMenuRightState = HIGH;
  static bool lastMenuEnterState = HIGH;
  
  // Si está durmiendo, cualquier botón lo despierta
  if (pet.isSleeping) {
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar al soltar cualquier botón
    if ((lastMenuLeftState == LOW && currentLeftState == HIGH) ||
        (lastMenuRightState == LOW && currentRightState == HIGH) ||
        (lastMenuEnterState == LOW && currentEnterState == HIGH)) {
      pet.wakeUp();
      playBeep();
      delay(100);
    }
    
    lastMenuLeftState = currentLeftState;
    lastMenuRightState = currentRightState;
    lastMenuEnterState = currentEnterState;
    return; // No procesar más botones mientras duerme
  }
  
  if (inGame) {
    // Controles del juego de esquivar - MANTENER LÓGICA ORIGINAL (al pulsar)
    // Botón izquierda
    if (digitalRead(BTN_LEFT) == LOW) {
      delay(50);
      if (digitalRead(BTN_LEFT) == LOW) {
        game.moveLeft();
        delay(150);
      }
    }
    
    // Botón derecha
    if (digitalRead(BTN_RIGHT) == LOW) {
      delay(50);
      if (digitalRead(BTN_RIGHT) == LOW) {
        game.moveRight();
        delay(150);
      }
    }
    
    // Botón Enter para salir del juego
    if (digitalRead(BTN_ENTER) == LOW) {
      delay(50);
      if (digitalRead(BTN_ENTER) == LOW) {
        endGame();
        delay(200);
      }
    }
  } else if (inMemoryGame) {
    // Controles del juego de memoria - MANTENER LÓGICA ORIGINAL (al pulsar)
    // Solo aceptar inputs cuando el juego está esperando
    if (memoryGame.isWaitingForInput()) {
      // Botón izquierda
      if (digitalRead(BTN_LEFT) == LOW) {
        delay(50);
        if (digitalRead(BTN_LEFT) == LOW) {
          memoryGame.registerInput(0);
          playBeep();
          delay(150);
        }
      }
      
      // Botón Enter (centro)
      if (digitalRead(BTN_ENTER) == LOW) {
        delay(50);
        if (digitalRead(BTN_ENTER) == LOW) {
          memoryGame.registerInput(1);
          playBeep();
          delay(150);
        }
      }
      
      // Botón derecha
      if (digitalRead(BTN_RIGHT) == LOW) {
        delay(50);
        if (digitalRead(BTN_RIGHT) == LOW) {
          memoryGame.registerInput(2);
          playBeep();
          delay(150);
        }
      }
    } else {
      // Botón Enter para salir del juego
      if (digitalRead(BTN_ENTER) == LOW) {
        delay(50);
        if (digitalRead(BTN_ENTER) == LOW) {
          endMemoryGame();
          delay(200);
        }
      }
    }
  } else if (showGameMenu) {
    // Controles del submenú de juegos - DETECTAR AL SOLTAR
    int totalGames = pet.getMemoryGameUnlocked() ? 2 : 1;
    
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Botón izquierda - navegar izquierda
    if (lastMenuLeftState == LOW && currentLeftState == HIGH) {
      gameMenuOption--;
      if (gameMenuOption < 0) gameMenuOption = totalGames - 1;
      playBeep();
      menuOpenTime = millis();
      delay(50);
    }
    
    // Botón derecha - navegar derecha
    if (lastMenuRightState == LOW && currentRightState == HIGH) {
      gameMenuOption++;
      if (gameMenuOption >= totalGames) gameMenuOption = 0;
      playBeep();
      menuOpenTime = millis();
      delay(50);
    }
    
    // Botón Enter - seleccionar juego
    if (lastMenuEnterState == LOW && currentEnterState == HIGH) {
      log_i("Game menu: Selected option %d", gameMenuOption);
      bool success = pet.play();
      log_i("pet.play() returned: %d (coins: %d)", success, pet.getCoins());
      if (success) {
        showGameMenu = false;
        if (gameMenuOption == 0) {
          log_i("Starting dodge game...");
          startGame();
        } else if (gameMenuOption == 1 && pet.getMemoryGameUnlocked()) {
          log_i("Starting memory game...");
          startMemoryGame();
        }
      } else {
        log_i("Not enough coins to play!");
        playSound(150, 50);
        showGameMenu = false;
      }
      delay(100);
    }
    
    lastMenuLeftState = currentLeftState;
    lastMenuRightState = currentRightState;
    lastMenuEnterState = currentEnterState;
  } else if (showMenu) {
    // Controles del menú principal - DETECTAR AL SOLTAR
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Botón izquierda - navegar izquierda
    if (lastMenuLeftState == LOW && currentLeftState == HIGH) {
      menuOption--;
      if (menuOption < 0) menuOption = 2;
      playBeep();
      menuOpenTime = millis();
      delay(50);
    }
    
    // Botón derecha - navegar derecha
    if (lastMenuRightState == LOW && currentRightState == HIGH) {
      menuOption++;
      if (menuOption > 2) menuOption = 0;
      playBeep();
      menuOpenTime = millis();
      delay(50);
    }
    
    // Botón Enter - seleccionar acción
    if (lastMenuEnterState == LOW && currentEnterState == HIGH) {
      bool success = false;
      switch(menuOption) {
        case 0: // TIENDA
          showMenu = false;
          showShopMenu = true;
          shopMenuOption = 0;
          menuOpenTime = millis();
          playSound(200, 100);
          break;
        case 1: // JUGAR
          showMenu = false;
          showGameMenu = true;
          gameMenuOption = 0;
          menuOpenTime = millis();
          playSound(150, 100);
          break;
        case 2: // DORMIR
          success = pet.sleep();
          if (success) {
            playSound(200, 50);
          } else {
            playSound(150, 50);
          }
          showMenu = false;
          break;
      }
      delay(100);
    }
    
    lastMenuLeftState = currentLeftState;
    lastMenuRightState = currentRightState;
    lastMenuEnterState = currentEnterState;
  } else {
    // Vista normal - solo ojos visible - DETECTAR AL SOLTAR
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Botón Enter - abrir menú (solo si no está durmiendo)
    if (lastMenuEnterState == LOW && currentEnterState == HIGH && !pet.isSleeping) {
      showMenu = true;
      menuOpenTime = millis();
      menuOption = 0;
      playSound(150, 100);
      delay(100);
    }
    
    lastMenuEnterState = currentEnterState;
  }
}

void startGame() {
  log_i("=== STARTING DODGE GAME ===");
  inGame = true;
  gameStartTime = millis();
  game.reset();
  playSound(400, 100);
  log_i("Dodge game started. inGame=%d", inGame);
}

void updateGame() {
  game.update();
  
  // Mostrar pantalla del juego
  displayMgr.showGameScreen(&game);
  
  // Detectar colisión
  if (game.checkCollision()) {
    endGame();
  }
  
  // Salir del juego con botón ENTER (presión larga o simple, simple por ahora)
  if (digitalRead(BTN_ENTER) == LOW) {
    delay(50);
    if (digitalRead(BTN_ENTER) == LOW) {
      endGame();
      delay(200);
    }
  }
}

void endGame() {
  inGame = false;
  
  // Guardar récord si se ha superado
  game.saveRecord();
  
  // Calcular monedas ganadas basadas en el nivel alcanzado
  // Fórmula: nivel * (nivel + 1) / 2 + 2 monedas por cada nivel completado (excepto el primero)
  int finalLevel = game.getLevel();
  int coinsEarned = (finalLevel * (finalLevel + 1)) / 2;
  if (finalLevel > 1) {
    coinsEarned += 2 * (finalLevel - 1); // 2 monedas extras por cada nivel después del 1
  }
  pet.addCoins(coinsEarned);
  pet.addBoredom(coinsEarned);
  
  // Mostrar pantalla de game over durante 3 segundos
  unsigned long gameOverStart = millis();
  while (millis() - gameOverStart < 3000) {
    displayMgr.showGameOver(&game, coinsEarned);
    delay(10);
  }
  
  // DESPUÉS de los 3 segundos, activar animación HAPPY
  pet.showHappyFace = true;
  pet.happyFaceTimer = millis();
  
  // Nota: La animación happy se gestionará en el loop principal
}

void startMemoryGame() {
  // ...existing code...
  log_i("=== STARTING MEMORY GAME ===");
  inMemoryGame = true;
  gameStartTime = millis();
  memoryGame.reset();
  playSound(400, 100);
  log_i("Memory game started. inMemoryGame=%d", inMemoryGame);
}

void updateMemoryGame() {
  memoryGame.update();
  
  // Mostrar pantalla del juego de memoria
  displayMgr.showMemoryGameScreen(&memoryGame);
}

void endMemoryGame() {
  inMemoryGame = false;
  
  // Calcular monedas ganadas basadas en el nivel alcanzado
  int finalLevel = memoryGame.getLevel();
  int coinsEarned = (finalLevel * (finalLevel + 1)) / 2;
  pet.addCoins(coinsEarned);
  pet.addBoredom(coinsEarned);
  
  // Mostrar animación HAPPY y dos pitidos agudos
  pet.showHappyFace = true;
  pet.happyFaceTimer = millis();
  playSound(1200, 50);
  delay(0);
  playSound(1200, 50);
  // Esperar 0.2 segundos para la animación
  unsigned long t0 = millis();
  while (millis() - t0 < 200) {
    displayMgr.showMainScreen();
    delay(0);
  }
  // Mostrar pantalla de fin de juego
  displayMgr.showMemoryGameOver(&memoryGame, coinsEarned);
  // Sin delay
}
