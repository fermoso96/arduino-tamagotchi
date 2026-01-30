#ifndef EYES_H
#define EYES_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <RoboEyesWrapper.h>

class EyesManager {
private:
  Adafruit_SSD1306* display;
  RoboEyes<Adafruit_SSD1306>* eyes;
  
  unsigned long lastBlinkTime;
  unsigned long blinkInterval;
  bool isBlinking;
  int mood; // 0=normal, 1=tired, 2=angry, 3=happy, 4=sleepy, 5=sad
  
public:
  EyesManager();
  void initialize(Adafruit_SSD1306* disp);
  
  void update();
  void drawEyesAnimated();
  void setMood(int newMood);
  
  void setHappy();
  void setSad();
  void setNormal();
  void setSleep();
  void setBlink();
  void setAwake();
  
private:
  void drawEyes();
};

#endif
