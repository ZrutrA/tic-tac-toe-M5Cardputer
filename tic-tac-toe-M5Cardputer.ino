#include <M5Cardputer.h>
#include <vector>
#include <SD.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define CELL_SIZE 40
#define GRID_OFFSET_X 60
#define GRID_OFFSET_Y 20

#define EMPTY 0
#define PLAYER_X 1
#define PLAYER_O 2

int board[3][3]; // Board array
int currentPlayer = PLAYER_O; // Start with player O
int cursorX = 0; // Cursor position
int cursorY = 0;
bool gameWon = false; // Game state

// Scores
int humanWins = 0;
int aiWins = 0;
int draws = 0;

// Winning line coordinates
int winLine[3][2];

// Track who starts the next game
bool humanStartsNext = true;
bool isFirstAIMove = true;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.fillScreen(TFT_BLACK);

  // Initialize board
  resetBoard();

  drawGrid();
  drawCursor();
}

void loop() {
  M5Cardputer.update();
  
  if (currentPlayer == PLAYER_O) {
    readButtons();
  } else {
    aiMove();
    delay(500); // Delay for AI move to simulate thinking
  }
}

void readButtons() {
  if (M5Cardputer.Keyboard.isKeyPressed('/')) { // Right
    cursorX = (cursorX + 1) % 3;
    updateDisplay();
    delay(200); // Debounce delay
  }
  if (M5Cardputer.Keyboard.isKeyPressed(';')) { // Up
    cursorY = (cursorY + 2) % 3; // +2 to emulate -1 mod 3
    updateDisplay();
    delay(200); // Debounce delay
  }
  if (M5Cardputer.Keyboard.isKeyPressed('.')) { // Down
    cursorY = (cursorY + 1) % 3;
    updateDisplay();
    delay(200); // Debounce delay
  }
  if (M5Cardputer.Keyboard.isKeyPressed(',')) { // Left
    cursorX = (cursorX + 2) % 3; // +2 to emulate -1 mod 3
    updateDisplay();
    delay(200); // Debounce delay
  }
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) { // Enter key
    makeMove();
    delay(200); // Debounce delay
  }
}

void drawGrid() {
  // Clear display
  M5Cardputer.Display.fillScreen(TFT_BLACK);

  // Draw scores
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(0, 0);
  M5Cardputer.Display.print("YOU:");
  M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5Cardputer.Display.print(humanWins);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.print(" AI:");
  M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5Cardputer.Display.print(aiWins);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.print(" Draws:");
  M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5Cardputer.Display.println(draws);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK); // Reset text color to white

  // Draw grid
  for (int i = 0; i < 4; i++) {
    M5Cardputer.Display.drawLine(GRID_OFFSET_X, GRID_OFFSET_Y + i * CELL_SIZE, GRID_OFFSET_X + 3 * CELL_SIZE, GRID_OFFSET_Y + i * CELL_SIZE, TFT_WHITE);
    M5Cardputer.Display.drawLine(GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y, GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y + 3 * CELL_SIZE, TFT_WHITE);
  }

  M5Cardputer.Display.display();
}

void drawCursor() {
  int x = cursorX * CELL_SIZE + GRID_OFFSET_X + CELL_SIZE / 2;
  int y = cursorY * CELL_SIZE + GRID_OFFSET_Y + CELL_SIZE / 2;
  int cursorSize = CELL_SIZE - 2;

  // Draw cursor as an empty yellow square with thicker lines
  M5Cardputer.Display.drawRect(x - cursorSize / 2, y - cursorSize / 2, cursorSize, cursorSize, TFT_YELLOW);
  M5Cardputer.Display.drawRect(x - cursorSize / 2 + 1, y - cursorSize / 2 + 1, cursorSize - 2, cursorSize - 2, TFT_YELLOW);
  M5Cardputer.Display.drawRect(x - cursorSize / 2 + 2, y - cursorSize / 2 + 2, cursorSize - 4, cursorSize - 4, TFT_YELLOW);

  M5Cardputer.Display.display();
}

