// Infinte Jump Game
// Jump over incoming blocks using your keyboard's arrow keys. When a block clears the screen, you get a point.
// Must be playe don an 80x30 command prompt

#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <Windows.h>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;	//So we can use "ms" in timing

int nScreenWidth = 80;				// Console screen width X (columns)
int nScreenHeight = 30;				// Console screen height Y (rows)
int nFieldWidth = 80;				// Gameplay field width
int nFieldHeight = 30;				// Gameplay field height
std::wstring charModel;				// Varible for holding character model
std::wstring blockModels[8];		// Array to hold block models
unsigned char *pField = nullptr;	// Initialize field 

int main()
{
	// Create Screen Buffer. Allows us to treat the console like an array of pixels. 
	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Fill in model variables
	charModel = L"OOOOOOOOO";								// 3x3
	blockModels[0].append(L"X....X....X....X....X....");	// 5x5
	blockModels[1].append(L"XX...XX...XX...XX...XX...");	
	blockModels[2].append(L"XXXXXXXXXXXXXXXXXXXXXXXXX");	
	blockModels[3].append(L"XX.XXXXX...XXXXXXXXXX.XX."); 
	blockModels[4].append(L"XX.XXXXX...XXXXXX........");
	blockModels[5].append(L"........................X");
	blockModels[6].append(L"XXXXX....................");
	blockModels[7].append(L"XXXXXXXXXX...............");

	pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) {					// Fill the play field with blank spaces
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = 11;
		}
	}

	// Game logic setup
	bool bKey[4];							// Storing key states
	int nCurrentX = nFieldWidth / 2 - 2;	// Starting x pos of character
	int nCurrentY = nFieldHeight - 3;		// Starting y pos of character
	int nBlockX [3] = { nFieldWidth, nFieldWidth + 30, nFieldWidth + 60 };		// Starting xposition of blocks
	int nBlockY[3] = { nFieldHeight - 5, nFieldHeight - 5, nFieldHeight - 5 };	// Starting y position of blocks
	int nCurrentPiece [3] = { 0, 1, 2 };	// Array selecting initial 3 blocks
	bool bGameOver = false;					// Main game loop trigger
	int mainCount = 0;						// Game loop counter, used for timing
	int blockSpeed = 1;						// Initial x speed of blocks
	int countSkip = 3;						// Apply x speed to blocks every 3 counts. Decreases to increase block speed.
	int score = 0;							// Score variable
	
	double jumpCounter = 0;					// Used for jumping math
	double yVel = 0;						// Initial jump velocity
	double grav = 1;						// Gravity variable used to adjust jumping physics
	double timeInt = .5;					// Time velocity is applied
	double speedLim = 1.5;					// Max jumping/falling speed

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
		if (bKey[2] == 1 && nCurrentY == nFieldHeight - 3) {		// Only allow jump if on the ground
			yVel = -3;
			jumpCounter = 0;
		}
		
		yVel += grav * jumpCounter;	// Adjust y velocity. If not in jump loop, jumpCounter = 0 and yVel is 0. 
		
		// Adjust position based off yVel and timeInt. Cap at speed limits. 
		if (yVel < -1 * speedLim) {
			nCurrentY -= speedLim;
		}
		else if (yVel > speedLim) {
			nCurrentY += speedLim;
		}
		else {
			nCurrentY += std::round(yVel * timeInt);
		}

		jumpCounter += .05;	// Increment jumpCounter

		// Set character to ground if trying to go below the screen
		if (nCurrentY > nFieldHeight - 3) {
			nCurrentY = nFieldHeight - 3;
			yVel = 0;
		}

		// Adjust block speed
		if (mainCount % 500 == 0 && countSkip == 1) {
			blockSpeed++; 
		}
		else if (mainCount % 500 == 0 && countSkip > 1) {
			countSkip -= 1;
		}

		// Move blocks
		if (mainCount % countSkip == 0) {	// Used to control block speed
			for (int i = 0; i < sizeof(nBlockX) / sizeof(nBlockX[0]); i++) {
				nBlockX[i] -= blockSpeed;
				if (nBlockX[i] < 0) {	// Spawn new block if reached the end
					nCurrentPiece[i] = std::rand() % (sizeof(blockModels) / sizeof(blockModels[0]));
					nBlockX[i] = nFieldWidth + 10 + std::rand() % 20;	// Partially randomize spacing between blocks
					score++;	// Increment score
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
				for (int i = 0; i < sizeof(nCurrentPiece) / sizeof(nCurrentPiece[0]); i++) {
					if (blockModels[nCurrentPiece[i]][5 * py + px] != L'.' && nBlockX[i] + px < nScreenWidth) {
						screen[(nBlockY[i] + py) * nScreenWidth + (nBlockX[i] + px)] = blockModels[nCurrentPiece[i]][5 * py + px];
					}
				}
			}
		}

		// Draw score
		swprintf_s(&screen[0], 16, L"SCORE: %8d", score);

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}

	// Shutdown
	CloseHandle(hConsole);
	std::cout << "Game over! Your score: " << score << "\n\n\n";
	system("pause");
	return 0;
}