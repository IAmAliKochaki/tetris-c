#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define Sleep(ms) usleep((ms))
static void setTerminalRawMode();
static void resetTerminalMode();

#endif

#include "tetris.h"

const int tetrominoes[7][16] = {
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // I
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // O
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0}, // S
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // Z
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, // T
    {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, // L
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0}  // J
};

int arena[A_HEIGHT][A_WIDTH];

uint32_t score = 0;
bool gameOver = false;
int currTetrominoIdx;
int currRotation = 0;
int currX = A_WIDTH / 2;
int currY = 0;

int main()
{
#ifndef _WIN32
    setTerminalRawMode();
#endif
    srand(time(NULL));   // generate new sequence each time
    printf("\e[2J\e[H"); // clear screen & reset cursor position
    memset(arena, 0, sizeof(arena[0][0]) * A_HEIGHT * A_WIDTH);
    newTetromino();

    const int targetFrameTime = 350;
    clock_t lastTime = clock();

    while (!gameOver)
    {
        clock_t now = clock();
        clock_t elapsed = (now - lastTime) * 1000 / CLOCKS_PER_SEC;
        processInputs();

        if (elapsed >= targetFrameTime)
        {
            if (!moveDown())
            {
                addToArena();
                checkLines();
                newTetromino();
            }
            lastTime = now;
        }

        drawArena();
        Sleep(10);
    }

    printf("\e[2J\e[H"); // clear screen & reset cursor position
    printf("Game over!\nScore: %d\n", score);
    printf("\e[?25h"); // show cursor
#ifndef _WIN32
    resetTerminalMode();
#endif
    return 0;
}

void newTetromino()
{
    currTetrominoIdx = rand() % 7;
    currRotation = 0;
    currX = (A_WIDTH / 2) - (T_WIDTH / 2);
    currY = 0;
    gameOver = !validPos(currTetrominoIdx, currRotation, currX, currY);
}

bool validPos(int tetromino, int rotation, int posX, int posY)
{
    for (int x = 0; x < T_WIDTH; x++)
    {
        for (int y = 0; y < T_HEIGHT; y++)
        {
            int index = rotate(x, y, rotation);
            if (1 != tetrominoes[tetromino][index])
            {
                continue;
            }

            int arenaX = x + posX;
            int arenaY = y + posY;
            if (0 > arenaX || A_WIDTH <= arenaX || A_HEIGHT <= arenaY)
            {
                return false;
            }

            int arenaXY = arena[arenaY][arenaX];
            if (0 <= arenaY && 1 == arenaXY)
            {
                return false;
            }
        }
    }
    return true;
}

int rotate(int x, int y, int rotation)
{
    switch (rotation % 4)
    {
    case 0:
        return x + y * T_WIDTH;
    case 1:
        return 12 + y - (x * T_WIDTH);
    case 2:
        return 15 - (y * T_WIDTH) - x;
    case 3:
        return 3 - y + (x * T_WIDTH);
    default:
        return 0;
    }
}

#ifndef _WIN32
static void setTerminalRawMode()
{
    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag &= ~(ICANON | ECHO);
    info.c_cc[VMIN] = 1;
    info.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &info);
}

static void resetTerminalMode()
{
    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag |= (ICANON | ECHO);
    tcsetattr(0, TCSANOW, &info);
}

int _getch()
{
    return getchar();
}
int _kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