void makeMove() {
  if (board[cursorX][cursorY] == EMPTY && !gameWon) {
    board[cursorX][cursorY] = currentPlayer;
    if (checkWin()) {
      gameWon = true;
      blinkWinningLine();
      if (currentPlayer == PLAYER_X) {
        aiWins++;
      } else {
        humanWins++;
      }
      humanStartsNext = !humanStartsNext;
      resetBoard();
      drawGrid();
      updateDisplay();
    } else if (isBoardFull()) {
      draws++;
      blinkDraw();
      humanStartsNext = !humanStartsNext;
      resetBoard();
      drawGrid();
      updateDisplay();
    } else {
      currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
      if (currentPlayer == PLAYER_X) {
        aiMove();
      }
    }
    updateDisplay();
  }
}

void aiMove() {
  if (gameWon) return;

  if (isFirstAIMove) {
    int x, y;
    do {
      x = random(0, 3);
      y = random(0, 3);
    } while (board[x][y] != EMPTY);
    cursorX = x;
    cursorY = y;
    isFirstAIMove = false;
  } else {
    int bestScore = -1000;
    int moveX = -1;
    int moveY = -1;

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board[i][j] == EMPTY) {
          board[i][j] = PLAYER_X;
          int score = minimax(board, 0, false);
          board[i][j] = EMPTY;
          if (score > bestScore) {
            bestScore = score;
            moveX = i;
            moveY = j;
          }
        }
      }
    }
    cursorX = moveX;
    cursorY = moveY;
  }
  
  board[cursorX][cursorY] = PLAYER_X;

  if (checkWin()) {
    gameWon = true;
    blinkWinningLine();
    aiWins++;
    humanStartsNext = !humanStartsNext;
    resetBoard();
    drawGrid();
  } else if (isBoardFull()) {
    draws++;
    blinkDraw();
    humanStartsNext = !humanStartsNext;
    resetBoard();
    drawGrid();
  } else {
    currentPlayer = PLAYER_O;
  }
  updateDisplay();
}

int minimax(int board[3][3], int depth, bool isMaximizing) {
  int result = evaluateBoard();
  if (result != 0) return result;
  if (isBoardFull()) return 0;

  if (isMaximizing) {
    int bestScore = -1000;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board[i][j] == EMPTY) {
          board[i][j] = PLAYER_X;
          int score = minimax(board, depth + 1, false);
          board[i][j] = EMPTY;
          bestScore = max(score, bestScore);
        }
      }
    }
    return bestScore;
  } else {
    int bestScore = 1000;
    for (int i = 0;  i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board[i][j] == EMPTY) {
          board[i][j] = PLAYER_O;
          int score = minimax(board, depth + 1, true);
          board[i][j] = EMPTY;
          bestScore = min(score, bestScore);
        }
      }
    }
    return bestScore;
  }
}

int evaluateBoard() {
  for (int i = 0; i < 3; i++) {
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
      if (board[i][0] == PLAYER_X) return 10;
      if (board[i][0] == PLAYER_O) return -10;
    }
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
      if (board[0][i] == PLAYER_X) return 10;
      if (board[0][i] == PLAYER_O) return -10;
    }
  }
  if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
    if (board[0][0] == PLAYER_X) return 10;
    if (board[0][0] == PLAYER_O) return -10;
  }
  if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
    if (board[0][2] == PLAYER_X) return 10;
    if (board[0][2] == PLAYER_O) return -10;
  }
  return 0;
}

void updateDisplay() {
  drawGrid();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      int x = i * CELL_SIZE + GRID_OFFSET_X + CELL_SIZE / 2;
      int y = j * CELL_SIZE + GRID_OFFSET_Y + CELL_SIZE / 2;

      if (board[i][j] == PLAYER_X) {
        M5Cardputer.Display.drawLine(x - 5, y - 5, x + 5, y + 5, TFT_GREEN);
        M5Cardputer.Display.drawLine(x + 5, y - 5, x - 5, y + 5, TFT_GREEN);
      } else if (board[i][j] == PLAYER_O) {
        M5Cardputer.Display.drawCircle(x, y, 5, TFT_RED);
      }
    }
  }
  drawCursor();
}

