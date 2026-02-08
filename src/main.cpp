#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Preferences.h>
#include "tamagotchi.h"
#include "game.h"
#include "memorygame.h"
#include "tictactoe.h"
#include "display.h"

// Configuración de pines
#define BTN_ENTER 1
#define BUZZER_PIN 5
#define I2C_SDA 8
#define I2C_SCL 9

// Declarar el display
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Variables globales
Tamagotchi pet;
DodgeGame game;
MemoryGame memoryGame;
TicTacToeGame ticTacToeGame;
DisplayManager displayMgr;
unsigned long lastUpdateTime = 0;
unsigned long gameStartTime = 0;
unsigned long menuOpenTime = 0;
bool inGame = false;
bool inMemoryGame = false;
bool inTicTacToe = false;
bool showMenu = false;
bool showGameMenu = false; // Submenú de juegos
bool showShopMenu = false; // Submenú de tienda
bool soundEnabled = true; // Control de sonido (ON/OFF)
int menuOption = 0; // 0: TIENDA, 1: JUGAR, 2: DORMIR, 3: SOUND
int gameMenuOption = 0; // 0: ESQUIVAR, 1: MEMORIA
int shopMenuOption = 0; // 0: Manzana, 1: Pan, 2: Queso, 3: Tarta, 4: Juego de memoria
const unsigned long MENU_TIMEOUT = 5000; // Cerrar menú después de 5 segundos sin actividad
const unsigned long LONG_PRESS_TIME = 500; // 500ms para considerar pulsación larga
Preferences soundPrefs; // Para guardar estado del sonido

// Declaraciones forward
void playHappySound();
void playAngrySound();
void playBoredSound();
void playSleepySound();
void playBeep();
void startGame();
void updateGame();
void endGame();
void startMemoryGame();
void updateMemoryGame();
void endMemoryGame();
void startTicTacToe();
void updateTicTacToe();
void endTicTacToe();
void handleButtons();
void playSound(int frequency, int duration);

// Sonido feliz: melodía ascendente
void playHappySound() {
  if (!soundEnabled) return;
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
  if (!soundEnabled) return;
  int melody[] = {1200, 900};
  int noteDuration = 120;
  for (int i = 0; i < 2; ++i) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 10);
  }
  noTone(BUZZER_PIN);
}

// Sonido aburrido: tres notas monótonas y bajas
void playBoredSound() {
  if (!soundEnabled) return;
  int melody[] = {600, 600, 600};
  int noteDuration = 150;
  for (int i = 0; i < 3; ++i) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 30);
  }
  noTone(BUZZER_PIN);
}

// Sonido de sueño: dos notas descendentes suaves y lentas
void playSleepySound() {
  if (!soundEnabled) return;
  int melody[] = {800, 400};
  int noteDuration = 200;
  for (int i = 0; i < 2; ++i) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 50);
  }
  noTone(BUZZER_PIN);
}

void playSound(int frequency, int duration);

void playSound(int frequency, int duration) {
  if (!soundEnabled) return;
  int testDuration = duration;
  if (testDuration < 100) testDuration = 100;
  tone(BUZZER_PIN, frequency, testDuration);
  delay(testDuration + 20);
  noTone(BUZZER_PIN);
}

