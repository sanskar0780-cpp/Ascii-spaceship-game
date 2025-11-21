#include <iostream>
#include <conio.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>
#include <fstream>
#include <xinput.h>
#include <mmsystem.h>
#include <ctime>
#include <cmath>

using namespace std;

#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "winmm.lib")

void playMusicMCI(const wchar_t* filename) {
    wchar_t cmd[512];
    // Open file
    swprintf(cmd, 512, L"open \"%s\" type waveaudio alias music", filename);
    MMRESULT res = mciSendString(cmd, NULL, 0, NULL);
    if (res != 0) {

        return;
    }
    // Play looped
    mciSendString(L"play music repeat", NULL, 0, NULL);
    cout << "DEBUG: MCI music started (" << filename << ")" << endl;
}

XINPUT_STATE controllerState;
XINPUT_STATE prevControllerState;

typedef DWORD(WINAPI* pXInputGetState_t)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI* pXInputSetState_t)(DWORD, XINPUT_VIBRATION*);
pXInputGetState_t myXInputGetState = nullptr;
pXInputSetState_t myXInputSetState = nullptr;
HMODULE xinputDLL = nullptr;

bool controllerConnected = false;
const float DEADZONE = 0.2f;

const int width = 120;
const int height = 48;

const int infoWidth = 60;

int playerY = height / 2 - 1;
int playerX = 5;

vector<vector<char>> board(height, vector<char>(width, ' '));
vector<vector<int>> boardColors(height, vector<int>(width, 7));

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void SetCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

void setColour(int colour) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colour);
}

int highScore = 0;

int loadHighScore() {
    ifstream file("highscore.txt");
    int hs = 0;
    if (file >> hs) {
        return hs;
    }
    return 0;
}

void saveHighScore(int newScore) {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << newScore;
    }
}

int shootCooldown = 0;
int shootCooldownMax = 4;

int thrustAnimation = 0;
const int THRUSTER_MAX_FRAMES = 3;
int thrusterSpeedTimer = 0;
const int THRUSTER_UPDATE_RATE = 5; // Change the animation frame every 5 frames

vector<string> playerShip = {
    "*/^\\ ",
    "<===>",
    "*\\_/ "
};

int shipWidth = 5;
int shipHeight = 3; 

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN, QUIT };
Direction dir;

enum EnemyType { STRAIGHT, SINE_WAVE, CHARGER };

// --- Global Enemy Art Definitions ---
vector<string> enemyShip_Scout = {
    "/-\\",
    "<O|",
    "\\-/"
};

vector<string> enemyShip_Fighter = {
    " <=>",
    "-(X)-",
    " <=>"
};

vector<string> enemyShip_Bomber = {
    "o-o",
    "|=|",
    "o-o"
};

struct Enemy {
    int x, y;
    Direction dir;
    vector<string> enemyShip;

    int height;
    int width;
    int colour;
    EnemyType type;

    int originalY;
    int sineTimer;
};


//Stars here

struct Stars {
    int x;
    int y;
    int colour;
};

vector<Stars> stars;
vector<Stars> oldstars;

void initialStars() {
    for (int i = 0; i < 20; i++) {
        stars.push_back({
            rand() % width,
            rand() % height,
            8
            });
    }
}


//BOSS DETAILS HERE

struct Boss {
    int x, y;
    int health;
    int maxHealth = 10; //default : 50
    vector <string> ship = {
        "  /\\  ",
        " |==| ",
        "/|**|\\",
        "\\|==|/",
        "  \\/  ",
        "  ==  "
    };

    int width = 6;
    int height = 6;
};

struct bossBullet {
    int x, y;
};

int bossHitFlashTimer = 0;

vector<bossBullet> bossBullets;
vector<bossBullet> oldBBPosition;
int bossShootTimer = 0;
int bossShootRate = 60;

int oldBossX = 0;
int oldBossY = 0;

Boss boss;
bool bossActive = false;
int bossSpawnScr = 100;

int bossMoveTimer = 0;
int bossMoveSpeed = 30;
int bossDir = 1;

bool upgrade1 = false;

vector<Enemy> oldEnemyPositions;

vector<Enemy> enemies;

int enemySpawnTimer = 0;
const int enemySpawnRate = 80;
const int enemySpeed = 10;

struct Bullet {
    int x, y;
};
vector<Bullet> bullets;
vector<Bullet> oldBulletPos;
int score = 0;

