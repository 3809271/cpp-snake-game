#include <ncurses.h>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#include <deque>
#include <cstring>

using namespace std;

#define INFO 10
#define MISSION 11
#define MISSION_X 12
#define MISSION_V 13
#define TITLE1 14
#define TITLE2 15
#define BKGRD 16

#define BODY 1
#define WALL 2
#define IWALL 3
#define POISON 5
#define GROWTH 6
#define GATE1 7
#define GATE2 8
#define EMPTY 9

const int NONE = -1;
const int DOWN = 0;
const int UP = 1;
const int LEFT = 2;
const int RIGHT = 3;

const int CLOCKWISE[4] = {LEFT, RIGHT, UP, DOWN};

typedef struct position {
    int y;
    int x;
} Position;

class Game {
public:
    int map[21][21];
    int stage, limit, speed, difficult;
    bool isGameOver = false;
    
    Position gate[2];
    int cooldown = 0;
    
    int items[100][2];
    int iTick, iSpeed, iQuantity;
    
    deque<Position> body;
    int sGrowth = 0;
    int sPoison = 0;
    int sGate = 0;
    int direction = NONE;
    
    int missions[4];
    int currLen = 0; int maxLen = 0;
    
    WINDOW *win, *info, *miss;
    
    Game(WINDOW* win, WINDOW* info, WINDOW* miss, int stage) {
        this->win = win;
        this->info = info;
        this->miss = miss;
        this->stage = stage;
        init_pair(EMPTY, COLOR_WHITE, COLOR_WHITE);
        init_pair(BODY, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(WALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(IWALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(GROWTH, COLOR_GREEN, COLOR_GREEN);
        init_pair(POISON, COLOR_RED, COLOR_RED);
        init_pair(GATE1, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(GATE2, COLOR_MAGENTA, COLOR_MAGENTA);
    }
    
    bool init(string path) {
        loadMap(path);
        
        currLen = 3;
        maxLen = 3;
        
        direction = LEFT;
        iTick = iSpeed;
        
        generateItem();
        generateGate();
        addMission();
        return true;
    }
    
    bool tick(int input) {
        if (iTick-- == 0) {
            clearItem();
            generateItem();
            iTick = iSpeed;
        }
        string temp = "Item : "+ to_string(iTick) + "     ";
        char b[25];
        mvwprintw(info, 9, 2, stringToChar(temp,b));
        if (cooldown-- == 0) {
            clearGate();
            generateGate();
        }
        if(limit-- == 0){
            isGameOver = true;
            return false;
        }
        temp = "Time Limit : "+ to_string(limit) + "   ";
        mvwprintw(miss, 3, 2, stringToChar(temp, b));
        wrefresh(miss);
        wrefresh(info);
        return move(input);
    }

    int loadMap(string path) {
        ifstream f;
        f.open(path);
        
        if (f.is_open()) {
            char temp[64];
            for (int y=0; y<21; y++) {
                f.getline(temp, 64);
                for (int x=0; x<21; x++) {
                    map[y][x] = temp[x]-48;
                    if (map[y][x]==0) map[y][x]=EMPTY;
                    else if (map[y][x]==BODY) body.push_back(Position{y, x});
                }
            }
            f.getline(temp, 64);
            speed = stoi(string(temp));
            f.getline(temp, 64);
            iSpeed = stoi(string(temp));
            f.getline(temp, 64);
            iQuantity = stoi(string(temp));
            f.getline(temp, 64);
            difficult = stoi(string(temp));
        } else {
            return 1;
        }
        return 0;
    }
    
    char* stringToChar(string a, char* b){
        strcpy(b,a.c_str());
        return b;
    }
    
    void addMission() {
        char b[20];
        
        srand((unsigned int)time(0));
        missions[0] = rand()%5 + 3 + difficult*2;
        missions[1] = rand()%3 + 1 + difficult;
        missions[2] = rand()%3 + 1 + difficult;
        missions[3] = rand()%2 + 1 + difficult;
        
        limit = 550 - difficult*50;
        
        wclear(info);
        wborder(info, '-','-','-','-','-','-','-','-');
        mvwprintw(info, 1, 2, "[ BOARD ]");
        mvwprintw(info, 3, 2, "B :  3/3");
        mvwprintw(info, 4, 2, "+ :  0");
        mvwprintw(info, 5, 2, "- :  0");
        mvwprintw(info, 6, 2, "G :  0");
        mvwprintw(info, 8, 2, "[ COOLDOWN ]");
        string tmp = "Item : "+ to_string(iTick);
        mvwprintw(info, 9, 2, stringToChar(tmp,b));
        wclear(miss);
        wborder(miss, '-','-','-','-','-','-','-','-');

        wattron(miss, COLOR_PAIR(MISSION));
        mvwprintw(miss, 1, 2, "[ MISSION ]");
        wattroff(miss, COLOR_PAIR(MISSION));
        
        wattron(miss, COLOR_PAIR(MISSION)); tmp = "Time Limit : "+ to_string(limit);
        mvwprintw(miss, 3, 2, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION));

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "B : "+ to_string(missions[0]);
        mvwprintw(miss, 4, 2, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION));
        
        wattron(miss, COLOR_PAIR(MISSION_X)); tmp = "(X)";
        mvwprintw(miss, 4, 10, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_X));

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "+ : "+ to_string(missions[1]);
        mvwprintw(miss, 5, 2, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION));

        wattron(miss, COLOR_PAIR(MISSION_X)); tmp = "(X)";
        mvwprintw(miss, 5, 10, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_X));

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "- : "+ to_string(missions[2]);
        mvwprintw(miss, 6, 2, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION));

        wattron(miss, COLOR_PAIR(MISSION_X)); tmp = "(X)";
        mvwprintw(miss, 6, 10, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_X));

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "G : "+ to_string(missions[3]);
        mvwprintw(miss, 7, 2, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION));

        wattron(miss, COLOR_PAIR(MISSION_X)); tmp = "(X)";
        mvwprintw(miss, 7, 10, stringToChar(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_X));

        wrefresh(info);
        wrefresh(miss);
    }
    
    bool missionCheck(int item){
        string temp;
        char b[20];
        temp = "B :  " + to_string(currLen) + "/" + to_string(maxLen);
        
        mvwprintw(info, 3, 2, stringToChar(temp, b));
        wattron(miss, COLOR_PAIR(MISSION_V));
        if(maxLen == missions[0]) mvwprintw(miss, 4, 10, "(V)");
        wattroff(miss, COLOR_PAIR(MISSION_V));
        switch(item){
            case GROWTH:
                wattron(miss, COLOR_PAIR(INFO));
                temp = "+ :  "+ to_string(sGrowth);
                mvwprintw(info, 4, 2, stringToChar(temp, b));
                wattroff(miss, COLOR_PAIR(INFO));
                wattron(miss, COLOR_PAIR(MISSION_V));
                if(sGrowth == missions[1]) mvwprintw(miss, 5, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_V));
                break;
            case POISON:
                wattron(miss, COLOR_PAIR(INFO));
                temp = "- :  "+ to_string(sPoison);
                mvwprintw(info, 5, 2, stringToChar(temp, b));
                wattroff(miss, COLOR_PAIR(INFO));
                wattron(miss, COLOR_PAIR(MISSION_V));
                if(sPoison == missions[2]) mvwprintw(miss, 6, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_V));
                break;
            case GATE1:
            case GATE2:
                wattron(miss, COLOR_PAIR(INFO));
                temp = "G :  "+ to_string(sGate);
                mvwprintw(info, 6, 2, stringToChar(temp, b));
                wattroff(miss, COLOR_PAIR(INFO));
                wattron(miss, COLOR_PAIR(MISSION_V));
                if(sGate == missions[3]) mvwprintw(miss, 7, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_V));
                break;
        }
        wattroff(miss, COLOR_PAIR(MISSION_V));
        if(missions[0] <= maxLen && missions[1] <= sGrowth && missions[2] <= sPoison && missions[3] <= sGate) return true;
        wrefresh(info);
        wrefresh(miss);
        return false;
    }
    
    void generateItem(){
        srand((unsigned int)time(0));
        int prob = rand()%(iQuantity+1);
        for(int i = 0; i<prob; i++){
            while(true){
                int item_x = rand()%17 + 2; //1~19 random
                int item_y = rand()%17 + 2;
                if(map[item_x][item_y] == EMPTY){
                    map[item_x][item_y] = GROWTH;
                    items[i][0] = item_x;
                    items[i][1] = item_y;
                    break;
                }
            }
        } //growth
        for(int i = prob; i<iQuantity; i++){
            while(true){
                int item_x = rand()%17 + 2; //1~19 random
                int item_y = rand()%17 + 2;
                if(map[item_x][item_y] == EMPTY){
                    map[item_x][item_y] = POISON;
                    items[i][0] = item_x;
                    items[i][1] = item_y;
                    break;
                }
            }
        } //poision
    }
    
    bool getItem(Position p) {
        int tile = map[p.y][p.x];
        
        if (tile == EMPTY) {
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;
            body.push_front(p);
            map[p.y][p.x] = BODY;
        } else if (tile == GROWTH) {
            sGrowth += 1;
            currLen += 1;
            maxLen = currLen>maxLen?currLen:maxLen;
            if(missionCheck(tile)) return false;
            body.push_front(p);
            map[p.y][p.x] = BODY;
            if (cooldown>=0) cooldown++;
        } else if (tile == POISON) {
            sPoison += 1;
            currLen -= 1;
            if(missionCheck(tile)) return false;
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;

            tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;
            
            body.push_front(p);
            map[p.y][p.x] = BODY;
            if (currLen<3){
                isGameOver = true;
                return false;
            }
            if (cooldown>0) cooldown--;
        } else if (tile == GATE1 || tile == GATE2) {
            sGate += 1;
            if(missionCheck(tile)) return false;
            Position depGate = gate[tile-GATE1];
            Position arrGate = gate[GATE2-tile];
            return useGate(depGate, arrGate);
        }
        return true;
    }
    
    void clearItem(){
        for(int i = 0; i<iQuantity; i++){
            if(map[items[i][0]][items[i][1]]!=BODY) map[items[i][0]][items[i][1]] = EMPTY;
        }
        wrefresh(win);
    }

    void generateGate() {
        srand((unsigned int)time(0));
        int gate_x, gate_y;
        while(true){
            gate_x = rand()%21;
            gate_y = rand()%21;
            if(map[gate_x][gate_y] == WALL){
                gate[0] = Position {gate_x, gate_y};
                break;
            }
        }
        map[gate_x][gate_y] = GATE1;
        while(true){
            gate_x = rand()%21;
            gate_y = rand()%21;
            if(map[gate_x][gate_y] == WALL){
                gate[1] = Position {gate_x, gate_y};
                break;
            }
        }
        map[gate_x][gate_y] = GATE2;
    }
    
    bool useGate(Position p1, Position p2) {
        if (!(p2.x==0 || p2.y==0 || p2.x==20 || p2.y==20)) {
            while (true) {
                Position depGate = Position {p2.y + (direction==UP?-1:0) + (direction==DOWN?1:0), p2.x + (direction==LEFT?-1:0) + (direction==RIGHT?1:0)};
                int tile = map[depGate.y][depGate.x];
                if (!((tile == GATE1) || (tile == GATE2) || (tile == WALL) || (tile == IWALL))) break;
                direction = CLOCKWISE[direction];
            }
        } else {
            if (p2.y==20) direction = UP;
            else if (p2.y==0) direction = DOWN;
            else if (p2.x==20) direction = LEFT;
            else if (p2.x==0) direction = RIGHT;
        }
        cooldown = currLen-1;
        Position arrGate = Position {p2.y + (direction==UP?-1:0) + (direction==DOWN?1:0), p2.x + (direction==LEFT?-1:0) + (direction==RIGHT?1:0)};
        if (isValidMove(arrGate))
            return getItem(arrGate);
        isGameOver = true;
        return false;
    }
    
    void clearGate(){
        map[gate[0].y][gate[0].x] = WALL;
        map[gate[1].y][gate[1].x] = WALL;
        wrefresh(win);
    }
    
    bool isValidMove(Position p) {
        if ((map[p.y][p.x] == WALL) || (map[p.y][p.x] == IWALL) || (map[p.y][p.x] == BODY)){
            isGameOver = true;
            return false;
        }
        return true;
    }
    
    bool move(int dir) {
        if (dir == UP) {
            if (direction != DOWN) {
                direction = UP;
            } else {
                isGameOver = true;
                return false;
            }
        } else if (dir == DOWN) {
            if (direction != UP){
                direction = DOWN;
            } else {
                isGameOver = true;
                return false;
            }
        } else if (dir == LEFT) {
            if (direction != RIGHT){
                direction = LEFT;
            } else{
                isGameOver = true;
                return false;
            }
        } else if (dir == RIGHT) {
            if (direction != LEFT){
                direction = RIGHT;
            } else{
                isGameOver = true;
                return false;
            }
        }
        
        if (direction != NONE) {
            Position p = body.front();
            
            if (direction == UP) {
                p.y -= 1;
            }
            else if (direction == DOWN) {
                p.y += 1;
            }
            else if (direction == LEFT) {
                p.x -= 1;
            }
            else if (direction == RIGHT) {
                p.x += 1;
            }
            
            if (isValidMove(p)) {
                return getItem(p);
            } else {
                isGameOver = true;
                return false;
            }
        }
        return true;
    }
    
    void drawMap() {
        for (int y=0; y<21; y++) {
            for (int x=0; x<21; x++) {
                int now = map[y][x];
                wattron(win, COLOR_PAIR(now));
                mvwprintw(win, y, x*2, "aa");
                wattroff(win, COLOR_PAIR(now));
            }
        }
        wrefresh(win);
    }
};

