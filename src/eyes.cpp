#include "eyes.h"

EyesManager::EyesManager() {
  display = nullptr;
  eyes = nullptr;
  lastBlinkTime = 0;
  blinkInterval = 3000;
  isBlinking = false;
  mood = 0; // normal
}

void EyesManager::initialize(Adafruit_SSD1306* disp) {
  display = disp;
  // Crear ojos RoboEyes con referencia al display
  eyes = new RoboEyes<Adafruit_SSD1306>(*disp);
  // Inicializar RoboEyes
  // Nota: begin() hace clearDisplay() internamente
  eyes->begin(128, 64, 120);  // width, height, 120 FPS (animación mucho más rápida)
  eyes->setDisplayColors(0, 1);  // background=0, main=1
  // Configuración estable con animaciones suaves
  eyes->setAutoblinker(true, 5, 2); // parpadeo automático cada 3-7 segundos
  eyes->setIdleMode(true, 10, 5);    // movimiento idle cada 5-15 segundos
  eyes->setCuriosity(true);
  eyes->setCyclops(false);
  eyes->setMood(0); // DEFAULT (neutral)
  eyes->setPosition(DEFAULT);
  eyes->setWidth(36, 36);
  eyes->setHeight(36, 36);
  eyes->setBorderradius(8, 8);
  eyes->setSpacebetween(10);
  // Asegurar ojos abiertos desde el inicio
  eyes->open();
  eyes->eyeLheightCurrent = eyes->eyeLheightDefault;
  eyes->eyeRheightCurrent = eyes->eyeRheightDefault;
  eyes->eyeLheightNext = eyes->eyeLheightDefault;
  eyes->eyeRheightNext = eyes->eyeRheightDefault;
  // Dibujar un frame inicial
  eyes->drawEyes();
  display->display();
  lastBlinkTime = millis();
}

void EyesManager::update() {
  unsigned long currentTime = millis();
  if (currentTime - lastBlinkTime >= blinkInterval) {
    isBlinking = !isBlinking;
    lastBlinkTime = currentTime;
    if (isBlinking) {
      blinkInterval = 150;
      log_i("Blink ON");
    } else {
      blinkInterval = 3000 + random(2000);
      log_i("Blink OFF");
    }
  }
  drawEyes();
}

void EyesManager::drawEyesAnimated() {
  if (display == nullptr || eyes == nullptr) return;

  // RoboEyes gestiona el frame rate internamente
  eyes->update();
}

void EyesManager::setMood(int newMood) {
  if (newMood != mood) {
    mood = newMood;
    if (newMood >= 0 && newMood <= 4) {
      eyes->setMood(newMood);
    }
    log_i("Eyes mood changed to: %d", newMood);
  }
}

void EyesManager::drawEyes() {
  if (display == nullptr || eyes == nullptr) return;
  
  unsigned long currentTime = millis();
  
  // Control de parpadeo
  if (currentTime - lastBlinkTime >= blinkInterval) {
    isBlinking = !isBlinking;
    lastBlinkTime = currentTime;
    if (isBlinking) {
      blinkInterval = 60;  // Duración del parpadeo
      eyes->eyeL_open = false;
      eyes->eyeR_open = false;
    } else {
      blinkInterval = 3000 + random(2000);  // Intervalo hasta próximo parpadeo
      eyes->eyeL_open = true;
      eyes->eyeR_open = true;
    }
  }
  
  // Aplicar mood/expresión
  switch(mood) {
    case 1:  // TIRED = DEFAULT con ojos cerrados
      eyes->tired = false;
      eyes->sleepy = false;
      eyes->happy = false;
      eyes->angry = false;
      eyes->curious = false;
      eyes->eyeL_open = false;
      eyes->eyeR_open = false;
      break;
    case 2:  // ANGRY
      eyes->angry = true;
      eyes->happy = false;
      eyes->tired = false;
      eyes->sleepy = false;
      break;
    case 3:  // HAPPY
      eyes->happy = true;
      eyes->angry = false;
      eyes->tired = false;
      eyes->sleepy = false;
      eyes->curious = true;
      break;
    case 4:  // SLEEPY (50% cerrado)
      eyes->sleepy = true;
      eyes->angry = false;
      eyes->happy = false;
      eyes->tired = false;
      eyes->eyeL_open = true;
      eyes->eyeR_open = true;
      eyes->curious = false;
      break;
    case 5:  // SAD
      eyes->angry = false;
      eyes->happy = false;
      eyes->tired = true;
      eyes->sleepy = false;
      eyes->eyeL_open = true;
      eyes->eyeR_open = true;
      eyes->curious = false;
      break;
    default:  // NORMAL
      eyes->happy = false;
      eyes->angry = false;
      eyes->tired = false;
      eyes->sleepy = false;
      eyes->curious = false;
      break;
  }
}

void EyesManager::setHappy() {
  mood = 3;
}

void EyesManager::setSad() {
  mood = 5;
}

void EyesManager::setNormal() {
  mood = 0;
}

void EyesManager::setSleep() {
  mood = 3;
  isBlinking = true;
}

void EyesManager::setBlink() {
  isBlinking = true;
}

void EyesManager::setAwake() {
  mood = 0;
  isBlinking = false;
}