void printinPanel(const vector<string>& lines, int startX, int startY, int colour) {
    setColour(colour);
    for (int i = 0; i < (int)lines.size(); i++) {
        SetCursorPosition(startX + width, startY + i);
        cout << lines[i];
    }
    setColour(7);
}

void drawPlayer(int x, int y) {
    for (int i = 0; i < shipHeight; i++) {
        SetCursorPosition(x + 1, y + 1 + i);
        cout << playerShip[i];
    }
}

void erasePlayer(int x, int y) {
    for (int i = 0; i < shipHeight; i++) {
        for (int j = 0; j < shipWidth; j++) {
            int by = y + i;
            int bx = x + j;
            SetCursorPosition(bx + 1, by + 1);
            if (by >= 0 && by < height && bx >= 0 && bx < width) {
                char ch = board[by][bx];
                int originalColor = boardColors[by][bx];
                setColour(originalColor);
                cout << ch;
            }
            else {
                setColour(7);
                cout << ' ';
            }
        }
    }
}

void drawEnemy(const Enemy& enemy) {

    setColour(enemy.colour);
    for (int i = 0; i < enemy.height; i++) {
        SetCursorPosition(enemy.x + 1, enemy.y + 1 + i);
        cout << enemy.enemyShip[i];
    }
    setColour(7);
}

void eraseEnemy(const Enemy& enemy) {
    string blank(enemy.width, ' ');

    for (int i = 0; i < enemy.height; i++) {
        
        SetCursorPosition(enemy.x + 1, enemy.y + 1 + i);
        
        cout << blank;
    }
}

void drawBullet(int bx, int by) {
    if (bx < 0 || bx >= width || by < 0 || by >= height) return;
    SetCursorPosition(bx + 1, by + 1);
    setColour(15);
    cout << '-';
    setColour(7);
}

void drawBossBullet(int bbx, int bby) {
    if (bbx < 0 || bbx >= width || bby < 0 || bby >= height) return;
    SetCursorPosition(bbx + 1, bby + 1);
    setColour(4);
    cout << "*";
    setColour(7);
}

void eraseBossBullet(int bbx, int bby) {
    if (bbx < 0 || bbx >= width || bby < 0 || bby >= height) return;
    SetCursorPosition(bbx + 1, bby + 1);
    char ch = board[bby][bbx];
    int col = boardColors[bby][bbx];
    setColour(col);
    cout << ch;
    setColour(7);
}

void eraseBullet(int bx, int by) {
    if (bx < 0 || bx >= width || by < 0 || by >= height) return;
    SetCursorPosition(bx + 1, by + 1);
    char ch = board[by][bx];
    int col = boardColors[by][bx];
    setColour(col);
    cout << ch;
    setColour(7);
}

void drawBoss(int bx, int by) {

    for (int i = 0; i < boss.height; i++) {
        SetCursorPosition(bx + 1, by + 1 + i);
        cout << boss.ship[i];

    }

}

void eraseBoss(int bx, int by) {
    for (int i = 0; i < boss.height; i++) {
        for (int j = 0; j < boss.width; j++) {
            int byy = by + i;
            int bxx = bx + j;
            if (byy >= 0 && byy < height && bxx >= 0 && bxx < width) {
                // Restore original board char and color
                char originalChar = board[byy][bxx];
                int originalColor = boardColors[byy][bxx];
                SetCursorPosition(bxx + 1, byy + 1);
                setColour(originalColor);
                cout << originalChar;
            }
            else {
                SetCursorPosition(bxx + 1, byy + 1);
                setColour(7);
                cout << ' ';
            }
        }
    }
    setColour(7);
}

vector<string> healthDisplay;

void drawStars(int x, int y, int color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        SetCursorPosition(x + 1, y + 1);
            setColour(color);
            cout << ".";
            setColour(7);
    }

}

void eraseStars(int x, int y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        SetCursorPosition(x + 1, y + 1);
        cout << " ";
    }
}

void clearInfoPanel() {
    for (int y = 0; y < height; y++) {
        SetCursorPosition(width + 2, y + 1);
        for (int x = 0; x < infoWidth; x++) cout << " ";
    }
}

void clearLinePanel(int startY, int Lines) {
    for (int i = 0; i < Lines; i++) {
        SetCursorPosition(width + 2, startY + i + 1);
        for (int x = 0; x < infoWidth; x++) cout << " ";
    }
}

