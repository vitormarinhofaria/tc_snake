#define _WIN32_WINNT 0x0500
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include "main.h"

#define HEIGHT 20
#define WIDTH 20

#define RED FOREGROUND_RED
#define BLUE FOREGROUND_BLUE
#define GREEN FOREGROUND_GREEN
#define WHITE RED | BLUE | GREEN

#define KEYPRESSED 0x0001
#define KEYDOWN 0x8000

#define IS_DEAD 0b0001
#define GOT_FRUIT 0b0010

void restoreConsole(HANDLE console)
{
    CONSOLE_CURSOR_INFO cursor = {.bVisible = true, .dwSize = sizeof(CONSOLE_CURSOR_INFO)};
    SetConsoleCursorInfo(console, &cursor);
}

void writePixel(CHAR_INFO *buffer, int x, int y, int color, wchar_t ch)
{
    int bufferY = y / 2;
    int position = (WIDTH * bufferY) + x;
    CHAR_INFO *cell = &buffer[position];
    cell->Char.UnicodeChar = ch;
    int currentColor = cell->Attributes;

    if (y % 2 == 0)
    {
        cell->Attributes = color;
    }
    else
    {
        if (currentColor != 0)
            color = color | currentColor;
        cell->Attributes = color;
    }
}

#define BHEIGHT (HEIGHT / 2)
void swapBuffer(HANDLE console, CHAR_INFO *buffer)
{
    static COORD size = {.X = WIDTH, .Y = BHEIGHT};
    static COORD base = {.X = 0, .Y = 0};
    static SMALL_RECT rect = {.Left = 0, .Top = 0, .Right = WIDTH - 1, .Bottom = BHEIGHT - 1};
    WriteConsoleOutputW(console, buffer, size, base, &rect);
    SetConsoleCursorPosition(console, (COORD){.X = WIDTH + 1, .Y = BHEIGHT + 1});
    memset(buffer, 0, sizeof(CHAR_INFO) * WIDTH * BHEIGHT);
}

const wchar_t map[HEIGHT][WIDTH] = {
    L"####################",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"#------------------#",
    L"####################",
};

int getColor(int y, int upper, int lower)
{
    if (y % 2 == 0)
        return upper;
    return lower;
}
bool isAscii = false;
void drawMap(CHAR_INFO buffer[HEIGHT * WIDTH], const wchar_t mapState[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            wchar_t c = mapState[y][x];
            int color = WHITE;
            wchar_t w = ' ';
            switch (c)
            {
            case L'#':
            {
                color = getColor(y, FOREGROUND_RED, BACKGROUND_RED);
                w = L'#';
                break;
            }
            case L'-':
            {
                // color = getColor(y, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
                color = getColor(y, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
                w = L' ';
                break;
            }
            case L'f':
            {
                color = getColor(y, FOREGROUND_GREEN | FOREGROUND_INTENSITY, BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                w = L'*';
                break;
            }
            case L's':
            {
                color = getColor(y, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                w = '@';
                break;
            }
            case L't':
            {
                color = getColor(y, FOREGROUND_GREEN | FOREGROUND_BLUE, BACKGROUND_GREEN | BACKGROUND_BLUE);
                w = 'o';
                break;
            }
            default:
                break;
            }
            if (isAscii)
            {
                writePixel(buffer, x, y, color, w);
            }
            else
            {
                // writePixel(buffer, x, y, color, 219);
                writePixel(buffer, x, y, color, L'▀');
            }
        }
    }
}

void writeText(HANDLE console, int x, int y, int color, const char *text, size_t len)
{
    WORD attr[1024] = {color};
    memset(attr, color, 1024);
    COORD coord = {.X = x, .Y = y};
    int w = 0;
    WriteConsoleOutputCharacterA(console, text, len, coord, &w);
    WriteConsoleOutputAttribute(console, attr, len, coord, &w);
}

void drawScore(HANDLE console, int score)
{
    char scoreStr[256] = {0};
    sprintf(scoreStr, "Score: %d", score);
    writeText(console, WIDTH + 1, 0, FOREGROUND_GREEN | FOREGROUND_INTENSITY, scoreStr, strlen(scoreStr));
}

void gameOver(HANDLE console)
{
    char gameOverStr[] = "Game Over";
    int w = 0;
    int strLen = strlen(gameOverStr);
    COORD coord = {.X = (WIDTH / 2) - (strLen / 2), .Y = HEIGHT / 2};
    writeText(console, coord.X, coord.Y, RED | FOREGROUND_INTENSITY, gameOverStr, strLen);
    restoreConsole(console);
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
            pressedKeys[i] = (keyState[i] & KEYDOWN) | pressedKeys[i];
        }
        Sleep(1);
    }
}

void readInput()
{
    for (int i = 0; i < 256; i++)
    {
        keyState[i] = GetAsyncKeyState(i);
        pressedKeys[i] = (keyState[i] & KEYPRESSED) | pressedKeys[i];
    }
}

void clearInputs()
{
    memset(pressedKeys, 0, 256);
}

char getTile(const Vec2 pos, const wchar_t mapState[WIDTH][HEIGHT])
{
    return mapState[pos.x][pos.y];
}