long long getMilliseconds() {
    return chrono::duration_cast<chrono::milliseconds> (
            chrono::system_clock::now().time_since_epoch()
        ).count();
}

int main() {
    // Ncurses setting
    WINDOW *winGame, *winScore, *winMission;
    
    initscr();
    start_color();
    
    curs_set(0);
    noecho();
    init_pair(TITLE1, COLOR_WHITE, COLOR_BLACK);
    init_pair(TITLE2, COLOR_BLACK, COLOR_GREEN);
    int title_x = 10;
    int title_y = 5;
    attron(COLOR_PAIR(TITLE1));
    mvprintw(title_y-1, title_x, "Game of the 14th team");
    attron(A_BOLD);
    mvprintw(title_y+7, title_x+12, "Press any key");
    attroff(COLOR_PAIR(TITLE1));
    attron(COLOR_PAIR(TITLE2));
    mvprintw(title_y+0, title_x, " _____                 _____                _          ");
    mvprintw(title_y+1, title_x, "/  __ \\   _      _    /  ___|              | |         ");
    mvprintw(title_y+2, title_x, "| /  \\/ _| |_  _| |_  \\ `--.  _ __    __ _ | | __  ___ ");
    mvprintw(title_y+3, title_x, "| |    |_   _||_   _|  `--. \\| '_ \\  / _` || |/ / / _ \\");
    mvprintw(title_y+4, title_x, "| \\__/\\  |_|    |_|   /\\__/ /| | | || |_| ||   < |  __/");
    mvprintw(title_y+5, title_x, " \\____/               \\____/ |_| |_| \\__,_||_|\\_\\ \\___|");
    attroff(COLOR_PAIR(TITLE2));
    attroff(A_BOLD);
    getch();
    clear();

    init_pair(BKGRD, COLOR_BLACK,
    COLOR_BLACK);
    border('x', 'x', 'x', 'x', 'x', 'x', 'x', 'x');
    refresh();

    winGame = newwin(22, 42, 1, 1);
    wrefresh(winGame);

    init_pair(INFO, COLOR_WHITE, COLOR_MAGENTA);
    winScore = newwin(11, 30, 1, 47);
    wbkgd(winScore, COLOR_PAIR(INFO));
    wattron(winScore, COLOR_PAIR(INFO));
    wborder(winScore, '-','-','-','-','-','-','-','-');
    wrefresh(winScore);

    init_pair(MISSION, COLOR_WHITE, COLOR_CYAN);
    init_pair(MISSION_X, COLOR_WHITE, COLOR_RED);
    init_pair(MISSION_V, COLOR_WHITE, COLOR_GREEN);

    winMission = newwin(9, 30, 13, 47);
    wbkgd(winMission, COLOR_PAIR(MISSION));
    wborder(winMission, '-','-','-','-','-','-','-','-');
    wrefresh(winMission);

    keypad(stdscr, TRUE);

    nodelay(stdscr, TRUE);
    for(int i = 1; i<=5; i++){
        // Game setting
        Game game(winGame, winScore, winMission, i);
        int input = NONE;
        long long time = getMilliseconds();
        game.init("maps/" + to_string(i));

        // Main loop
        while (true) {
            int ch = getch();
            if (ch == 110 || ch == 78) {
                break;
            }
            if (258 <= ch && ch <= 261)
                input = ch - 258;
            
            long long curr = getMilliseconds();
            long long dt = curr - time;
            if (dt >= game.speed) {
                if (!game.tick(input)) {
                    // Game over
                    break;
                }
                game.drawMap();

                wrefresh(winGame);
                time = curr;
                input = NONE;
            }
        }

        //game_over || game_clear
        if (game.isGameOver) {
            wclear(winMission);
            mvwprintw(winMission, 1, 8, "[ Game Over! ]");
            mvwprintw(winMission, 3, 11, "ReStart?");
            mvwprintw(winMission, 4, 13, "Y/N");
            wborder(winMission, '-','-','-','-','-','-','-','-');
            wrefresh(winMission);
            
            int ch2;
            while(true){
                ch2 = getch();
                if(ch2 == 121 || ch2 == 89){
                    i--;
                    break;
                }
                else if(ch2 == 110 || ch2 == 78) break;
            }
            if(ch2 == 110 || ch2 == 78) break;
        }
        else if(i == 4){
            wclear(winMission);
            mvwprintw(winMission, 1, 7, "[ Game Clear! ]");
            mvwprintw(winMission, 3, 11, "ReStart?");
            mvwprintw(winMission, 4, 13, "Y/N");
            wborder(winMission, '-','-','-','-','-','-','-','-');
            wrefresh(winMission);
            int ch2;
            while(true){
                ch2 = getch();
                if(ch2 == 121 || ch2 == 89){
                    i=0;
                    break;
                }
                else if(ch2 == 110 || ch2 == 78) break;
            }
            if(ch2 == 110 || ch2 == 78) break;
        }

        getch();
    }
    delwin(winGame);
    endwin();
    return 0;
}
