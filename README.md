# Arduino Tamagotchi

Un Tamagotchi interactivo con ESP32 C3 Mini, display OLED y un juego de esquivar obstáculos.

## Hardware

- **Microcontrolador**: ESP32 C3 Mini
- **Display**: OLED 0.96" (128x64)
- **Buzzer**: Para sonidos
- **Botones**: 3 botones de control (Conectar un pin al GPIO y el opuesto a GND)
   - BTN_LEFT (GPIO 0): Navegar Izquierda
   - BTN_ENTER (GPIO 1): Aceptar / Acción
   - BTN_RIGHT (GPIO 2): Navegar Derecha

## Características

### Estados del Tamagotchi

- **Normal**: Estado saludable
- **Hambriento**: Cuando el hambre > 70
- **Aburrido**: Cuando el aburrimiento > 70
- **Cansado**: Cuando el cansancio > 70
- **Durmiendo**: Duerme 20 minutos automáticamente si está muy cansado

### Mecánicas de Cuidado

- **Alimentar**: Reduce hambre, aumenta salud
- **Jugar**: Reduce aburrimiento, aumenta cansancio, aumenta salud
- **Dormir**: Restaura el cansancio y reduce el aburrimiento

### Juego de Nave

- Esquiva obstáculos en 3 carriles
- Botones izquierda/derecha para movimiento
- Aumenta velocidad con cada nivel
- Gana puntos por esquivar
- Monedas = puntos / 10

## Instalación

1. Instala PlatformIO en VS Code
2. Clona este repositorio
3. Abre la carpeta en VS Code
4. Conecta tu ESP32 C3 Mini
5. Ejecuta `PlatformIO: Upload`

## Esquema de Pines

```
ESP32 C3 Mini
├─ GPIO 0  → BTN_LEFT
├─ GPIO 1  → BTN_ENTER
├─ GPIO 2  → BTN_RIGHT
├─ GPIO 5  → BUZZER (+) | GND (-)
├─ GPIO 8  → I2C SDA (Display)
└─ GPIO 9  → I2C SCL (Display)
```

## Estructura del Proyecto

```
arduino-tamagotchi/
├── src/
│   ├── main.cpp          # Programa principal
│   ├── tamagotchi.cpp    # Lógica del Tamagotchi
│   ├── game.cpp          # Lógica del juego de nave
│   └── display.cpp       # Gestión de pantalla OLED
├── include/
│   ├── tamagotchi.h      # Header del Tamagotchi
│   ├── game.h            # Header del juego
│   └── display.h         # Header del display
├── platformio.ini        # Configuración de PlatformIO
└── README.md             # Este archivo
```

## Próximas Mejoras

- [ ] Integrar librería FluxGarage RoboEyes para animación de ojos
- [ ] Agregar más juegos
- [ ] Sistema de tienda para comprar items
- [ ] Evolución del tamagotchi
- [ ] Guardado de progreso en EEPROM
