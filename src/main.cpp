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
int menuOption = 0; // 0: TIENDA, 1: JUGAR, 2: DORMIR
int gameMenuOption = 0; // 0: ESQUIVAR, 1: MEMORIA
int shopMenuOption = 0; // 0: Manzana, 1: Pan, 2: Queso, 3: Tarta, 4: Juego de memoria
const unsigned long MENU_TIMEOUT = 5000; // Cerrar menú después de 5 segundos sin actividad

// Declaraciones forward
void startGame();
void updateGame();
void endGame();
void startMemoryGame();
void updateMemoryGame();
void endMemoryGame();
void handleButtons();
void playSound(int frequency, int duration);

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
  
  // Leer botones
  handleButtons();
  
  // Mostrar animación ANGRY si corresponde (superpone todo)
  static bool angrySoundPlayed = false;
  static bool happySoundPlayed = false;
  if (pet.showAngryFace) {
    displayMgr.showMainScreen(); // Mostrará los ojos en modo ANGRY
    if (!angrySoundPlayed) {
      for (int i = 0; i < 3; ++i) {
        playSound(200, 500);
        delay(100);
      }
      angrySoundPlayed = true;
    }
  } else if (pet.showHappyFace) {
    displayMgr.showMainScreen(); // Mostrará los ojos en modo HAPPY
    if (!happySoundPlayed) {
      playSound(1200, 1000);
      delay(50);
      playSound(1200, 1000);
      happySoundPlayed = true;
    }
    angrySoundPlayed = false;
  } else {
    happySoundPlayed = false;
    angrySoundPlayed = false;
    if (pet.showInsufficientCoins) {
      // Mostrar mensaje de monedas insuficientes
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
    } else if (showShopMenu) {
      // Controles de la tienda y renderizado
      int totalShopItems = pet.getMemoryGameUnlocked() ? 4 : 5;
      // Botón izquierda
      if (digitalRead(BTN_LEFT) == LOW) {
        delay(50);
        if (digitalRead(BTN_LEFT) == LOW) {
          shopMenuOption--;
          if (shopMenuOption < 0) shopMenuOption = totalShopItems - 1;
          playSound(100, 20);
          menuOpenTime = millis();
          delay(150);
        }
      }
      // Botón derecha
      if (digitalRead(BTN_RIGHT) == LOW) {
        delay(50);
        if (digitalRead(BTN_RIGHT) == LOW) {
          shopMenuOption++;
          if (shopMenuOption >= totalShopItems) shopMenuOption = 0;
          playSound(100, 20);
          menuOpenTime = millis();
          delay(150);
        }
      }
      // Botón Enter - comprar
      if (digitalRead(BTN_ENTER) == LOW) {
        delay(50);
        if (digitalRead(BTN_ENTER) == LOW) {
          bool bought = false;
          bool insufficient = false;
          if (shopMenuOption == 4 && !pet.getMemoryGameUnlocked()) {
            bought = pet.buyMemoryGame();
            // No pitido aquí, lo hace la animación HAPPY
            if (!bought) { playSound(150, 50); insufficient = true; }
          } else {
            bought = pet.buyFood(shopMenuOption);
            if (!bought) { playSound(150, 50); insufficient = true; }
            // Si la compra fue exitosa, mostrar happy y buzzer
            if (bought) {
              showShopMenu = false;
              // Mostrar mood happy y buzzer 2 veces 1s agudo
              unsigned long happyStart = millis();
              for (int i = 0; i < 2; ++i) {
                playSound(1200, 1000);
                delay(1000);
              }
              // Mantener mood happy en pantalla 3 segundos
              while (millis() - happyStart < 3000) {
                displayMgr.showMainScreen();
                delay(10);
              }
            }
          }
          menuOpenTime = millis();
          // Si se compró el juego de memoria, actualizar menú
          if (shopMenuOption == 4 && bought) {
            shopMenuOption = 0;
          }
          // Si no tiene monedas suficientes, salir de la tienda y mostrar mensaje
          if (insufficient) {
            showShopMenu = false;
          } else if (!bought) {
            // Salir de la tienda con Enter largo (mantener presionado)
            unsigned long t0 = millis();
            while (digitalRead(BTN_ENTER) == LOW && millis() - t0 < 800) {
              delay(10);
            }
            if (millis() - t0 >= 800) {
              showShopMenu = false;
            }
          }
          delay(200);
        }
      }
      displayMgr.showShopMenuScreen(shopMenuOption, pet.getMemoryGameUnlocked());
      if (millis() - menuOpenTime > MENU_TIMEOUT) {
        showShopMenu = false;
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
// Variables para la tienda
bool showShopMenu = false;
int shopMenuOption = 0; // 0: Manzana, 1: Pan, 2: Queso, 3: Tarta, 4: Juego de memoria

  if (currentTime - lastHeartbeat >= 2000) {
    lastHeartbeat = currentTime;
    log_i("Stats - H:%d%% B:%d%% S:%d%% Coins:%d", 
          pet.getHunger(), pet.getBoredom(), pet.getSleepiness(), pet.getCoins());
  }
  
  delay(50); // Pequeño delay para evitar saturar el CPU
}

void handleButtons() {
  if (inGame) {
    // Controles del juego de esquivar
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
    // Controles del juego de memoria
    // Solo aceptar inputs cuando el juego está esperando
    if (memoryGame.isWaitingForInput()) {
      // Botón izquierda
      if (digitalRead(BTN_LEFT) == LOW) {
        delay(50);
        if (digitalRead(BTN_LEFT) == LOW) {
          memoryGame.registerInput(0);
          playSound(262, 100); // Do
          delay(150);
        }
      }
      
      // Botón Enter (centro)
      if (digitalRead(BTN_ENTER) == LOW) {
        delay(50);
        if (digitalRead(BTN_ENTER) == LOW) {
          memoryGame.registerInput(1);
          playSound(330, 100); // Mi
          delay(150);
        }
      }
      
      // Botón derecha
      if (digitalRead(BTN_RIGHT) == LOW) {
        delay(50);
        if (digitalRead(BTN_RIGHT) == LOW) {
          memoryGame.registerInput(2);
          playSound(392, 100); // Sol
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
    // Controles del submenú de juegos
    // Determinar cantidad de juegos disponibles
    int totalGames = pet.getMemoryGameUnlocked() ? 2 : 1;
    // Botón izquierda - navegar izquierda
    if (digitalRead(BTN_LEFT) == LOW) {
      delay(50);
      if (digitalRead(BTN_LEFT) == LOW) {
        gameMenuOption--;
        if (gameMenuOption < 0) gameMenuOption = totalGames - 1;
        playSound(100, 20);
        menuOpenTime = millis();
        delay(150);
      }
    }
    // Botón derecha - navegar derecha
    if (digitalRead(BTN_RIGHT) == LOW) {
      delay(50);
      if (digitalRead(BTN_RIGHT) == LOW) {
        gameMenuOption++;
        if (gameMenuOption >= totalGames) gameMenuOption = 0;
        playSound(100, 20);
        menuOpenTime = millis();
        delay(150);
      }
    }
    // Botón Enter - seleccionar juego
    if (digitalRead(BTN_ENTER) == LOW) {
      delay(50);
      if (digitalRead(BTN_ENTER) == LOW) {
        log_i("Game menu: Selected option %d", gameMenuOption);
        // Verificar si tiene suficientes monedas para jugar
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
          // No tiene suficientes monedas
          log_i("Not enough coins to play!");
          playSound(150, 50); // Sonido de error
          showGameMenu = false;
        }
        delay(200);
      }
    }
  } else if (showMenu) {
    // Controles del menú
    // Botón izquierda - navegar izquierda
    if (digitalRead(BTN_LEFT) == LOW) {
      delay(50);
      if (digitalRead(BTN_LEFT) == LOW) {
        menuOption--;
        if (menuOption < 0) menuOption = 2;
        playSound(100, 20); // Click suave
        menuOpenTime = millis(); // Resetear timeout
        delay(150);
      }
    }
    
    // Botón derecha - navegar derecha
    if (digitalRead(BTN_RIGHT) == LOW) {
      delay(50);
      if (digitalRead(BTN_RIGHT) == LOW) {
        menuOption++;
        if (menuOption > 2) menuOption = 0;
        playSound(100, 20); // Click suave
        menuOpenTime = millis(); // Resetear timeout
        delay(150);
      }
    }
    
    // Botón Enter - seleccionar acción o cerrar menú
    if (digitalRead(BTN_ENTER) == LOW) {
      delay(50);
      if (digitalRead(BTN_ENTER) == LOW) {
        bool success = false;
        // Ejecutar acción del menú
        switch(menuOption) {
          case 0: // TIENDA
            showMenu = false;
            showShopMenu = true;
            shopMenuOption = 0;
            menuOpenTime = millis();
            playSound(200, 100);
            break;
          case 1: // JUGAR
            // Abrir submenú de juegos directamente (sin aplicar efectos aún)
            showMenu = false; // Cerrar menú principal
            showGameMenu = true; // Abrir submenú de juegos
            gameMenuOption = 0; // Resetear a primera opción
            menuOpenTime = millis(); // Resetear timeout
            playSound(150, 100);
            break;
          case 2: // DORMIR
            success = pet.sleep();
            if (success) {
              playSound(200, 50);
            } else {
              playSound(150, 50); // Sonido de error
            }
            showMenu = false; // Cerrar menú después de la acción
            break;
        }
        delay(200);
      }
    }
  } else {
    // Vista normal - solo ojos visible
    // Botón Enter - abrir menú (solo si no está durmiendo)
    if (digitalRead(BTN_ENTER) == LOW && !pet.isSleeping) {
      delay(50);
      if (digitalRead(BTN_ENTER) == LOW) {
        showMenu = true;
        menuOpenTime = millis();
        menuOption = 0; // Resetear a la primera opción
        playSound(150, 100);
        delay(200);
      }
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
  
  // Calcular monedas ganadas basadas en el nivel alcanzado
  // Fórmula: nivel * (nivel + 1) / 2
  // Nivel 1: 1 moneda, Nivel 2: 3 monedas, Nivel 3: 6 monedas, Nivel 4: 10 monedas, etc
  int finalLevel = game.getLevel();
  int coinsEarned = (finalLevel * (finalLevel + 1)) / 2;
  pet.addCoins(coinsEarned);
  pet.addBoredom(coinsEarned);
  
  // Nota: Los efectos de play() ya se aplicaron al iniciar el juego
  // (hambre -15%, aburrimiento +20%, sueño -15%)
  
  // Mostrar animación HAPPY y dos pitidos agudos
  pet.showHappyFace = true;
  pet.happyFaceTimer = millis();
  playSound(1200, 1000);
  delay(50);
  playSound(1200, 1000);
  // Esperar 3 segundos para la animación
  unsigned long t0 = millis();
  while (millis() - t0 < 3000) {
    displayMgr.showMainScreen();
  }
  // Mostrar pantalla de fin de juego
  displayMgr.showGameOver(&game, coinsEarned);
  delay(2000);
}

void playSound(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration + 50);
}

void startMemoryGame() {
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
  playSound(1200, 1000);
  delay(50);
  playSound(1200, 1000);
  // Esperar 3 segundos para la animación
  unsigned long t0 = millis();
  while (millis() - t0 < 3000) {
    displayMgr.showMainScreen();
  }
  // Mostrar pantalla de fin de juego
  displayMgr.showMemoryGameOver(&memoryGame, coinsEarned);
  delay(2000);
}
