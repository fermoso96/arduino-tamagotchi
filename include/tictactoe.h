#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <Arduino.h>
#include <Preferences.h>

// Estados del juego
enum TicTacToeState {
  TIC_IDLE,
  TIC_PLAYER_TURN,
  TIC_TAMAGOTCHI_TURN,
  TIC_GAME_OVER
};

// Contenido de cada celda
enum CellContent {
  CELL_EMPTY = 0,
  CELL_PLAYER = 1,      // X para el jugador
  CELL_TAMAGOTCHI = 2   // O para el Tamagotchi
};

// Resultado del juego
enum GameResult {
  RESULT_NONE,
  RESULT_PLAYER_WIN,
  RESULT_TAMAGOTCHI_WIN,
  RESULT_DRAW
};

class TicTacToeGame {
private:
  int board[3][3];              // Tablero 3x3 (0=vacío, 1=jugador, 2=tamagotchi)
  int cursorX, cursorY;         // Posición del cursor
  TicTacToeState state;
  GameResult result;
  bool playerFirst;             // true si el jugador empieza
  int movesCount;               // Contador de movimientos
  
  // Estadísticas
  int wins;                     // Victorias del jugador
  int draws;                    // Empates
  int losses;                   // Derrotas del jugador
  Preferences prefs;
  
  // Para IA del Tamagotchi
  void tamagotchiMove();
  bool tryToWin();              // Intenta ganar si puede
  bool tryToBlock();            // Intenta bloquear al jugador (85% de probabilidad)
  void randomMove();            // Movimiento aleatorio
  
  // Comprobaciones
  bool checkWinner(int player);
  bool isBoardFull();
  GameResult checkGameOver();
  
public:
  TicTacToeGame();
  void initialize();
  void reset();
  void update();
  void updateStats(GameResult gameResult);  // Actualizar estadísticas
  void loadStats();                          // Cargar estadísticas
  void saveStats();                          // Guardar estadísticas
  
  // Control del jugador
  void moveCursor();            // Mover cursor a la siguiente posición vacía
  bool tryPlacePiece();         // Intentar colocar ficha (retorna true si se colocó)
  
  // Getters
  TicTacToeState getState() const { return state; }
  GameResult getResult() const { return result; }
  int getCellContent(int x, int y) const { return board[y][x]; }
  int getCursorX() const { return cursorX; }
  int getCursorY() const { return cursorY; }
  bool isPlayerFirst() const { return playerFirst; }
  int getWins() const { return wins; }
  int getDraws() const { return draws; }
  int getLosses() const { return losses; }
  
  // Para display
  void getBoard(int outBoard[3][3]) const;
};

#endif