void displayBossHealth() {
    int barLength = 20;
    int filled = (boss.health * barLength) / boss.maxHealth;
    setColour(12);  // Color for brackets and spaces
    cout << "Boss Health = [";
    setColour(38);  // Color for filled part ("=")
    for (int i = 0; i < barLength; i++) {
        if (i < filled) {
            setColour(38);
            cout << " ";
        }
        else {
            setColour(7);
            cout << " ";
        }
    }
    setColour(12);  // Back to bracket color
    cout << "]";
    setColour(7);  // Reset to default
}

void clearBoardPanel() {
    for (int y = 0; y < height; y++) {
        SetCursorPosition(1, y + 1);
        for (int x = 0; x < width; x++) {
            cout << ' ';
        }
    }
}

void StartScreen() {
    setColour(4);
    cout << "__________                                ___________       __                \n";
    cout << "\\______   \\_______   ____   ______ ______ \\_   _____/ _____/  |_  ___________ \n";
    cout << " |     ___/\\_  __ \\_/ __ \\ /  ___//  ___/  |    __)_ /    \\   __\\/ __ \\_  __ \\\n";
    setColour(9);
    cout << " |    |     |  | \\/\\  ___/ \\___ \\ \\___ \\   |        \\   |  \\  | \\  ___/|  | \\/\n";
    cout << " |____|     |__|    \\___  >____  >____  > /_______  /___|  /__|  \\___  >__|   \n";
    cout << "                        \\/     \\/     \\/          \\/     \\/          \\/       \n";
}

void drawOnBoard(vector<string>& art, int startX, int startY, int colour) {
    for (int y = 0; y < (int)art.size(); y++) {
        for (int x = 0; x < (int)art[y].size(); x++) {
            char ch = art[y][x];
            if (ch != ' ') {
                if (startY + y >= 0 && startY + y < height &&
                    startX + x >= 0 && startX + x < width) {
                    board[startY + y][startX + x] = ch;
                    boardColors[startY + y][startX + x] = colour;
                }
                SetCursorPosition(startX + x + 1, startY + y + 1);
                setColour(colour);
                cout << ch;
                setColour(7);
            }
        }
    }
}

void flushInputBuffer() {
    while (_kbhit()) {
        _getch();
    }
}

