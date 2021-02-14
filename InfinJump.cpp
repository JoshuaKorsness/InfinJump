#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <Windows.h>
#include <chrono>
#include <cmath>
using namespace std::chrono_literals;	//So we can use "ms" in timing

int nScreenWidth = 80;     // Console screen width X (columns)
int nScreenHeight = 30;    // Console screen height Y (rows)
int nFieldWidth = 80;      // Gameplay field width
int nFieldHeight = 30;     // Gameplay field height
std::wstring charModel;
std::wstring blockModels[3];	// Array to hold block models
unsigned char *pField = nullptr;

int main()
{
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Fill in model variables
	charModel = L"OOOOOOOOO";	// 3x3
	blockModels[0].append(L"X....X....X....X....X....");	// 5x5
	blockModels[1].append(L"XX...XX...XX...XX...XX...");	// 5x5
	blockModels[2].append(L"XXXXXXXXXXXXXXXXXXXXXXXXX");	// 5x5

	pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = 11;
		}
	}

	// Game logic setup
	bool bKey[4];	// Storing key states
	int nCurrentX = nFieldWidth / 2 - 2;	// Starting x pos of character
	int nCurrentY = nFieldHeight - 3;		// Starting y pos of character
	int nBlockX = nFieldWidth;
	int nBlockY = nFieldHeight - 5;
	int nCurrentPiece = 0;
	bool bGameOver = false;					// Main game loop trigger
	double jumpCounter = 0;					// Used for jumping math
	double yVel = 0;
	double grav = 1;
	double timeInt = .5;
	double speedLim = 1.5;

	while (!bGameOver) {	// Main game loop

		// Timing =======================
		std::this_thread::sleep_for(25ms);

		// Input ========================
		for (int k = 0; k < 4; k++) {							 //R    L   U   D
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x26\x28"[k]))) != 0;
		}

		// Game Logic ===================
		if (bKey[0] == 1 && (nCurrentX + 4) % nFieldWidth != 0) {	// Allow movement if key pressed and in boundaries
			nCurrentX += 2;
		}
		else if (bKey[1] == 1 && (nCurrentX + nFieldWidth) % nFieldWidth != 0) {
			nCurrentX -= 2;
		}
		if (bKey[2] == 1 && nCurrentY == nFieldHeight - 3) {
			yVel = -3;
			jumpCounter = 0;
		}
		
		yVel += grav * jumpCounter;
		//nCurrentY += std::round(yVel*timeInt);
		
		if (yVel < -1*speedLim) {
			nCurrentY -= speedLim;
		}
		else if (yVel > speedLim) {
			nCurrentY += speedLim;
		}
		else {
			nCurrentY += std::round(yVel * timeInt);
		}

		jumpCounter += .05;
		if (nCurrentY > nFieldHeight - 3) {
			nCurrentY = nFieldHeight - 3;
			yVel = 0;
		}

		// Move block
		nBlockX -= 1;
		if (nBlockX < 0) {
			nBlockX = nFieldWidth;
			nCurrentPiece = std::rand() % 3;
		}

		// CHeck for collision
		for (int px = 0; px < 3; px++) {
			for (int py = 0; py < 3; py++) {
				if (charModel[3 * py + px] == L'O') {
					for (int bx = 0; bx < 5; bx++) {
						for (int by = 0; by < 5; by++) {
							if (blockModels[nCurrentPiece][5 * by + bx] == L'X') {
								if ((nCurrentY + py) == (nBlockY + by) && (nCurrentX + px) == (nBlockX + bx)) {
									bGameOver = true;
								}
							}
						}
					}
				}
			}
		}

		// Display ======================
		
		// Draw field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[y * nScreenWidth + x] = L" ABCDEFGH=#"[pField[y * nFieldWidth + x]];
			}
		}

		//	Draw character
		for (int px = 0; px < 3; px++) {
			for (int py = 0; py < 3; py++) {
				if (charModel[3*py + px] != L'.') {
					screen[(nCurrentY + py) * nScreenWidth + (nCurrentX + px)] = charModel[3*py + px];
				}
			}
		}

		// Draw current block
		for (int px = 0; px < 5; px++) {
			for (int py = 0; py < 5; py++) {
				if (blockModels[nCurrentPiece][5 * py + px] != L'.') {
					screen[(nBlockY + py) * nScreenWidth + (nBlockX + px)] = blockModels[nCurrentPiece][5 * py + px];
				}
			}
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}

	// Shutdown
	CloseHandle(hConsole);
	std::cout << "Game over!";
	system("pause");
	return 0;
}