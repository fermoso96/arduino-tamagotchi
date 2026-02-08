#include "tictactoe.h"

TicTacToeGame::TicTacToeGame() {
  wins = 0;
  draws = 0;
  losses = 0;
  reset();
}

void TicTacToeGame::initialize() {
  prefs.begin("tictactoe", false);
  loadStats();
  reset();
}

void TicTacToeGame::loadStats() {
  wins = prefs.getInt("wins", 0);
  draws = prefs.getInt("draws", 0);
  losses = prefs.getInt("losses", 0);
}

void TicTacToeGame::saveStats() {
  prefs.putInt("wins", wins);
  prefs.putInt("draws", draws);
  prefs.putInt("losses", losses);
}

void TicTacToeGame::updateStats(GameResult gameResult) {
  if (gameResult == RESULT_PLAYER_WIN) {
    wins++;
  } else if (gameResult == RESULT_TAMAGOTCHI_WIN) {
    losses++;
  } else if (gameResult == RESULT_DRAW) {
    draws++;
  }
  saveStats();
}

void TicTacToeGame::reset() {
  // Limpiar tablero
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      board[y][x] = CELL_EMPTY;
    }
  }
  
  // Posición inicial del cursor
  cursorX = 1;
  cursorY = 1;
  
  // Decidir quién empieza aleatoriamente
  playerFirst = random(0, 2) == 0;
  
  movesCount = 0;
  result = RESULT_NONE;
  
  // Estado inicial
  if (playerFirst) {
    state = TIC_PLAYER_TURN;
  } else {
    state = TIC_TAMAGOTCHI_TURN;
  }
}

void TicTacToeGame::update() {
  if (state == TIC_TAMAGOTCHI_TURN) {
    // Pequeña pausa para que no sea instantáneo
    delay(500);
    tamagotchiMove();
    
    // Comprobar si hay ganador o empate
    result = checkGameOver();
    if (result != RESULT_NONE) {
      state = TIC_GAME_OVER;
    } else {
      state = TIC_PLAYER_TURN;
    }
  }
}

void TicTacToeGame::moveCursor() {
  // Buscar la siguiente posición vacía
  int attempts = 0;
  do {
    cursorX++;
    if (cursorX > 2) {
      cursorX = 0;
      cursorY++;
      if (cursorY > 2) {
        cursorY = 0;
      }
    }
    attempts++;
  } while (board[cursorY][cursorX] != CELL_EMPTY && attempts < 9);
}

bool TicTacToeGame::tryPlacePiece() {
  if (state != TIC_PLAYER_TURN) return false;
  if (board[cursorY][cursorX] != CELL_EMPTY) return false;
  
  // Colocar la ficha del jugador
  board[cursorY][cursorX] = CELL_PLAYER;
  movesCount++;
  
  // Comprobar si hay ganador o empate
  result = checkGameOver();
  if (result != RESULT_NONE) {
    state = TIC_GAME_OVER;
  } else {
    state = TIC_TAMAGOTCHI_TURN;
  }
  
  return true;
}

void TicTacToeGame::tamagotchiMove() {
  // Primero intenta ganar
  if (tryToWin()) {
    movesCount++;
    return;
  }
  
  // Luego intenta bloquear al jugador (85% de las veces)
  int blockChance = random(0, 100);
  if (blockChance < 85) {
    if (tryToBlock()) {
      movesCount++;
      return;
    }
  }
  
  // Si no puede ganar ni debe bloquear, hace un movimiento aleatorio
  randomMove();
  movesCount++;
}

bool TicTacToeGame::tryToWin() {
  // Comprobar todas las filas, columnas y diagonales
  // para ver si el Tamagotchi puede ganar en este turno
  
  // Comprobar filas
  for (int y = 0; y < 3; y++) {
    int tamagotchiCount = 0;
    int emptyCount = 0;
    int emptyX = -1;
    
    for (int x = 0; x < 3; x++) {
      if (board[y][x] == CELL_TAMAGOTCHI) tamagotchiCount++;
      else if (board[y][x] == CELL_EMPTY) {
        emptyCount++;
        emptyX = x;
      }
    }
    
    if (tamagotchiCount == 2 && emptyCount == 1) {
      board[y][emptyX] = CELL_TAMAGOTCHI;
      return true;
    }
  }
  
  // Comprobar columnas
  for (int x = 0; x < 3; x++) {
    int tamagotchiCount = 0;
    int emptyCount = 0;
    int emptyY = -1;
    
    for (int y = 0; y < 3; y++) {
      if (board[y][x] == CELL_TAMAGOTCHI) tamagotchiCount++;
      else if (board[y][x] == CELL_EMPTY) {
        emptyCount++;
        emptyY = y;
      }
    }
    
    if (tamagotchiCount == 2 && emptyCount == 1) {
      board[emptyY][x] = CELL_TAMAGOTCHI;
      return true;
    }
  }
  
  // Comprobar diagonal principal (0,0 -> 2,2)
  int tamagotchiCount = 0;
  int emptyCount = 0;
  int emptyX = -1, emptyY = -1;
  
  for (int i = 0; i < 3; i++) {
    if (board[i][i] == CELL_TAMAGOTCHI) tamagotchiCount++;
    else if (board[i][i] == CELL_EMPTY) {
      emptyCount++;
      emptyX = i;
      emptyY = i;
    }
  }
  
  if (tamagotchiCount == 2 && emptyCount == 1) {
    board[emptyY][emptyX] = CELL_TAMAGOTCHI;
    return true;
  }
  
  // Comprobar diagonal secundaria (0,2 -> 2,0)
  tamagotchiCount = 0;
  emptyCount = 0;
  
  for (int i = 0; i < 3; i++) {
    if (board[i][2-i] == CELL_TAMAGOTCHI) tamagotchiCount++;
    else if (board[i][2-i] == CELL_EMPTY) {
      emptyCount++;
      emptyX = 2-i;
      emptyY = i;
    }
  }
  
  if (tamagotchiCount == 2 && emptyCount == 1) {
    board[emptyY][emptyX] = CELL_TAMAGOTCHI;
    return true;
  }
  
  return false;
}

