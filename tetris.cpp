/***************************
 * Автор: Вяткин Владислав *
 * Название: Тетрис        *
 * Дата: 19.12.2022        *
 ***************************/

//#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <stdio.h>
#include <cstring>

using namespace std;

const int fieldWidth = 20;
const int fieldHeight = 30;
const int screenWidth = fieldWidth * 2;
const int screenHeight = fieldHeight;

const char symbolFigure = 219;
const char symbolField = 176;
const char symbolFigureDown = 178;

typedef char screenMap[screenHeight][screenWidth];
typedef char fieldMap[fieldHeight][fieldWidth];

const int shapeWidth = 4;
const int shapeHeight = 4;
typedef char shape[shapeHeight][shapeWidth];

char* shapes[] = {
	(char*)".....**..**.....", // []
	(char*)"....****........", // I
	(char*)"....***..*......", // L
	(char*)".....***.*......", // T
	(char*)".....**.**......", // S
	(char*)"....**...**.....", // Z
};
const int shapesSize = sizeof(shapes) / sizeof(shapes[0]);

int score = 0;

void setCursorPosition(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class clScreen {
	void setEnd() {
		scr[screenHeight - 1][screenWidth - 1] = '\0';
	}
public:
	screenMap scr;
	clScreen() {
		clear();
	}
	void clear() {
		memset(scr, '.', sizeof(scr));
	}
	void show() {
		setCursorPosition(0, 0);
		setEnd();
		cout << scr[0];
	}
};

class clField {
public:
	fieldMap field;
	clField() {
		clear();
	}
	void clear() {
		memset(field, symbolField, sizeof(field));
	}
	void put(screenMap& scr);
	void destroy();
};

class clFigure {
	int x, y;
	shape vid;
	char turn;
	COORD coord[shapeHeight * shapeWidth];
	int coordCount;
	clField* field = 0;
public:
	clFigure() {
		memset(this, 0, sizeof(*this));
	}
	void fieldSet(clField* _field) {
		field = _field;
	}
	void shape(const char* _vid) {
		memcpy(vid, _vid, sizeof(vid));
	}
	void pos(int _x, int _y) {
		x = _x;
		y = _y;
		coordCalculating();
	}
	char turnGet() {
		return turn;
	}
	void turnSet(char _turn);
	void put(screenMap& scr);
	void put(fieldMap& field);
	bool move(int changeX, int changeY);
	int checkExitBoundaries();
private:
	void coordCalculating();
};

class game {
	clScreen screen;
	clField field;
	clFigure figure;
public:
	game();
	void playerControl();
	void move();
	void show();
};

game::game() {
	figure.fieldSet(&field);
	figure.shape(shapes[rand() % shapesSize]);
	figure.pos(fieldWidth / 2 - shapeWidth / 2, 0);
}

void game::playerControl() {
	static int turn = 0;
	if (GetKeyState(' ') < 0) {
		turn += 1;
	}
	if (turn == 1) {
		figure.turnSet(figure.turnGet() + 1);
		++turn;
	}
	if (GetKeyState(' ') >= 0) {
		turn = 0;
	}

	if (GetKeyState('S') < 0) {
		figure.move(0, 1);
	}
	if (GetKeyState('A') < 0) {
		figure.move(-1, 0);
	}
	if (GetKeyState('D') < 0) {
		figure.move(1, 0);
	}
}

void game::show() {
	screen.clear();
	field.put(screen.scr);
	figure.put(screen.scr);
	screen.show();
}

void game::move() {
	static int tick = 0;
	++tick;
	if (tick >= 5) {
		if (!figure.move(0, 1)) {
			figure.put(field.field);
			figure.shape(shapes[rand() % shapesSize]);
			figure.pos(fieldWidth / 2 - shapeWidth / 2, 0);
			if (figure.checkExitBoundaries() > 0) {
				field.clear();
			}
		}

		field.destroy();
		tick = 0;
	}
}

void clFigure::put(screenMap& scr) {
	for (int coordIndex = 0; coordIndex < coordCount; ++coordIndex) {
		scr[coord[coordIndex].Y][coord[coordIndex].X * 2] =
			scr[coord[coordIndex].Y][coord[coordIndex].X * 2 + 1] = symbolFigure;
	}
}

void clFigure::put(fieldMap& field) {
	for (int coordIndex = 0; coordIndex < coordCount; ++coordIndex) {
		field[coord[coordIndex].Y][coord[coordIndex].X] = symbolFigureDown;
	}
}

void clFigure::turnSet(char _turn) {
	int prevTurn = turn;

	if (_turn > 3) {
		turn = 0;
	}
	else if (_turn < 0) {
		turn = 3;
	}
	else {
		turn = _turn;
	}

	int check = checkExitBoundaries();

	if (check == 0) {
		return;
	}
	if (check == 1) {
		int coordSymbolX = x;
		int k;

		if (x > (fieldWidth / 2)) {
			k = -1;
		}
		else {
			k = 1;
		}

		for (int turnIndex = 1; turnIndex < 3; ++turnIndex) {
			x += k;
			if (checkExitBoundaries() == 0) {
				return;
			}

			x = coordSymbolX;
		}
	}
	turn = prevTurn;
	coordCalculating();
}

bool clFigure::move(int changeX, int changeY) {
	int prevX = x, prevY = y;
	pos(x + changeX, y + changeY);

	int check = checkExitBoundaries();

	if (check == 2) {
		pos(prevX, prevY);
		return false; // приземлить фигуру и создать новую
	}
	return true;
}

int clFigure::checkExitBoundaries() {
	coordCalculating();

	for (int coordY = 0; coordY < coordCount; ++coordY) {
		if (coord[coordY].Y >= fieldHeight
			|| field->field[coord[coordY].Y][coord[coordY].X] == symbolFigureDown) {
			return 2;
		}
	}

	return 0;
}

void clFigure::coordCalculating() {
	int symbCoordX, symbCoordY;
	coordCount = 0;
	for (int coordX = 0; coordX < shapeWidth; ++coordX) {
		for (int coordY = 0; coordY < shapeHeight; ++coordY) {
			if (vid[coordY][coordX] == '*') {
				if (turn == 0) {
					symbCoordX = x + coordX;
					symbCoordY = y + coordY;
				}
				else if (turn == 1) {
					symbCoordX = x + (shapeHeight - coordY - 1);
					symbCoordY = y + coordX;
				}
				else if (turn == 2) {
					symbCoordX = x + (shapeWidth - coordX - 1);
					symbCoordY = y + (shapeHeight - coordY - 1);
				}
				else if (turn == 3) {
					symbCoordX = x + coordY;
					symbCoordY = y + (shapeHeight - coordX - 1) + (shapeWidth - shapeHeight);
				}
				if (symbCoordX >= fieldWidth) {
					coord[coordCount].X = symbCoordX - fieldWidth;
					coord[coordCount].Y = symbCoordY;
				} else if (symbCoordX < 0) {
					coord[coordCount].X = symbCoordX + fieldWidth;
					coord[coordCount].Y = symbCoordY;
				} else {
					coord[coordCount].X = symbCoordX;
					coord[coordCount].Y = symbCoordY;
				}

				++coordCount;
			}
		}
	}
}

void clField::put(screenMap& scr) {
	for (int coordX = 0; coordX < fieldWidth; ++coordX) {
		for (int coordY = 0; coordY < fieldHeight; ++coordY) {
			scr[coordY][coordX * 2] = scr[coordY][coordX * 2 + 1]
				= field[coordY][coordX];
		}
	}
}

void clField::destroy() {
	static bool fillLine;

	for (int coordY = fieldHeight - 1; coordY >= 0; --coordY) {
		fillLine = true;
		for (int coordX = 0; coordX < fieldWidth; ++coordX) {
			if (field[coordY][coordX] != symbolFigureDown) {
				fillLine = false;
			}
		}
		if (fillLine) {
			for (int y = coordY; y >= 1; --y) {
				memcpy(field[y], field[y - 1], sizeof(field[y]));
			}

			score += 100;
			return;
		}
	}
}

int main()
{
	char command[1000];
	sprintf_s(command, "mode con cols=%d lines=%d", screenWidth, screenHeight);
	system(command);
	srand(time(NULL));
	game game;

	while (true) {
		game.playerControl();
		game.move();
		game.show();
		if (GetKeyState(VK_ESCAPE) < 0) {

			break;
		}
		Sleep(100);
	}

	setlocale(LC_ALL, "RUS");
	printf("\t Вы набрали %d очков\n", score);
}