#endif
void processInputs()
{
    if (!_kbhit())
    {
        return;
    }

    while (_kbhit())
    {
        int key = _getch();

#ifdef _WIN32
        switch (key)
        {
        case 32: // Spacebar
            int nextRotation = (currRotation + 1) % 4;
            if (validPos(currTetrominoIdx, nextRotation, currX, currY))
            {
                currRotation = nextRotation;
            }
            break;
        case 75: // Left arrow key
            if (validPos(currTetrominoIdx, currRotation, currX - 1, currY))
            {
                currX--;
            }
            break;
        case 77: // Right arrow key
            if (validPos(currTetrominoIdx, currRotation, currX + 1, currY))
            {
                currX++;
            }
            break;
        case 80: // Down arrow key
            if (validPos(currTetrominoIdx, currRotation, currX, currY + 1))
            {
                currY++;
            }
            break;
        }

#else
        // POSIX handle escape sequences
        if (key == 27 && _kbhit())
        {
            _getch(); // skip '['
            int arrow = _getch();

            switch (arrow)
            {
            case 'D': // Left arrow key
                if (validPos(currTetrominoIdx, currRotation, currX - 1, currY))
                {
                    currX--;
                }
                break;
            case 'C': // Right arrow key
                if (validPos(currTetrominoIdx, currRotation, currX + 1, currY))
                {
                    currX++;
                }
                break;
            case 'B': // Down arrow key
                if (validPos(currTetrominoIdx, currRotation, currX, currY + 1))
                {
                    currY++;
                }
                break;
            }
        }
        else if (key == 32) // Spacebar
        {
            int nextRotation = (currRotation + 1) % 4;
            if (validPos(currTetrominoIdx, nextRotation, currX, currY))
            {
                currRotation = nextRotation;
            }
            break;
        }
#endif
    }
}

bool moveDown()
{
    if (validPos(currTetrominoIdx, currRotation, currX, currY + 1))
    {
        currY++;
        return true;
    }
    return false;
}

void addToArena()
{
    for (int y = 0; y < T_HEIGHT; y++)
    {
        for (int x = 0; x < T_WIDTH; x++)
        {
            int index = rotate(x, y, currRotation);
            if (1 != tetrominoes[currTetrominoIdx][index])
            {
                continue;
            }

            int arenaX = currX + x;
            int arenaY = currY + y;
            bool xInRange = (0 <= arenaX) && (arenaX < A_WIDTH);
            bool yInRange = (0 <= arenaY) && (arenaY < A_HEIGHT);
            if (xInRange && yInRange)
            {
                arena[arenaY][arenaX] = 1;
            }
        }
    }
}

void checkLines()
{
    int clearedLines = 0;

    for (int y = A_HEIGHT - 1; y >= 0; y--)
    {
        bool lineFull = true;
        for (int x = 0; x < A_WIDTH; x++)
        {
            if (0 == arena[y][x])
            {
                lineFull = false;
                break;
            }
        }

        if (!lineFull)
        {
            continue;
        }

        clearedLines++;
        for (int yy = y; yy > 0; yy--)
        {
            for (int xx = 0; xx < A_WIDTH; xx++)
            {
                arena[yy][xx] = arena[yy - 1][xx];
            }
        }

        for (int xx = 0; xx < A_WIDTH; xx++)
        {
            arena[0][xx] = 0;
        }
        y++;
    }

    if (0 < clearedLines)
    {
        score += 100 * clearedLines;
    }
}

void drawArena()
{
    printf("\e[?25l\e[H"); // hide cursor & reset cursor position

    char buffer[512];
    int bufferIndex = 0;

    for (int y = 0; y < A_HEIGHT; y++)
    {
        buffer[bufferIndex++] = '|';

        for (int x = 0; x < A_WIDTH; x++)
        {
            int rotatedPos = rotate(x - currX, y - currY, currRotation);
            bool validX = x >= currX && x < currX + T_WIDTH;
            bool validY = y >= currY && y < currY + T_HEIGHT;
            bool xyFilled = 1 == tetrominoes[currTetrominoIdx][rotatedPos];

            if (1 == arena[y][x] || (validX && validY && xyFilled))
            {
                buffer[bufferIndex++] = '#';
            }
            else
            {
                buffer[bufferIndex++] = ' ';
            }
        }

        buffer[bufferIndex++] = '|';
        buffer[bufferIndex++] = '\n';
    }

    buffer[bufferIndex] = '\0';
    printf("%s\n\nScore: %d\n\n", buffer, score);
}