// Bip simple para botón
void playBeep() {
  if (!soundEnabled) return;
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
  // Inicializar pin del botón central
  pinMode(BTN_ENTER, INPUT_PULLUP);
  
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
  
  // Inicializar juego de tres en raya
  ticTacToeGame.initialize();
  
  // Inicializar display manager
  displayMgr.initialize(&display, &pet);
  
  // Cargar configuración de sonido
  soundPrefs.begin("sound", false);
  soundEnabled = soundPrefs.getBool("enabled", true); // Por defecto ON
  
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
  
  // Reproducir sonidos de estado bajo (hambre, aburrimiento, sueño)
  if (pet.playHungrySound) {
    playAngrySound();
    pet.playHungrySound = false;
  }
  if (pet.playBoredSound) {
    playBoredSound();
    pet.playBoredSound = false;
  }
  if (pet.playSleepySound) {
    playSleepySound();
    pet.playSleepySound = false;
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
      // Calcular total de items disponibles
      int totalShopItems = 4; // Siempre hay 4 comidas
      if (!pet.getMemoryGameUnlocked()) totalShopItems++; // +1 si memoria no desbloqueado
      if (!pet.getTicTacToeUnlocked()) totalShopItems++; // +1 si tres en raya no desbloqueado
      
      // Si hay mensaje de monedas insuficientes, mostrarlo y bloquear controles
      if (pet.showInsufficientCoins) {
        // Solo mostrar la pantalla de mensaje, sin aceptar controles
        displayMgr.showInsufficientCoinsScreen();
        if (millis() - menuOpenTime > MENU_TIMEOUT) {
          showShopMenu = false;
        }
      } else {
      // Detección de pulsaciones: corta = navegar, larga = seleccionar
      static bool shopBtnPressed = false;
      static unsigned long shopBtnPressTime = 0;
      
      bool currentShopEnterState = digitalRead(BTN_ENTER);
      
      // Detectar cuando se presiona el botón
      if (!shopBtnPressed && currentShopEnterState == LOW) {
        shopBtnPressed = true;
        shopBtnPressTime = millis();
      }
      
      // Detectar cuando se suelta el botón
      if (shopBtnPressed && currentShopEnterState == HIGH) {
        unsigned long pressDuration = millis() - shopBtnPressTime;
        shopBtnPressed = false;
        
        if (pressDuration < LONG_PRESS_TIME) {
          // PULSACIÓN CORTA: Navegar hacia abajo (cíclico)
          shopMenuOption++;
          if (shopMenuOption >= totalShopItems) shopMenuOption = 0;
          menuOpenTime = millis();
          delay(50);
        } else {
          // PULSACIÓN LARGA: Seleccionar (comprar)
        bool bought = false;
        bool insufficient = false;
        
        // Mapear la opción seleccionada al item real
        int realItem = shopMenuOption;
        if (shopMenuOption >= 4) {
          // Estamos en la zona de juegos
          if (!pet.getMemoryGameUnlocked()) {
            // Si memoria no está desbloqueado, item 4 = memoria
            if (shopMenuOption == 4) {
              bought = pet.buyMemoryGame();
            } else if (shopMenuOption == 5) {
              // Item 5 = tres en raya
              bought = pet.buyTicTacToeGame();
            }
          } else {
            // Si memoria ya está desbloqueado, item 4 = tres en raya
            if (shopMenuOption == 4) {
              bought = pet.buyTicTacToeGame();
            }
          }
        } else {
          // Comida (items 0-3)
          bought = pet.buyFood(shopMenuOption);
        }
        
        if (!bought) { 
          playSound(150, 50); 
          insufficient = true; 
        } else {
          // Si compra exitosa, cerrar tienda y dejar que la animación happy se gestione en el loop
          pet.showInsufficientCoins = false;
          showShopMenu = false;
        }
        
        menuOpenTime = millis();
        if (bought && shopMenuOption >= 4) {
          shopMenuOption = 0;
        }
        if (insufficient) {
          pet.showInsufficientCoins = true;
          pet.insufficientCoinsTimer = millis();
        }
          delay(100);
        }
      }
      
        displayMgr.showShopMenuScreen(shopMenuOption);
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
    } else if (inTicTacToe) {
      updateTicTacToe();
    } else if (showGameMenu) {
      displayMgr.showGameMenuScreen(gameMenuOption);
      // Cerrar menú si pasó mucho tiempo
      if (millis() - menuOpenTime > MENU_TIMEOUT) {
        showGameMenu = false;
      }
    } else if (showMenu) {
      displayMgr.showMenuScreen(menuOption, soundEnabled);
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
  // Si está durmiendo, pulsación larga lo despierta
  if (pet.isSleeping) {
    static bool sleepBtnPressed = false;
    static unsigned long sleepBtnPressTime = 0;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    if (!sleepBtnPressed && currentEnterState == LOW) {
      sleepBtnPressed = true;
      sleepBtnPressTime = millis();
    }
    
    if (sleepBtnPressed && currentEnterState == HIGH) {
      unsigned long pressDuration = millis() - sleepBtnPressTime;
      sleepBtnPressed = false;
      
      if (pressDuration >= LONG_PRESS_TIME) {
        pet.wakeUp();
        delay(100);
      }
    }
    return;
  }
  
  if (inGame) {
    // Controles del juego de esquivar - SOLO BOTÓN CENTRO
    static bool lastGameEnterState = HIGH;
    bool currentGameEnterState = digitalRead(BTN_ENTER);
    
    // Detectar pulsación del botón (al soltar para evitar múltiples cambios)
    if (lastGameEnterState == LOW && currentGameEnterState == HIGH) {
      game.toggleLane();
      delay(100);  // Debounce
    }
    
    lastGameEnterState = currentGameEnterState;
  } else if (inMemoryGame) {
    // Controles del juego de memoria - SOLO BOTÓN CENTRO
    // Variables estáticas para medir duración de pulsación
    static bool memoryButtonPressed = false;
    static unsigned long memoryButtonPressTime = 0;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar cuando se PULSA el botón
    if (!memoryButtonPressed && currentEnterState == LOW) {
      memoryButtonPressed = true;
      memoryButtonPressTime = millis();
      memoryGame.registerButtonPress();
      delay(50);  // Debounce
    }
    
    // Detectar cuando se SUELTA el botón
    if (memoryButtonPressed && currentEnterState == HIGH) {
      unsigned long pressDuration = millis() - memoryButtonPressTime;
      memoryButtonPressed = false;
      
      // Registrar el símbolo según la duración
      memoryGame.registerButtonRelease(pressDuration);
      
      // Reproducir el mismo pitido que en la secuencia
      if (pressDuration < 400) {
        // Punto: pitido corto
        playSound(1000, 100);
      } else {
        // Raya: pitido largo
        playSound(800, 300);
      }
      
      delay(100);
    }
  } else if (inTicTacToe) {
    // Controles del juego de tres en raya - SOLO BOTÓN CENTRO
    static bool ticTacBtnPressed = false;
    static unsigned long ticTacBtnPressTime = 0;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar cuando se PULSA el botón
    if (!ticTacBtnPressed && currentEnterState == LOW) {
      ticTacBtnPressed = true;
      ticTacBtnPressTime = millis();
    }
    
    // Detectar cuando se SUELTA el botón
    if (ticTacBtnPressed && currentEnterState == HIGH) {
      unsigned long pressDuration = millis() - ticTacBtnPressTime;
      ticTacBtnPressed = false;
      
      if (pressDuration < LONG_PRESS_TIME) {
        // PULSACIÓN CORTA: Mover cursor
        ticTacToeGame.moveCursor();
        playBeep();
      } else {
        // PULSACIÓN LARGA: Colocar ficha
        if (ticTacToeGame.tryPlacePiece()) {
          playSound(800, 150);
        } else {
          playSound(200, 100);  // Error: casilla ocupada
        }
      }
      
      delay(100);
    }
  } else if (showGameMenu) {
    // Controles del submenú de juegos - Pulsación corta navega, larga selecciona
    // Calcular total de juegos disponibles
    int totalGames = 1; // Siempre hay esquivar
    if (pet.getMemoryGameUnlocked()) totalGames++; // +1 si memoria desbloqueado
    if (pet.getTicTacToeUnlocked()) totalGames++; // +1 si tres en raya desbloqueado
    
    static bool gameMenuBtnPressed = false;
    static unsigned long gameMenuBtnPressTime = 0;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar cuando se presiona
    if (!gameMenuBtnPressed && currentEnterState == LOW) {
      gameMenuBtnPressed = true;
      gameMenuBtnPressTime = millis();
    }
    
    // Detectar cuando se suelta
    if (gameMenuBtnPressed && currentEnterState == HIGH) {
      unsigned long pressDuration = millis() - gameMenuBtnPressTime;
      gameMenuBtnPressed = false;
      
      if (pressDuration < LONG_PRESS_TIME) {
        // PULSACIÓN CORTA: Navegar hacia abajo
        gameMenuOption++;
        if (gameMenuOption >= totalGames) gameMenuOption = 0;
        menuOpenTime = millis();
        delay(50);
      } else {
        // PULSACIÓN LARGA: Seleccionar juego
        log_i("Game menu: Selected option %d", gameMenuOption);
        bool success = pet.play();
        log_i("pet.play() returned: %d (coins: %d)", success, pet.getCoins());
        if (success) {
          showGameMenu = false;
          
          // Mapear la opción a juego real
          if (gameMenuOption == 0) {
            log_i("Starting dodge game...");
            startGame();
          } else if (gameMenuOption == 1) {
            // Item 1 puede ser memoria o tres en raya
            if (pet.getMemoryGameUnlocked()) {
              log_i("Starting memory game...");
              startMemoryGame();
            } else if (pet.getTicTacToeUnlocked()) {
              log_i("Starting tic-tac-toe...");
              startTicTacToe();
            }
          } else if (gameMenuOption == 2) {
            // Item 2 solo existe si memoria está desbloqueado
            if (pet.getTicTacToeUnlocked()) {
              log_i("Starting tic-tac-toe...");
              startTicTacToe();
            }
          }
        } else {
          log_i("Not enough coins to play!");
          playSound(150, 50);
          showGameMenu = false;
        }
        delay(100);
      }
    }
  } else if (showMenu) {
    // Controles del menú principal - Pulsación corta navega, larga selecciona
    static bool mainMenuBtnPressed = false;
    static unsigned long mainMenuBtnPressTime = 0;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar cuando se presiona
    if (!mainMenuBtnPressed && currentEnterState == LOW) {
      mainMenuBtnPressed = true;
      mainMenuBtnPressTime = millis();
    }
    
    // Detectar cuando se suelta
    if (mainMenuBtnPressed && currentEnterState == HIGH) {
      unsigned long pressDuration = millis() - mainMenuBtnPressTime;
      mainMenuBtnPressed = false;
      
      if (pressDuration < LONG_PRESS_TIME) {
        // PULSACIÓN CORTA: Navegar hacia abajo
        menuOption++;
        if (menuOption > 3) menuOption = 0;
        menuOpenTime = millis();
        delay(50);
      } else {
        // PULSACIÓN LARGA: Seleccionar acción
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
          case 3: // SOUND
            soundEnabled = !soundEnabled;
            soundPrefs.putBool("enabled", soundEnabled);
            menuOpenTime = millis();
            break;
        }
        delay(100);
      }
    }
  } else {
    // Vista normal - solo ojos visible - Cualquier pulsación abre menú
    static bool mainScreenBtnPressed = false;
    
    bool currentEnterState = digitalRead(BTN_ENTER);
    
    // Detectar cuando se presiona
    if (!mainScreenBtnPressed && currentEnterState == LOW) {
      mainScreenBtnPressed = true;
    }
    
    // Detectar cuando se suelta
    if (mainScreenBtnPressed && currentEnterState == HIGH) {
      mainScreenBtnPressed = false;
      
      // Abrir menú con cualquier pulsación
      showMenu = true;
      menuOpenTime = millis();
      menuOption = 0;
      playSound(150, 100);
      delay(100);
    }
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
  log_i("=== STARTING MEMORY GAME ===");
  inMemoryGame = true;
  gameStartTime = millis();
  memoryGame.reset();
  playSound(800, 150);
  
  // Pequeña pausa antes de empezar
  delay(500);
  
  // Mostrar la secuencia inicial
  memoryGame.startShowingSequence();
  
  // Reproducir la secuencia con ojos y sonidos
  const int* sequence = memoryGame.getSequence();
  int seqLength = memoryGame.getSequenceLength();
  
  for (int i = 0; i < seqLength; i++) {
    if (sequence[i] == MORSE_DOT) {
      // PUNTO: parpadeo rápido + pitido corto
      displayMgr.showEyesBlink();
      playSound(1000, 100);  // Pitido corto y agudo
      delay(200);
      displayMgr.showEyesNormal();
      delay(300);  // Pausa entre símbolos
    } else {
      // RAYA: parpadeo lento + pitido largo
      displayMgr.showEyesBlink();
      delay(100);
      playSound(800, 300);  // Pitido largo y grave
      delay(200);
      displayMgr.showEyesNormal();
      delay(400);  // Pausa más larga
    }
  }
  
  // Mostrar pantalla "REPITE"
  delay(500);
  memoryGame.startWaitingInput();
  
  log_i("Memory game started. Sequence length: %d", seqLength);
}

void updateMemoryGame() {
  static int lastLevel = -1;
  
  memoryGame.update();
  
  MemoryGameState state = memoryGame.getState();
  int currentLevel = memoryGame.getLevel();
  
  // Detectar avance de nivel
  if (lastLevel >= 0 && currentLevel > lastLevel) {
    // Nivel completado! Mostrar secuencia nueva
    log_i("Level up! New level: %d", currentLevel);
    
    // Pequeña pausa y sonido de éxito
    displayMgr.showEyesNormal();
    playSound(1500, 100);
    delay(200);
    playSound(1800, 100);
    delay(500);
    
    // Mostrar la nueva secuencia
    memoryGame.startShowingSequence();
    const int* sequence = memoryGame.getSequence();
    int seqLength = memoryGame.getSequenceLength();
    
    for (int i = 0; i < seqLength; i++) {
      if (sequence[i] == MORSE_DOT) {
        // PUNTO: parpadeo rápido + pitido corto
        displayMgr.showEyesBlink();
        playSound(1000, 100);
        delay(200);
        displayMgr.showEyesNormal();
        delay(300);
      } else {
        // RAYA: parpadeo lento + pitido largo
        displayMgr.showEyesBlink();
        delay(100);
        playSound(800, 300);
        delay(200);
        displayMgr.showEyesNormal();
        delay(400);
      }
    }
    
    delay(500);
    memoryGame.startWaitingInput();
  }
  
  lastLevel = currentLevel;
  
  if (state == MGS_GAME_OVER) {
    lastLevel = -1;  // Reset para próximo juego
    endMemoryGame();
    return;
  }
  
  // Mostrar pantalla del juego de memoria
  displayMgr.showMemoryGameScreen(&memoryGame);
}

void endMemoryGame() {
  inMemoryGame = false;
  
  // Calcular monedas ganadas: nivel × 3
  int finalLevel = memoryGame.getLevel();
  int coinsEarned = finalLevel * 3;
  
  pet.addCoins(coinsEarned);
  pet.addBoredom(5);  // Pequeño aumento de diversión
  
  // Mostrar animación HAPPY si ganó monedas
  if (coinsEarned > 0) {
    pet.showHappyFace = true;
    pet.happyFaceTimer = millis();
    playSound(1200, 50);
    delay(100);
    playSound(1200, 50);
  }
  
  // Mostrar pantalla de fin de juego
  displayMgr.showMemoryGameOver(&memoryGame, coinsEarned);
  
  // Esperar un momento antes de volver
  delay(3000);
  
  log_i("Memory game ended. Level: %d, Coins earned: %d", finalLevel, coinsEarned);
}

void startTicTacToe() {
  log_i("=== STARTING TIC-TAC-TOE ===");
  inTicTacToe = true;
  gameStartTime = millis();
  ticTacToeGame.reset();
  playSound(600, 150);
  delay(300);
  
  log_i("Tic-Tac-Toe started. Player first: %d", ticTacToeGame.isPlayerFirst());
}

void updateTicTacToe() {
  // Actualizar lógica del juego (IA del Tamagotchi)
  ticTacToeGame.update();
  
  // Mostrar pantalla del juego
  displayMgr.showTicTacToeScreen(&ticTacToeGame);
  
  // Comprobar si el juego ha terminado
  if (ticTacToeGame.getState() == TIC_GAME_OVER) {
    delay(500);  // Pausa antes de mostrar resultado
    endTicTacToe();
  }
}

void endTicTacToe() {
  inTicTacToe = false;
  
  GameResult result = ticTacToeGame.getResult();
  int coinsEarned = 0;
  
  // Actualizar estadísticas
  ticTacToeGame.updateStats(result);
  
  // Calcular monedas según el resultado
  if (result == RESULT_PLAYER_WIN) {
    coinsEarned = 3;  // Victoria: +3 monedas
    pet.addCoins(coinsEarned);
    pet.addBoredom(5);  // Diversión
    
  } else if (result == RESULT_TAMAGOTCHI_WIN) {
    coinsEarned = -1;  // Derrota: -1 moneda
    pet.addCoins(coinsEarned);
    pet.addBoredom(3);  // Algo de diversión
    
  } else if (result == RESULT_DRAW) {
    coinsEarned = 1;  // Empate: +1 moneda
    pet.addCoins(coinsEarned);
    pet.addBoredom(4);  // Diversión moderada
  }
  
  // Mostrar pantalla de fin de juego durante 3 segundos
  unsigned long gameOverStart = millis();
  while (millis() - gameOverStart < 3000) {
    displayMgr.showTicTacToeGameOver(&ticTacToeGame, coinsEarned);
    delay(10);
  }
  
  // DESPUÉS de los 3 segundos, activar animación HAPPY si ganó monedas
  if (coinsEarned > 0) {
    pet.showHappyFace = true;
    pet.happyFaceTimer = millis();
    playSound(1500, 100);
    delay(100);
    playSound(1800, 100);
  }
  
  log_i("Tic-Tac-Toe ended. Result: %d, Coins earned: %d", result, coinsEarned);
}

