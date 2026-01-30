/*
 * Wrapper para FluxGarage RoboEyes que evita conflictos de macros globales
 */

#ifndef _ROBOEYES_WRAPPER_H
#define _ROBOEYES_WRAPPER_H

#include <Adafruit_SSD1306.h>

// Guardia para evitar conflictos de macros con RoboEyes
#pragma push_macro("BGCOLOR")
#pragma push_macro("MAINCOLOR")
#pragma push_macro("DEFAULT")
#pragma push_macro("TIRED")
#pragma push_macro("ANGRY")
#pragma push_macro("HAPPY")
#pragma push_macro("ON")
#pragma push_macro("OFF")
#undef BGCOLOR
#undef MAINCOLOR
#undef DEFAULT
#undef TIRED
#undef ANGRY
#undef HAPPY
#undef ON
#undef OFF

#include <FluxGarage_RoboEyes.h>

#pragma pop_macro("OFF")
#pragma pop_macro("ON")
#pragma pop_macro("HAPPY")
#pragma pop_macro("ANGRY")
#pragma pop_macro("TIRED")
#pragma pop_macro("DEFAULT")
#pragma pop_macro("MAINCOLOR")
#pragma pop_macro("BGCOLOR")

#endif