void drawBoardContents() {
    for (int y = 0; y < height; y++) {
        SetCursorPosition(1, y + 1);
        for (int x = 0; x < width; x++) {
            board[y][x] = ' ';
            boardColors[y][x] = 7;
        }
    }
    clearBoardPanel();
    clearInfoPanel();
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void drawThrusters(int playerX, int playerY, int prevX, int prevY) {

    int oldThrusterDrawX = prevX - 1;
    for (int i = 0; i < shipHeight; i++) {
        SetCursorPosition(oldThrusterDrawX - THRUSTER_MAX_FRAMES + 1, prevY + 1 + i);
        setColour(7);

        cout << "   ";
    }

    int thrusterX = playerX - 1;
    string flame;
    int colour;

    switch (thrustAnimation) {
    case 0 : 
        flame = ">";
        colour = 14;
        break;

    case 1 :
        flame = ">>";
        colour = 12;
        break;

    case 2: 
        flame = ">>>";
        colour = 6;
        break;
    }

    for (int i = 0; i < shipHeight; i++) {

        SetCursorPosition(thrusterX - flame.length() + 1, playerY + 1 + i);
        setColour(colour);

        cout << flame;

        int spaceToClear = 3 - flame.length();
        for (int k = 0; k < spaceToClear; k++) {
            cout << ' ';
        }
    }
    setColour(7);
}

bool gameOver = false;

void drawInitialBoard() {
    SetCursorPosition(0, 0);
    setColour(11);
    cout << "+";
    for (int x = 0; x < width; x++) {
        cout << "-";
    }
    cout << "+";
    for (int x = 0; x < infoWidth; x++) {
        cout << "-";
    }
    cout << "+\n";
    for (int y = 0; y < height; y++) {
        setColour(11);
        cout << "|";
        for (int x = 0; x < width; x++) cout << " ";
        cout << "|";
        for (int x = 0; x < infoWidth; x++) cout << " ";
        cout << "|\n";
    }
    setColour(11);
    cout << "+";
    for (int x = 0; x < width; x++) {
        cout << "-";
    }
    cout << "+";
    for (int x = 0; x < infoWidth; x++) {
        cout << "-";
    }
    cout << "+\n";
    setColour(7);
}

int prevPlayerY = playerY;
int prevPlayerX = playerX;

void updatePlayer() {
    prevPlayerX = playerX;
    prevPlayerY = playerY;
    int nextY = playerY;
    switch (dir) {
    case UP:
        nextY--;
        break;
    case DOWN:
        nextY++;
        break;
    }
    if (nextY >= 0 && nextY + shipHeight <= height) {
        playerY = nextY;
    }
}

void updatePlayerOnScreen() {
    erasePlayer(prevPlayerX, prevPlayerY);
    setColour(14);
    drawPlayer(playerX, playerY);
    setColour(7);
}

void input() {
    dir = STOP;
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
        case '0': highScore += -99999999; break;
        case 'q': gameOver = true; break;
        case 'w': dir = UP; break;
        case 's': dir = DOWN; break;
        case ' ': // Spacebar for shoot
            if (shootCooldown <= 0) {
                if (!upgrade1) {
                    int spawnY = playerY + 1;

                    bullets.push_back({ playerX + shipWidth, spawnY });
                }
                else if (upgrade1) {
                    int spawnY = playerY;
                    bullets.push_back({ playerX + shipWidth, spawnY });
                    bullets.push_back({ playerX + shipWidth, spawnY + 1 });
                    bullets.push_back({ playerX + shipWidth, spawnY + 2});
                }
                shootCooldown = shootCooldownMax;
            }
            break;
        
        default: break;
        }
    }
    DWORD result;
    if (myXInputGetState) {
        result = myXInputGetState(0, &controllerState);
    }
    else {
        result = ERROR_DEVICE_NOT_CONNECTED;
    }
    if (result == ERROR_SUCCESS) {
        controllerConnected = true;
        float stickX = controllerState.Gamepad.sThumbLX / 32768.0f;
        float stickY = controllerState.Gamepad.sThumbLY / 32768.0f;
        if (abs(stickX) < DEADZONE) stickX = 0.0f;
        if (abs(stickY) < DEADZONE) stickY = 0.0f;
        if (abs(stickY) > abs(stickX)) {
            if (stickY < 0) {
                dir = UP;
            }
            else if (stickY > 0) {
                dir = DOWN;
            }
        }
        WORD buttons = controllerState.Gamepad.wButtons;
        if (dir == STOP) {
            if (buttons & XINPUT_GAMEPAD_DPAD_UP) {
                dir = UP;
            }
            else if (buttons & XINPUT_GAMEPAD_DPAD_DOWN) {
                dir = DOWN;
            }
        }
        WORD pressed = buttons & ~prevControllerState.Gamepad.wButtons;
        if (pressed & XINPUT_GAMEPAD_A) {
            if (shootCooldown <= 0) {
                if (!upgrade1) {
                    int spawnY = playerY + 1;
                    bullets.push_back({ playerX + shipWidth, spawnY });
                }
                else {
                    bullets.push_back({ playerX + shipWidth, playerY });
                    bullets.push_back({ playerX + shipWidth, playerY + 1 });
                    bullets.push_back({ playerX + shipWidth, playerY + 2 });
                }
                shootCooldown = shootCooldownMax;
            }
        }
        if (pressed & XINPUT_GAMEPAD_B) {
            dir = QUIT;
        }
        prevControllerState = controllerState;
    }
    else {
        controllerConnected = false;
    }
}