void handleInput(Vec2 *moveDir)
{
    if (pressedKeys['A'] && moveDir->x != 1)
    {
        moveDir->x = -1;
        moveDir->y = 0;
    }
    if (pressedKeys['D'] && moveDir->x != -1)
    {
        moveDir->x = 1;
        moveDir->y = 0;
    }
    if (pressedKeys['W'] && moveDir->y != 1)
    {
        moveDir->x = 0;
        moveDir->y = -1;
    }
    if (pressedKeys['S'] && moveDir->y != -1)
    {
        moveDir->x = 0;
        moveDir->y = 1;
    }
    if (pressedKeys['P'])
    {
        isAscii = !isAscii;
    }
}

void checkColision(Vec2 *snake[256], wchar_t mapState[HEIGHT][WIDTH], byte *state)
{
    Vec2 *snakeHead = snake[0];
    if (snakeHead->x < 0 || snakeHead->x >= WIDTH || snakeHead->y < 0 || snakeHead->y >= HEIGHT)
    {
        *state |= IS_DEAD;
    }
    if (mapState[snakeHead->y][snakeHead->x] == L'#' || mapState[snakeHead->y][snakeHead->x] == L't')
    {
        *state |= IS_DEAD;
    }
    if (mapState[snakeHead->y][snakeHead->x] == L'f')
    {
        *state |= GOT_FRUIT;
    }
}

bool spawnFruit(Vec2 *pos, wchar_t mapState[HEIGHT][WIDTH])
{
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    if (mapState[y][x] == L'-')
    {
        pos->x = x;
        pos->y = y;
        mapState[y][x] = L'f';
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
        restoreConsole(console);
        exit(0);
        break;
    default:
        break;
    }
    return true;
}

void drawSnake(Vec2 *snake[256], wchar_t mapState[HEIGHT][WIDTH])
{
    mapState[snake[0]->y][snake[0]->x] = L's';
    for (int i = 1; i < 256; i++)
    {
        Vec2 *part = snake[i];
        if (part == NULL)
        {
            break;
        }
        mapState[part->y][part->x] = L't';
    }
}
void clearSnake(Vec2 *snake[256], wchar_t mapState[HEIGHT][WIDTH])
{
    for (int i = 0; i < 256; i++)
    {
        Vec2 *part = snake[i];
        if (part == NULL)
        {
            break;
        }
        mapState[part->y][part->x] = L'-';
    }
}
void moveSnake(Vec2 *snake[256], int numParts, Vec2 moveDir, byte *state)
{
    Vec2 *snakeHead = snake[0];
    Vec2 newHead = *snakeHead;
    vec2Add(&newHead, moveDir);
    for (int i = numParts; i > 0; i--)
    {
        Vec2 *part = snake[i];
        if (part == NULL)
        {
            break;
        }
        *snake[i] = *snake[i - 1];
        if (vec2Equals(*snake[i], newHead))
        {
            *state |= IS_DEAD;
            break;
        }
    }
    *snakeHead = newHead;
}

int main(int argc, char **argv)
{
    _setmode(_fileno(stdout), CP_UTF8);
    if (argc > 1)
    {
        if (strcmp(argv[1], "ascii") == 0)
        {
            isAscii = true;
        }
    }
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CHAR_INFO buffer[WIDTH * (BHEIGHT)] = {0};

    CONSOLE_CURSOR_INFO cursor = {.bVisible = false, .dwSize = sizeof(CONSOLE_CURSOR_INFO)};
    SetConsoleCursorInfo(console, &cursor);

    SetConsoleCtrlHandler(sigHandler, true);

    system("cls");
    int numParts = 0;
    Vec2 *snake[256] = {NULL};
    snake[0] = malloc(sizeof(Vec2));

    wchar_t mapState[HEIGHT][WIDTH] = {0};
    memcpy_s(mapState, sizeof(mapState), map, sizeof(map));

    bool running = true;
    Vec2 *snakeHead = snake[0];
    snakeHead->x = WIDTH / 2;
    snakeHead->y = HEIGHT / 2;

    drawSnake(snake, mapState);

    Vec2 fruitPos;
    spawnFruit(&fruitPos, mapState);

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    srand(time.HighPart);
    Vec2 moveDir = {1, 0};

    byte state = 0;
    char *th = "Press P to toggle ASCII mode";
    // writeText(console, WIDTH + 1, 1, FOREGROUND_GREEN | FOREGROUND_INTENSITY, th, strlen(th));

    while (running)
    {
        readInput();

        if (mapState[fruitPos.y][fruitPos.x] != L'f')
        {
        }

        handleInput(&moveDir);
        clearSnake(snake, mapState);
        moveSnake(snake, numParts, moveDir, &state);
        checkColision(snake, mapState, &state);
        drawSnake(snake, mapState);
        if (state & IS_DEAD)
        {
            running = false;
            gameOver(console);
            break;
        }
        if (state & GOT_FRUIT)
        {
            numParts++;
            snake[numParts] = malloc(sizeof(Vec2));
            *snake[numParts] = *snake[numParts - 1];
            spawnFruit(&fruitPos, mapState);
        }

        drawMap(buffer, mapState);
        swapBuffer(console, buffer);
        drawScore(console, numParts);
        clearInputs();
        state = 0;
        Sleep(100 * 2);
    }

    return 0;
}
