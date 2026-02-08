# Arduino Tamagotchi

Un Tamagotchi interactivo con ESP32 C3 Mini, display OLED y 4 juegos diferentes para mantener a tu mascota virtual feliz y entretenida.

## Hardware

- **Microcontrolador**: ESP32 C3 Mini
- **Display**: OLED 0.96" (128x64, comunicación I2C)
- **Buzzer**: Buzzer pasivo para sonidos
- **Botón**: 1 botón pushbutton de control (Conectar un pin al GPIO y el opuesto a GND)
   - BTN_ENTER (GPIO 1): Navegación, selección y acciones

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

### Juegos Disponibles

#### 1. Juego de Esquivar (Dodge Game)

- Esquiva obstáculos en 2 carriles (central e inferior)
- **Control**: Botón central para cambiar de carril
- Aumenta velocidad y dificultad con cada nivel
- Gana monedas según tu puntuación
- Se guarda tu récord máximo

#### 2. Juego de Memoria (Memory Game)

- Memoriza secuencias de puntos y rayas (estilo código Morse)
- **Control**:
   - Pulsación corta = Punto (·)
   - Pulsación larga = Raya (−)
- La secuencia aumenta de longitud con cada nivel
- Perfecto para entrenar tu memoria

#### 3. Tres en Raya (Tic Tac Toe)

- Juego clásico contra el Tamagotchi
- **Control**:
   - Pulsación corta = Mover cursor
   - Pulsación larga = Seleccionar casilla
- Guarda estadísticas de victorias/derrotas/empates
- La IA del Tamagotchi mejora con el tiempo

## Instalación

1. Instala PlatformIO en VS Code
2. Clona este repositorio
3. Abre la carpeta en VS Code
4. Conecta tu ESP32 C3 Mini
5. Ejecuta `PlatformIO: Upload`

## Esquema de Pines

```
ESP32 C3 Mini
├─ GPIO 1  → BTN_ENTER (Botón pushbutton, el otro pin a GND)
├─ GPIO 5  → BUZZER (Pin positivo del buzzer pasivo, negativo a GND)
├─ GPIO 8  → I2C SDA (Display OLED)
└─ GPIO 9  → I2C SCL (Display OLED)

Display OLED 0.96" (128x64)
├─ VCC → 3.3V (ESP32)
├─ GND → GND (ESP32)
├─ SDA → GPIO 8 (ESP32)
└─ SCL → GPIO 9 (ESP32)

Buzzer Pasivo
├─ Positivo (+) → GPIO 5 (ESP32)
└─ Negativo (-) → GND (ESP32)

Botón Pushbutton
├─ Pin 1 → GPIO 1 (ESP32)
└─ Pin 2 → GND (ESP32)
```

**Nota**: El ESP32 C3 Mini tiene resistencias pull-up internas habilitadas en el botón, por lo que no se requiere resistencia externa.

## Estructura del Proyecto

```
arduino-tamagotchi/
├── src/
│   ├── main.cpp          # Programa principal y manejo de entrada
│   ├── tamagotchi.cpp    # Lógica del Tamagotchi (estados, salud)
│   ├── display.cpp       # Gestión de pantalla OLED
│   ├── eyes.cpp          # Animación de ojos con RoboEyes
│   ├── game.cpp          # Juego de esquivar obstáculos
│   ├── memorygame.cpp    # Juego de memoria (morse)
│   ├── tapgame.cpp       # Juego de tocar objetivos
│   └── tictactoe.cpp     # Juego de tres en raya
├── include/
│   ├── tamagotchi.h      # Header del Tamagotchi
│   ├── display.h         # Header del display
│   ├── eyes.h            # Header de animación de ojos
│   ├── game.h            # Header del juego de esquivar
│   ├── memorygame.h      # Header del juego de memoria
│   ├── tapgame.h         # Header del juego de tocar
│   └── tictactoe.h       # Header del tres en raya
├── lib/
│   └── RoboEyes/         # Librería FluxGarage RoboEyes
├── platformio.ini        # Configuración de PlatformIO
└── README.md             # Este archivo
```

## Próximas Mejoras

- [x] Integrar librería FluxGarage RoboEyes para animación de ojos
- [x] Sistema con múltiples juegos (4 juegos implementados)
- [ ] Sistema de tienda mejorado con más items
- [ ] Evolución del tamagotchi con diferentes formas
- [ ] Más animaciones y expresiones
- [ ] Sonidos mejorados y melodías variadas