void logic() {

    if (shootCooldown > 0) shootCooldown--;

    updatePlayer();

    thrusterSpeedTimer++;

    if (thrusterSpeedTimer >= THRUSTER_UPDATE_RATE) {
        
        thrustAnimation = (thrustAnimation + 1) % THRUSTER_MAX_FRAMES;
        thrusterSpeedTimer = 0; // Reset the timer
    }

    //Stars spawner
    static int frameCounter = 0;
    frameCounter++;

    oldstars = stars;
    if (frameCounter % 15 == 0) {
        for (auto& s : stars) {
            s.x--;

            if (s.x <= 1) {
                s.x = width - 1;
                s.y = rand() % height;
            }
        }
    }

    if (frameCounter > 1000) {
        frameCounter = 0;
    }

    //Enemy spawner 
    enemySpawnTimer++;
    if (enemySpawnTimer >= enemySpawnRate) {
        enemySpawnTimer = 0;
        int spawnY = rand() % (height - 3);
        Enemy newEnemy;

        newEnemy.dir = LEFT;

        int type = rand() % 3;

        if (type == 0) {
            newEnemy.type = STRAIGHT;
            newEnemy.enemyShip = enemyShip_Bomber;
            newEnemy.width = 3;
            newEnemy.height = 3;
            newEnemy.colour = 12;
        }
        else if (type == 1) {
            newEnemy.type = SINE_WAVE;
            newEnemy.enemyShip = enemyShip_Scout;
            newEnemy.width = 5;
            newEnemy.height = 3;
            newEnemy.colour = 13;
        }
        else {
            newEnemy.type = CHARGER;
            newEnemy.enemyShip = enemyShip_Fighter;
            newEnemy.width = 6;
            newEnemy.height = 3;
            newEnemy.colour = 14;
        }

        newEnemy.x = width - newEnemy.width - 1;
        newEnemy.y = rand() % height - newEnemy.height;
        if (newEnemy.type == SINE_WAVE) {
            newEnemy.originalY = newEnemy.y; // Store its starting height
            newEnemy.sineTimer = 0;
        }

        enemies.push_back(newEnemy);
    }

    static int enemyMoveTimer = 0;
    enemyMoveTimer++;
    if (enemyMoveTimer >= enemySpeed) {
        enemyMoveTimer = 0;
        oldEnemyPositions = enemies;
        for (auto& e : enemies) {
            switch (e.type) {
            case STRAIGHT :
                e.x--;
                break;
            case SINE_WAVE:
                e.x--;
                e.sineTimer++;

                e.y = e.originalY + static_cast<int>(cos(e.sineTimer * 0.2) * 6);
                break;

            case CHARGER:
                e.x -= 3;
                break;
            }

            if (e.y < 0) e.y = 0;
            if (e.y + e.height >= height) e.y = height - e.height;
        }
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.x < 0; }), enemies.end());
    }

    //boss active condition : 
    if (!bossActive && score >= bossSpawnScr) {
        bossActive = true;
        bossSpawnScr += 500;
        if (bossShootRate >= 20) {
            bossShootRate -= 10;
        }
        if (bossMoveSpeed >= 5) {
            bossMoveSpeed -= 5;
        }
        boss.x = width - boss.width - 2;
        boss.y = height / 2 - boss.height / 2;
        boss.health = boss.maxHealth;
    }


    oldBulletPos = bullets;
    for (auto& b : bullets) {
        b.x++;
        if (b.x >= width) {
            b.x = -1;
        }
    }
    oldBBPosition = bossBullets;
    for (auto& bb : bossBullets) {
        bb.x--;
        if (bb.x < 1) {
            bb.x = -1;
        }
    }
    bossBullets.erase(remove_if(bossBullets.begin(), bossBullets.end(), [](const bossBullet& bb) { return bb.x < 0; }), bossBullets.end());

    //bullet-enemy collision
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        bool hit = false;
        for (auto eit = enemies.begin(); eit != enemies.end(); ) {

            bool hitX = (it->x >= eit->x) && (it->x < eit->x + eit->width);
            bool hitY = (it->y >= eit->y) && (it->y < eit->y + eit->height);

            if (hitX && hitY) {
                eraseEnemy(*eit);

                eit = enemies.erase(eit);
                hit = true;
                score += 50;
                break;
            }
            else {
                ++eit;
            }
        }

        //Boss bullets - player bullet collision
        for (auto bbit = bossBullets.begin(); bbit != bossBullets.end(); ) {

            if (it->x == bbit->x && it->y == bbit->y) {

                eraseBossBullet(bbit->x, bbit->y);

                bbit = bossBullets.erase(bbit);

                hit = true;


                break;
            }
            else {
                ++bbit;
            }
        }

        if (bossActive) {

            // ONLY COLLISION CHECK HERE
            
            if (it->x >= boss.x && it->x < boss.x + boss.width &&
                it->y >= boss.y && it->y < boss.y + boss.height) {
                boss.health--;
                bossHitFlashTimer = 3;
                hit = true;

                if (boss.health <= 0) {

                    upgrade1 = true;
                    bossActive = false;
                    eraseBoss(boss.x, boss.y);
                    score += 100;
                }
            }
        }

        if (hit || it->x < 0) {
            eraseBullet(it->x, it->y);
            it = bullets.erase(it);
        }
        else {
            ++it;
        }

    } // END OF PLAYER BULLET LOOP

    //Boss bullet collision
    for (const auto& bb : bossBullets) {
        if (bb.x >= playerX && bb.x < playerX + shipWidth &&
            bb.y >= playerY && bb.y < playerY + shipHeight) {
            gameOver = true;
            break;
        }
    }

    // BOSS AI - Runs independently every frame!
    if (bossActive) {

        //  SAVE OLD POSITION (Fixes erase bug)
        oldBossX = boss.x;
        oldBossY = boss.y;

        //  BOSS SHOOT
        bossShootTimer++;
        if (bossShootTimer >= bossShootRate) {
            bossShootTimer = 0;
            int shootY = boss.y + boss.height / 2;
            int offsets[] = { 1, 2, 3, -1, -2, -3, 0 };
            for (int offset : offsets) {
                bossBullets.push_back({ boss.x, shootY + offset });
            }
        }

        //  BOSS MOVEMENT
        bossMoveTimer++;
        if (bossMoveTimer >= bossMoveSpeed) {
            bossMoveTimer = 0;
            int bossRandDir = rand() % 3 - 1;
            boss.y += bossRandDir;
            if (boss.y < 0) boss.y = 0;
            if (boss.y + boss.height > height) boss.y = height - boss.height;
        }

    }
}