bool checkWin() {
  // Check rows and columns
  for (int i = 0; i < 3; i++) {
    if (board[i][0] == currentPlayer && board[i][1] == currentPlayer && board[i][2] == currentPlayer) {
      winLine[0][0] = i; winLine[0][1] = 0;
      winLine[1][0] = i; winLine[1][1] = 1;
      winLine[2][0] = i; winLine[2][1] = 2;
      return true;
    }
    if (board[0][i] == currentPlayer && board[1][i] == currentPlayer && board[2][i] == currentPlayer) {
      winLine[0][0] = 0; winLine[0][1] = i;
      winLine[1][0] = 1; winLine[1][1] = i;
      winLine[2][0] = 2; winLine[2][1] = i;
      return true;
    }
  }

  // Check diagonals
  if (board[0][0] == currentPlayer && board[1][1] == currentPlayer && board[2][2] == currentPlayer) {
    winLine[0][0] = 0; winLine[0][1] = 0;
    winLine[1][0] = 1; winLine[1][1] = 1;
    winLine[2][0] = 2; winLine[2][1] = 2;
    return true;
  }
  if (board[0][2] == currentPlayer && board[1][1] == currentPlayer && board[2][0] == currentPlayer) {
    winLine[0][0] = 0; winLine[0][1] = 2;
    winLine[1][0] = 1; winLine[1][1] = 1;
    winLine[2][0] = 2; winLine[2][1] = 0;
    return true;
  }

  return false;
}

bool isBoardFull() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (board[i][j] == EMPTY) {
        return false;
      }
    }
  }
  return true;
}

void blinkWinningLine() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      int x = winLine[j][0] * CELL_SIZE + GRID_OFFSET_X + CELL_SIZE / 2;
      int y = winLine[j][1] * CELL_SIZE + GRID_OFFSET_Y + CELL_SIZE / 2;

      if (board[winLine[j][0]][winLine[j][1]] == PLAYER_X) {
        M5Cardputer.Display.drawLine(x - 5, y - 5, x + 5, y + 5, i % 2 == 0 ? TFT_GREEN : TFT_BLACK);
        M5Cardputer.Display.drawLine(x + 5, y - 5, x - 5, y + 5, i % 2 == 0 ? TFT_GREEN : TFT_BLACK);
      } else {
        M5Cardputer.Display.drawCircle(x, y, 5, i % 2 == 0 ? TFT_RED : TFT_BLACK);
      }
    }
    M5Cardputer.Display.display();
    delay(500);
  }
}

void blinkDraw() {
  for (int i = 0; i < 5; i++) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        int posX = x * CELL_SIZE + GRID_OFFSET_X + CELL_SIZE / 2;
        int posY = y * CELL_SIZE + GRID_OFFSET_Y + CELL_SIZE / 2;

        if (board[x][y] == PLAYER_X) {
          M5Cardputer.Display.drawLine(posX - 5, posY - 5, posX + 5, posY + 5, i % 2 == 0 ? TFT_GREEN : TFT_BLACK);
          M5Cardputer.Display.drawLine(posX + 5, posY - 5, posX - 5, posY + 5, i % 2 == 0 ? TFT_GREEN : TFT_BLACK);
        } else if (board[x][y] == PLAYER_O) {
          M5Cardputer.Display.drawCircle(posX, posY, 5, i % 2 == 0 ? TFT_RED : TFT_BLACK);
        }
      }
    }
    M5Cardputer.Display.display();
    delay(500);
  }
}

void resetBoard() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      board[i][j] = EMPTY;
    }
  }
  currentPlayer = humanStartsNext ? PLAYER_O : PLAYER_X;
  cursorX = 0;
  cursorY = 0;
  gameWon = false;
  isFirstAIMove = true;
  if (currentPlayer == PLAYER_X) {
    aiMove(); // AI starts the game if it's AI's turn
  }
}
