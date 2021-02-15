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
	int nBlockX [3] = { nFieldWidth, nFieldWidth + 30, nFieldWidth + 60 };
	int nBlockY[3] = { nFieldHeight - 5, nFieldHeight - 5, nFieldHeight - 5 };
	int nCurrentPiece [3] = { 0, 1, 2 };
	bool bGameOver = false;					// Main game loop trigger
	double jumpCounter = 0;					// Used for jumping math
	double yVel = 0;
	double grav = 1;
	double timeInt = .5;
	double speedLim = 1.5;
	int mainCount = 0;
	int blockSpeed = 1;
	int countSkip = 3;

	while (!bGameOver) {	// Main game loop

		// Timing =======================
		std::this_thread::sleep_for(25ms);
		mainCount++; 

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
		
		if (yVel < -1 * speedLim) {
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
		if (mainCount % 500 == 0 && countSkip == 1) {
			blockSpeed++; 
		}
		else if (mainCount % 500 == 0 && countSkip > 1) {
			countSkip -= 1;
		}

		if (mainCount % countSkip == 0) {	// Used to control block speed
			for (int i = 0; i < sizeof(nBlockX) / sizeof(nBlockX[0]); i++) {
				nBlockX[i] -= blockSpeed;
				if (nBlockX[i] < 0) {
					nCurrentPiece[i] = std::rand() % (sizeof(nBlockX) / sizeof(nBlockX[0]));
					nBlockX[i] = nFieldWidth + 10 + std::rand() % 20;
				}
			}
		}

		// Check for collision
		for (int px = 0; px < 3; px++) {
			for (int py = 0; py < 3; py++) {
				if (charModel[3 * py + px] == L'O') {
					for (int bx = 0; bx < 5; bx++) {
						for (int by = 0; by < 5; by++) {
							for (int i = 0; i < sizeof(nCurrentPiece) / sizeof(nCurrentPiece[0]); i++) {
								if (blockModels[nCurrentPiece[i]][5 * by + bx] == L'X') {
									if ((nCurrentY + py) == (nBlockY[i] + by) && (nCurrentX + px) == (nBlockX[i] + bx)) {
										bGameOver = true;
									}
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
				for (int i = 0; i < sizeof(blockModels) / sizeof(blockModels[0]); i++) {
					if (blockModels[nCurrentPiece[i]][5 * py + px] != L'.' && nBlockX[i] < nScreenWidth) {
						screen[(nBlockY[i] + py) * nScreenWidth + (nBlockX[i] + px)] = blockModels[nCurrentPiece[i]][5 * py + px];
					}
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