int main() {
    hideCursor();
    srand(static_cast<unsigned int>(time(nullptr)));

    StartScreen();
    _getch();
    system("cls");

    PlaySound(L"Starting_page.wav", NULL, SND_ASYNC | SND_LOOP);
    highScore = loadHighScore();

    drawInitialBoard();
    drawBoardContents();

    initialStars();
    SetCursorPosition(playerX + 1, playerY + 1);
    drawPlayer(playerX, playerY);




    ZeroMemory(&prevControllerState, sizeof(XINPUT_STATE));
    xinputDLL = LoadLibraryA("xinput1_4.dll");
    if (!xinputDLL) {
        xinputDLL = LoadLibraryA("xinput1_3.dll");
    }
    if (xinputDLL) {
        myXInputGetState = (pXInputGetState_t)GetProcAddress(xinputDLL, "XInputGetState");
        myXInputSetState = (pXInputSetState_t)GetProcAddress(xinputDLL, "XInputSetState");
    }
    while (!gameOver) {
        input();
        logic();

        for (const auto& s : oldstars)eraseStars(s.x, s.y);
        for (const auto& s : stars) drawStars(s.x, s.y, s.colour);


        drawThrusters(playerX, playerY, prevPlayerX, prevPlayerY);
        updatePlayerOnScreen();
        if (!enemies.empty()) {
            for (const auto& oe : oldEnemyPositions) {
                eraseEnemy(oe);
            }
            for (const auto& e : enemies) {
                drawEnemy(e);
            }
        }
        if (bossActive) {
            eraseBoss(oldBossX, oldBossY);

            if (bossHitFlashTimer > 0) {
                setColour(12); // RED
                bossHitFlashTimer--;
            }
            else {
                setColour(15); // White colour
            }

            drawBoss(boss.x, boss.y);
            SetCursorPosition(width + 2 + 5, 8);
            displayBossHealth();
        }
        else {
            clearLinePanel(7, 1);
        }

        if (!bullets.empty()) {
            for (const auto& ob : oldBulletPos) {
                eraseBullet(ob.x, ob.y);
            }
            for (const auto& b : bullets) {
                drawBullet(b.x, b.y);
            }
        }
        if (!bossBullets.empty()) {
            for (const auto& obb : oldBBPosition) {
                eraseBossBullet(obb.x, obb.y);
            }
            for (const auto& bb : bossBullets) {
                drawBossBullet(bb.x, bb.y);
            }
        }
        
        //High score display

        if (highScore < 0) {
            highScore = 0;
        }

        vector<string> highScoreDisplay = { "High Score: " + to_string(highScore) };
        printinPanel(highScoreDisplay, 4, 1, 14);

        //score display
        vector<string> scoreDisplay = { "Score: " + to_string(score) };
        printinPanel(scoreDisplay, 4, 3, 10);

        this_thread::sleep_for(chrono::milliseconds(20));
    }
    system("cls");
    drawInitialBoard();

    SetCursorPosition(width / 2 - 4, height / 2);

    cout << "GAME OVER!";
    SetCursorPosition(0, height + 2);
    if (score > highScore) {
        highScore = score;
        SetCursorPosition(width / 2 - 10, height / 2 + 1);
        setColour(14); // Yellow
        cout << "!!! NEW HIGH SCORE: " << score << " !!!";
        saveHighScore(score);
    }
    else {
        SetCursorPosition(width / 2 - 15, height / 2 + 1);
        setColour(10); // Green
        cout << "Your Score: " << score << " | High Score: " << highScore;
    }
    setColour(7);

    SetCursorPosition(0, height + 2);
    if (xinputDLL) FreeLibrary(xinputDLL);
    return 0;
}