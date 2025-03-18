#define _WIN32_WINNT 0x0500
#include <stdio.h>
#include <windows.h>
#include <threads.h>
#include <stdbool.h>
#include "main.h"

#define HEIGHT 20
#define WIDTH (HEIGHT * 2)

#define RED FOREGROUND_RED
#define BLUE FOREGROUND_BLUE
#define GREEN FOREGROUND_GREEN
#define WHITE RED | BLUE | GREEN

void writePixel(CHAR_INFO *buffer, int x, int y, int color, char ch)
{
    buffer[(WIDTH * y) + x].Char.UnicodeChar = ch;
    buffer[(WIDTH * y) + x].Attributes = color;
}

void swapBuffer(HANDLE console, CHAR_INFO *buffer)
{
    static COORD size = {.X = WIDTH, .Y = HEIGHT};
    static COORD base = {.X = 0, .Y = 0};
    static SMALL_RECT rect = {.Left = 0, .Top = 0, .Right = WIDTH - 1, .Bottom = HEIGHT - 1};
    WriteConsoleOutputA(console, buffer, size, base, &rect);
    SetConsoleCursorPosition(console, (COORD){.X = WIDTH + 1, .Y = HEIGHT + 1});
}

const char map[HEIGHT][WIDTH] = {
    "########################################",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "##------------------------------------##",
    "########################################",
};

void drawMap(CHAR_INFO buffer[HEIGHT * WIDTH], const char mapState[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            char c = mapState[y][x];
            int color = WHITE;
            switch (c)
            {
            case '#':
            {
                color = RED;
                break;
            }
            case '-':
            {
                color = BLUE;
                break;
            }
            case 'f':
            {
                color = GREEN;
                break;
            }
            // case 's': {
            //     color = WHITE;
            //     break;
            // }
            default:
                break;
            }
            writePixel(buffer, x, y, color, 219);
        }
    }
}

SHORT keyState[256] = {0};
bool pressedKeys[256] = {0};

int inputLoop(void *arg)
{
    while (true)
    {

        for (int i = 0; i < 256; i++)
        {
            keyState[i] = GetAsyncKeyState(i);
            pressedKeys[i] = (keyState[i] & 0x8000) | pressedKeys[i];
        }
        Sleep(1);
    }
}

void clearInputs()
{
    memset(pressedKeys, 0, 256);
}

char getTile(const Vec2 pos, const char mapState[WIDTH][HEIGHT])
{
    return mapState[pos.x][pos.y];
}

void handleMovement(Vec2 *snakePos, char mapState[HEIGHT][WIDTH])
{
    mapState[snakePos->y][snakePos->x] = '-';
    if (pressedKeys['A'])
    {
        snakePos->x--;
        if (mapState[snakePos->y][snakePos->x] == '#')
        {
            snakePos->x++;
        }
    }
    if (pressedKeys['D'])
    {
        snakePos->x++;
        if (mapState[snakePos->y][snakePos->x] == '#')
        {
            snakePos->x--;
        }
    }
    if (pressedKeys['W'])
    {
        snakePos->y--;
        if (mapState[snakePos->y][snakePos->x] == '#')
        {
            snakePos->y++;
        }
    }
    if (pressedKeys['S'])
    {
        snakePos->y++;
        if (mapState[snakePos->y][snakePos->x] == '#')
        {
            snakePos->y--;
        }
    }
    char newTile = mapState[snakePos->y][snakePos->x];
    if (newTile == 'f')
    {
        // MessageBox(NULL, L"Comeu Fruta", NULL, MB_OK);
    }
    mapState[snakePos->y][snakePos->x] = 's';
}

bool spawnFruit(Vec2 *pos, char mapState[HEIGHT][WIDTH])
{
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    if (mapState[y][x] == '-')
    {
        pos->x = x;
        pos->y = y;
        mapState[y][x] = 'f';
        return true;
    }
    return spawnFruit(pos, mapState);
}

BOOL WINAPI sigHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursor = {.bVisible = true, .dwSize = sizeof(CONSOLE_CURSOR_INFO)};
        SetConsoleCursorInfo(console, &cursor);
        exit(0);
        break;
    default:
        break;
    }
}

int main()
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CHAR_INFO buffer[WIDTH * HEIGHT] = {0};

    CONSOLE_CURSOR_INFO cursor = {.bVisible = false, .dwSize = sizeof(CONSOLE_CURSOR_INFO)};
    SetConsoleCursorInfo(console, &cursor);

    SetConsoleCtrlHandler(sigHandler, true);

    system("pause");
    system("cls");

    thrd_t inputThread;
    thrd_create(&inputThread, inputLoop, NULL);

    char mapState[HEIGHT][WIDTH] = {0};
    memcpy_s(mapState, WIDTH * HEIGHT, map, WIDTH * HEIGHT);

    bool running = true;

    Vec2 snakePos = {WIDTH / 2, HEIGHT / 2};
    mapState[snakePos.y][snakePos.x] = 's';

    Vec2 fruitPos;
    spawnFruit(&fruitPos, mapState);

    long time = 0;
    QueryPerformanceCounter(&time);
    srand(time);

    while (running)
    {
        drawMap(buffer, mapState);
        // writePixel(buffer, snakePos.x, snakePos.y, WHITE, 219);
        swapBuffer(console, buffer);

        if (vec2Equals(snakePos, fruitPos))
        {
        }
        if (mapState[fruitPos.y][fruitPos.x] != 'f')
        {
            spawnFruit(&fruitPos, mapState);
        }

        handleMovement(&snakePos, mapState);
        clearInputs();
        Sleep(16*6);
    }

    return 0;
}