bool TicTacToeGame::tryToBlock() {
  // Similar a tryToWin pero buscando 2 fichas del jugador
  
  // Comprobar filas
  for (int y = 0; y < 3; y++) {
    int playerCount = 0;
    int emptyCount = 0;
    int emptyX = -1;
    
    for (int x = 0; x < 3; x++) {
      if (board[y][x] == CELL_PLAYER) playerCount++;
      else if (board[y][x] == CELL_EMPTY) {
        emptyCount++;
        emptyX = x;
      }
    }
    
    if (playerCount == 2 && emptyCount == 1) {
      board[y][emptyX] = CELL_TAMAGOTCHI;
      return true;
    }
  }
  
  // Comprobar columnas
  for (int x = 0; x < 3; x++) {
    int playerCount = 0;
    int emptyCount = 0;
    int emptyY = -1;
    
    for (int y = 0; y < 3; y++) {
      if (board[y][x] == CELL_PLAYER) playerCount++;
      else if (board[y][x] == CELL_EMPTY) {
        emptyCount++;
        emptyY = y;
      }
    }
    
    if (playerCount == 2 && emptyCount == 1) {
      board[emptyY][x] = CELL_TAMAGOTCHI;
      return true;
    }
  }
  
  // Comprobar diagonal principal
  int playerCount = 0;
  int emptyCount = 0;
  int emptyX = -1, emptyY = -1;
  
  for (int i = 0; i < 3; i++) {
    if (board[i][i] == CELL_PLAYER) playerCount++;
    else if (board[i][i] == CELL_EMPTY) {
      emptyCount++;
      emptyX = i;
      emptyY = i;
    }
  }
  
  if (playerCount == 2 && emptyCount == 1) {
    board[emptyY][emptyX] = CELL_TAMAGOTCHI;
    return true;
  }
  
  // Comprobar diagonal secundaria
  playerCount = 0;
  emptyCount = 0;
  
  for (int i = 0; i < 3; i++) {
    if (board[i][2-i] == CELL_PLAYER) playerCount++;
    else if (board[i][2-i] == CELL_EMPTY) {
      emptyCount++;
      emptyX = 2-i;
      emptyY = i;
    }
  }
  
  if (playerCount == 2 && emptyCount == 1) {
    board[emptyY][emptyX] = CELL_TAMAGOTCHI;
    return true;
  }
  
  return false;
}

void TicTacToeGame::randomMove() {
  // Buscar todas las posiciones vacías
  int emptyPositions[9][2];
  int emptyCount = 0;
  
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (board[y][x] == CELL_EMPTY) {
        emptyPositions[emptyCount][0] = x;
        emptyPositions[emptyCount][1] = y;
        emptyCount++;
      }
    }
  }
  
  // Elegir una posición aleatoria
  if (emptyCount > 0) {
    int choice = random(0, emptyCount);
    int x = emptyPositions[choice][0];
    int y = emptyPositions[choice][1];
    board[y][x] = CELL_TAMAGOTCHI;
  }
}

bool TicTacToeGame::checkWinner(int player) {
  // Comprobar filas
  for (int y = 0; y < 3; y++) {
    if (board[y][0] == player && board[y][1] == player && board[y][2] == player) {
      return true;
    }
  }
  
  // Comprobar columnas
  for (int x = 0; x < 3; x++) {
    if (board[0][x] == player && board[1][x] == player && board[2][x] == player) {
      return true;
    }
  }
  
  // Comprobar diagonal principal
  if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
    return true;
  }
  
  // Comprobar diagonal secundaria
  if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
    return true;
  }
  
  return false;
}

bool TicTacToeGame::isBoardFull() {
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (board[y][x] == CELL_EMPTY) {
        return false;
      }
    }
  }
  return true;
}

GameResult TicTacToeGame::checkGameOver() {
  if (checkWinner(CELL_PLAYER)) {
    return RESULT_PLAYER_WIN;
  }
  
  if (checkWinner(CELL_TAMAGOTCHI)) {
    return RESULT_TAMAGOTCHI_WIN;
  }
  
  if (isBoardFull()) {
    return RESULT_DRAW;
  }
  
  return RESULT_NONE;
}

void TicTacToeGame::getBoard(int outBoard[3][3]) const {
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      outBoard[y][x] = board[y][x];
    }
  }
}
