#include <Arduino.h>

#define PIN_START 2
#define PIN_END 13

enum class Square {
  Empty,
  X,
  O,
};

static const int pinToCoord[9][3] = {
  { 3, 0, 0 },
  { 4, 1, 0 },
  { 5, 2, 0 },
  { 2, 0, 1 },
  { 7, 1, 1 },
  { 6, 2, 1 },
  { 8, 0, 2 },
  { 10, 1, 2 },
  { 12, 2, 2 },
};

static bool lastPinStates[PIN_END];

struct TicTacToeBoard {
  Square board[3][3];
  Square turn = Square::X;

  void reset(void) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        board[x][y] = Square::Empty;
      }
    }
    turn = Square::X;

    Serial.print("PRINT BOARD.GCO\n");
  }

  bool checkVictory(int x, int y, int dx, int dy) {
    Square last;

    for (int i = 0; i < 3; i++) {
      if (board[x][y] == Square::Empty)
        return false;
  
      if (i == 0) {
        last = board[x][y];
      } else if (board[x][y] != last) {
        return false;
      }

      x += dx;
      y += dy;
    }

    return true;
  }

  bool checkDraw(void) {
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
        if (board[x][y] == Square::Empty)
          return false;
      }
    }
    return true;
  }

  bool checkAllVictory(void) {
    if (checkVictory(0, 0, 1, 0))
      return true;
    if (checkVictory(0, 1, 1, 0))
      return true;
    if (checkVictory(0, 2, 1, 0))
      return true;
    if (checkVictory(0, 0, 0, 1))
      return true;
    if (checkVictory(1, 0, 0, 1))
      return true;
    if (checkVictory(2, 0, 0, 1))
      return true;
    if (checkVictory(0, 0, 1, 1))
      return true;
    if (checkVictory(2, 0, -1, 1))
      return true;
    return false;
  }

  void mark(int x, int y) {
    if (x < 0 || x >= 3)
      return;
    if (y < 0 || y >= 3)
      return;
    if (board[x][y] != Square::Empty)
      return;

    board[x][y] = turn;
    Serial.print("PRINT ");
    Serial.print(turn == Square::X ? "X" : "O");
    Serial.print(x == 0 ? "L" : x == 1 ? "C" : "R");
    Serial.print(y == 0 ? "1" : y == 1 ? "2" : "3");
    Serial.print(".GCO\n");

    // TODO send command to printer

    if (turn == Square::X) {
      turn = Square::O;
    } else {
      turn = Square::X;
    }

    // TODO check for victory
    if (checkAllVictory() || checkDraw()) {
      Serial.print("Game has ended.\n");
      reset();
    }
  }
};

static TicTacToeBoard board;

void setup() {
  Serial.begin(9600);
  board.reset();
  for (int i = PIN_START; i < PIN_END; i++) {
    pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  for (int i = PIN_START; i < PIN_END; i++) {
    bool newState = !digitalRead(i);
    if (newState && !lastPinStates[i]) {
      for (int j = 0; j < 9; j++) {
        if (pinToCoord[j][0] == i) {
          int x = pinToCoord[j][1];
          int y = pinToCoord[j][2];
          board.mark(x, y);
          for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
              switch (board.board[x][y]) {
                case Square::Empty:
                  Serial.print(" ");
                  break;
                case Square::X:
                  Serial.print("X");
                  break;
                case Square::O:
                  Serial.print("O");
                  break;
              }
            }
            Serial.print("\n");

          }
          break;
        }
      }
      Serial.print("Read pin ");
      Serial.print(i);
      Serial.print("\n");
    }
    lastPinStates[i] = newState;
  }
  delay(100);
}
