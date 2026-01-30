# Arduino Tamagotchi - Instrucciones para Copilot

## Descripción del Proyecto
Tamagotchi interactivo con ESP32 C3 Mini que incluye:
- Gestión de estados del mascota (hambre, aburrimiento, cansancio, sueño)
- Juego de nave esquivando obstáculos en 3 carriles
- Sistema de monedas basado en puntos
- Display OLED 0.96" (128x64)
- Buzzer para retroalimentación de audio
- 5 botones de control

## Hardware Disponible
- ESP32 C3 Mini
- Display OLED 0.96" (comunicación I2C)
- Buzzer pasivo
- 5 botones pushbutton

## Estructura del Código
- `src/main.cpp`: Loop principal, lectura de botones, gestión de estados
- `src/tamagotchi.cpp`: Lógica del Tamagotchi (estados, actualizaciones)
- `src/game.cpp`: Lógica del juego de nave (obstáculos, colisiones, puntuación)
- `src/display.cpp`: Renderizado en pantalla OLED

## Próximas Tareas
- [ ] Crear carpeta del proyecto en VS Code
- [ ] Compilar y verificar errores
- [ ] Probar en hardware
- [ ] Integrar librería FluxGarage RoboEyes
- [ ] Implementar iconos adicionales (hambre, sueño)
- [ ] Mejorar gráficos del juego

## Notas de Desarrollo
- Usar delays mínimos para no bloquear el loop principal
- Verificar pines GPIO antes de subir a hardware
- Optimizar consumo de memoria en ESP32
